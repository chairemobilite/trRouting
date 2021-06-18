#include "calculator.hpp"

namespace TrRouting
{
    
  std::string Calculator::odTripsRouting()
  {
    if (params.debugDisplay)
      std::cout << "  preparing odTripsRouting" << std::endl;

    RoutingResult  routingResult;
    std::string    response;
    nlohmann::json json;
    nlohmann::json odTripJson;
    nlohmann::json lineProfilesJson;
    nlohmann::json pathProfilesJson;
    std::map<boost::uuids::uuid, float> lineProfiles; // key: line uuid, value: count od trips using this line
    std::map<boost::uuids::uuid, std::vector<std::vector<float>>> pathProfiles; // key: path uuid, value: [index: segment index, value: [index: hourOfDay, demand]]
    std::map<boost::uuids::uuid, std::vector<float>> pathTotalProfiles; // key: path uuid, value: [index: segment index, value: totalDemand]
    std::vector<float> demandByHourOfDay;

    int    legTripIdx;
    int    legLineIdx;
    int    legPathIdx;
    Path * legPath;
    int    legConnectionStartIdx;
    int    legConnectionEndIdx;
    int    connectionDepartureTimeSeconds;
    int    connectionDepartureTimeHour;
    bool   atLeastOneOdTrip           {false};
    bool   atLeastOneCompatiblePeriod {false};
    bool   attributesMatches          {true};
    bool   resetFilters               {true};
    int    odTripsCount = odTrips.size();
    float  maximumSegmentHourlyDemand = 0.0;
    float  maximumSegmentTotalDemand  = 0.0;
    int    countOdTripsCalculated     = 0;
    int    totalTravelTimeSeconds     = 0;

    json["odTrips"] = nlohmann::json::array();

    for (auto & line : lines)
    {
      lineProfiles[line->uuid] = 0.0;
    }
    for (int i = 0; i <= 28; i++)
    {
      demandByHourOfDay.push_back(0.0);
    }
    
    if (params.responseFormat == "csv" && params.batchNumber == 1) // write header only on first batch, so we can easily append subsequent batches to the same csv file
    {
      // write csv header:
      response += "uuid,internalId,status,ageGroup,gender,occupation,destinationActivity,mode,expansionFactor,travelTimeSeconds,onlyWalkingTravelTimeSeconds,onlyCyclingTravelTimeSeconds,onlyDrivingTravelTimeSeconds,"
                     "declaredDepartureTimeSeconds,departureTimeSeconds,initialDepartureTimeSeconds,arrivalTimeSeconds,numberOfTransfers,inVehicleTravelTimeSeconds,"
                     "transferTravelTimeSeconds,waitingTimeSeconds,accessTravelTimeSeconds,egressTravelTimeSeconds,transferWaitingTimeSeconds,"
                     "firstWaitingTimeSeconds,nonTransitTravelTimeSeconds,lineUuids,modeShortnames,agencyUuids,boardingNodeUuids,unboardingNodeUuids,tripUuids\n";
    }

    if (params.debugDisplay)
      std::cout << "  starting odTripsRouting" << std::endl;

    int stopAtI {-1};
    std::vector<int> odTripIndexes(odTripsCount);
    std::cerr << odTripsCount << std::endl;
    std::iota(odTripIndexes.begin(), odTripIndexes.end(), 0);

    if (odTripsCount == 0)
    {
      if (params.responseFormat == "json")
      {
        response = json.dump(2);
      }
      return response;
    }
    if (params.odTripsSampleRatio > 0.0 && params.odTripsSampleRatio < 1.0)
    {

      //std::random_device rd;
      //std::mt19937 engine(rd(params.seed));

      //std::cout << " shuffle odTrips with seed: " << params.seed << " generates number " << engine() << std::endl;
      //engine.seed(params.seed);
      // sort by departure time seconds before shuffling so seeds are consistent:
      
      std::cout << " first ODTrip uuid: " << odTripIndexes[0] << std::endl;
      std::shuffle(odTripIndexes.begin(), odTripIndexes.end(), std::mt19937{params.seed});
      std::cout << " first ODTrip uuid after shuffle: " << odTripIndexes[0] << std::endl;
      //stopAtI = ceil((float)(odTrips.size()) * params.odTripsSampleRatio);
    }

    int i {0};
    int j {0};
    int odTripIndex {0}; // for shuffle
    int sampleSize {(int)(ceil((float)(odTripsCount) * params.odTripsSampleRatio))};

    for (int i = 0; i < sampleSize; i++)
    {

      odTripIndex = odTripIndexes[i];
      odTrip      = odTrips[odTripIndex].get();
      //std::cout << odTrip->uuid << std::endl;

      if ( i % params.batchesCount != params.batchNumber - 1) // when using multiple parallel calculators
      {
        //i++;
        continue;
      }

      /*if (stopAtI != -1 && i == stopAtI)
      {
        break;
      }*/
      
      attributesMatches          = true;
      atLeastOneCompatiblePeriod = false;
      
      // verify that od trip matches selected attributes:
      if ( (params.odTripsAgeGroups.size()   > 0 && std::find(params.odTripsAgeGroups.begin(),   params.odTripsAgeGroups.end(),   persons[odTrip->personIdx]->ageGroup)   == params.odTripsAgeGroups.end()) 
        || (params.odTripsGenders.size()     > 0 && std::find(params.odTripsGenders.begin(),     params.odTripsGenders.end(),     persons[odTrip->personIdx]->gender)     == params.odTripsGenders.end())
        || (params.odTripsOccupations.size() > 0 && std::find(params.odTripsOccupations.begin(), params.odTripsOccupations.end(), persons[odTrip->personIdx]->occupation) == params.odTripsOccupations.end())
        || (params.odTripsActivities.size()  > 0 && std::find(params.odTripsActivities.begin(),  params.odTripsActivities.end(),  odTrip->destinationActivity)            == params.odTripsActivities.end())
        || (params.odTripsModes.size()       > 0 && std::find(params.odTripsModes.begin(),       params.odTripsModes.end(),       odTrip->mode)                           == params.odTripsModes.end())
      )
      {
        attributesMatches = false;
      }

      // filter wrong data source if only data source is provided:
      if (params.onlyDataSourceIdx != -1 && odTrip->dataSourceIdx != params.onlyDataSourceIdx)
      {
        attributesMatches = false;
      }

      // verify that od trip matches at least one selected period:
      for (auto & period : params.odTripsPeriods)
      {
        if (odTrip->departureTimeSeconds >= period.first && odTrip->departureTimeSeconds < period.second)
        {
          atLeastOneCompatiblePeriod = true;
        }
      }
      
      if (attributesMatches && (atLeastOneCompatiblePeriod || params.odTripsPeriods.size() == 0))
      {

        if (params.debugDisplay)
        {
          std::cout << "od trip uuid " << odTrip->uuid << " (" << (i+1) << "/" << odTripsCount << ")" << std::endl << " dts: " << odTrip->departureTimeSeconds << " atLeastOneCompatiblePeriod: " << (atLeastOneCompatiblePeriod ? "true " : "false ") << "attributesMatches: " << (attributesMatches ? "true " : "false ") << std::endl;
        }
        else if ((i + 1) % 1000 == 0)
        {
          if (stopAtI > 0)
          {
            std::cout << (i+1) << "/" << (stopAtI + 1) << std::endl;
          }
          else
          {
            std::cout << (i+1) << "/" << odTripsCount << std::endl;
          }
        }
        origin        = odTrip->origin.get();
        destination   = odTrip->destination.get();
        routingResult = calculate(true, resetFilters); // reset filters only on first calculation
        resetFilters  = false;
        countOdTripsCalculated++;

        float correctedExpansionFactor = odTrip->expansionFactor / params.odTripsSampleRatio;
        atLeastOneOdTrip = true;
        if (routingResult.legs.size() > 0)
        {
          totalTravelTimeSeconds += correctedExpansionFactor * routingResult.travelTimeSeconds;
          if (params.responseFormat != "csv" && params.calculateProfiles == true)
          {
            for (auto & leg : routingResult.legs)
            {
              legTripIdx            = std::get<0>(leg);
              legLineIdx            = std::get<1>(leg);
              legPathIdx            = std::get<2>(leg);
              legPath               = paths[legPathIdx].get();
              legConnectionStartIdx = std::get<3>(leg);
              legConnectionEndIdx   = std::get<4>(leg);
              lineProfiles[lines[legLineIdx].get()->uuid] += correctedExpansionFactor;
              
              if (pathProfiles.find(legPath->uuid) == pathProfiles.end())
              {
                pathProfiles[legPath->uuid] = std::vector<std::vector<float>>(legPath->nodesIdx.size() - 1, demandByHourOfDay);
                pathTotalProfiles[legPath->uuid] = std::vector<float>(legPath->nodesIdx.size() - 1, 0.0);
              }
              for (int connectionIndex = legConnectionStartIdx; connectionIndex <= legConnectionEndIdx; connectionIndex++)
              {
                connectionDepartureTimeSeconds = *tripConnectionDepartureTimes[legTripIdx][connectionIndex];
                *tripConnectionDemands[legTripIdx][connectionIndex] += correctedExpansionFactor;
                connectionDepartureTimeHour    = connectionDepartureTimeSeconds / 3600;
                //std::cout << "pUuid:" << legPath->uuid << " dth:" << connectionDepartureTimeHour << " cI:" << connectionIndex << " oldD:" << pathProfiles[legPath.uuid][connectionIndex][connectionDepartureTimeHour] << std::endl;
                pathProfiles[legPath->uuid][connectionIndex][connectionDepartureTimeHour] += correctedExpansionFactor;
                pathTotalProfiles[legPath->uuid][connectionIndex] += correctedExpansionFactor;
                if (maximumSegmentHourlyDemand < pathProfiles[legPath->uuid][connectionIndex][connectionDepartureTimeHour])
                {
                  maximumSegmentHourlyDemand = pathProfiles[legPath->uuid][connectionIndex][connectionDepartureTimeHour];
                }
                if (maximumSegmentTotalDemand < pathTotalProfiles[legPath->uuid][connectionIndex])
                {
                  maximumSegmentTotalDemand = pathTotalProfiles[legPath->uuid][connectionIndex];
                }
              }
            }
          }
        }
        
        std::vector<std::string> lineShortnames;
        //std::vector<std::string> lineLongnames;
        std::vector<std::string> agencyAcronyms;
        for (auto & lineIdx : routingResult.linesIdx)
        {
          lineShortnames.push_back(lines[lineIdx].get()->shortname);
          //lineLongnames.push_back(lines[lineIdx].get()->longname);
          agencyAcronyms.push_back(agencies[lines[lineIdx].get()->agencyIdx].get()->acronym);
        }

        if (params.responseFormat == "csv")
        {
          //std::replace( ageGroup.begin(), ageGroup.end(), '-', '_' ); // remove dash so Excel does not convert to age groups to numbers...
          response += boost::uuids::to_string(odTrip->uuid) + ",\"" + odTrip->internalId + "\",\"" + routingResult.status + "\",\"" /*+ ageGroup*/ + "\",\"" /*+ odTrip->gender*/ + "\",\"" /*+ odTrip->occupation*/ + "\",\"";
          response += odTrip->destinationActivity + "\",\"" + odTrip->mode + "\"," + std::to_string(correctedExpansionFactor) + "," + std::to_string(routingResult.travelTimeSeconds) + ",";
          response += std::to_string(odTrip->walkingTravelTimeSeconds) + "," + std::to_string(odTrip->cyclingTravelTimeSeconds) + "," + std::to_string(odTrip->drivingTravelTimeSeconds) + "," + std::to_string(odTrip->departureTimeSeconds) + "," + std::to_string(routingResult.departureTimeSeconds) + "," + std::to_string(routingResult.initialDepartureTimeSeconds) + ",";
          response += std::to_string(routingResult.arrivalTimeSeconds) + "," + std::to_string(routingResult.numberOfTransfers) + "," + std::to_string(routingResult.inVehicleTravelTimeSeconds) + ",";
          response += std::to_string(routingResult.transferTravelTimeSeconds) + "," + std::to_string(routingResult.waitingTimeSeconds) + "," + std::to_string(routingResult.accessTravelTimeSeconds) + ",";
          response += std::to_string(routingResult.egressTravelTimeSeconds) + "," + std::to_string(routingResult.transferWaitingTimeSeconds) + "," + std::to_string(routingResult.firstWaitingTimeSeconds) + ",";
          response += std::to_string(routingResult.nonTransitTravelTimeSeconds) + ",";

          int countLineUuids = routingResult.lineUuids.size();
          j = 0;
          for (auto & lineShortname : lineShortnames)
          {
            response += lineShortname;
            if (j < countLineUuids - 1)
            {
              response += "|";
            }
            j++;
          }
          response += ",";
          /*j = 0;
          for (auto & lineLongname : lineLongnames)
          {
            response += lineLongname;
            if (j < countLineUuids - 1)
            {
              response += "|";
            }
            j++;
          }
          response += ",";*/
          j = 0;
          for (auto & agencyAcronym : agencyAcronyms)
          {
            response += agencyAcronym;
            if (j < countLineUuids - 1)
            {
              response += "|";
            }
            j++;
          }
          response += ",";
          j = 0;
          for (auto & lineUuid : routingResult.lineUuids)
          {
            response += boost::uuids::to_string(lineUuid);
            if (j < countLineUuids - 1)
            {
              response += "|";
            }
            j++;
          }
          response += ",";
          j = 0;
          for (auto & modeShortname : routingResult.modeShortnames)
          {
            response += modeShortname;
            if (j < countLineUuids - 1)
            {
              response += "|";
            }
            j++;
          }
          response += ",";
          j = 0;
          for (auto & agencyUuid : routingResult.agencyUuids)
          {
            response += boost::uuids::to_string(agencyUuid);
            if (j < countLineUuids - 1)
            {
              response += "|";
            }
            j++;
          }
          response += ",";
          j = 0;
          for (auto & boardingNodeUuid : routingResult.boardingNodeUuids)
          {
            response += boost::uuids::to_string(boardingNodeUuid);
            if (j < countLineUuids - 1)
            {
              response += "|";
            }
            j++;
          }
          response += ",";
          j = 0;
          for (auto & unboardingNodeUuid : routingResult.unboardingNodeUuids)
          {
            response += boost::uuids::to_string(unboardingNodeUuid);
            if (j < countLineUuids - 1)
            {
              response += "|";
            }
            j++;
          }
          response += ",";
          j = 0;
          for (auto & tripUuid : routingResult.tripUuids)
          {
            response += boost::uuids::to_string(tripUuid);
            if (j < countLineUuids - 1)
            {
              response += "|";
            }
            j++;
          }
          response += "\n";
        }
        else
        {
          odTripJson = {};
          odTripJson["uuid"]                          = boost::uuids::to_string(odTrip->uuid);
          odTripJson["status"]                        = routingResult.status;
          /*odTripJson["ageGroup"]                    = persons[odTrip->personIdx].ageGroup; // this fails (segmentation fault)...
          odTripJson["gender"]                        = persons[odTrip->personIdx].gender;
          odTripJson["occupation"]                    = persons[odTrip->personIdx].occupation;*/
          odTripJson["originLat"]                     = odTrip->origin->latitude;
          odTripJson["originLon"]                     = odTrip->origin->longitude;
          odTripJson["destinationLat"]                = odTrip->destination->latitude;
          odTripJson["destinationLon"]                = odTrip->destination->longitude;
          odTripJson["internalId"]                    = odTrip->internalId;
          odTripJson["originActivity"]                = odTrip->originActivity;
          odTripJson["destinationActivity"]           = odTrip->destinationActivity;
          odTripJson["declaredMode"]                  = odTrip->mode;
          odTripJson["expansionFactor"]               = correctedExpansionFactor;
          odTripJson["travelTimeSeconds"]             = routingResult.travelTimeSeconds;
          odTripJson["initialLostTimeAtDepartureSeconds"] = routingResult.initialLostTimeAtDepartureSeconds;
          odTripJson["onlyWalkingTravelTimeSeconds"]  = odTrip->walkingTravelTimeSeconds;
          odTripJson["onlyCyclingTravelTimeSeconds"]  = odTrip->cyclingTravelTimeSeconds;
          odTripJson["onlyDrivingTravelTimeSeconds"]  = odTrip->drivingTravelTimeSeconds;
          odTripJson["declaredDepartureTimeSeconds"]  = odTrip->departureTimeSeconds;
          odTripJson["declaredArrivalTimeSeconds"]    = odTrip->arrivalTimeSeconds;
          odTripJson["departureTimeSeconds"]          = routingResult.departureTimeSeconds;
          odTripJson["arrivalTimeSeconds"]            = routingResult.arrivalTimeSeconds;
          odTripJson["numberOfTransfers"]             = routingResult.numberOfTransfers;
          odTripJson["inVehicleTravelTimeSeconds"]    = routingResult.inVehicleTravelTimeSeconds;
          odTripJson["transferTravelTimeSeconds"]     = routingResult.transferTravelTimeSeconds;
          odTripJson["waitingTimeSeconds"]            = routingResult.waitingTimeSeconds;
          odTripJson["accessTravelTimeSeconds"]       = routingResult.accessTravelTimeSeconds;
          odTripJson["egressTravelTimeSeconds"]       = routingResult.egressTravelTimeSeconds;
          odTripJson["transferWaitingTimeSeconds"]    = routingResult.transferWaitingTimeSeconds;
          odTripJson["firstWaitingTimeSeconds"]       = routingResult.firstWaitingTimeSeconds;
          odTripJson["nonTransitTravelTimeSeconds"]   = routingResult.nonTransitTravelTimeSeconds;

          odTripJson["linesShortnames"]               = lineShortnames;
          //odTripJson["linesLongname"]                 = lineLongnames;
          odTripJson["agenciesAcronyms"]              = agencyAcronyms;
          odTripJson["modesShortnames"]               = routingResult.modeShortnames;
          odTripJson["agencyUuids"]                   = Toolbox::uuidsToStrings(routingResult.agencyUuids);
          odTripJson["lineUuids"]                     = Toolbox::uuidsToStrings(routingResult.lineUuids);
          odTripJson["boardingNodeUuids"]             = Toolbox::uuidsToStrings(routingResult.boardingNodeUuids);
          odTripJson["unboardingNodeUuids"]           = Toolbox::uuidsToStrings(routingResult.unboardingNodeUuids);
          //odTripJson["tripUuids"]                     = Toolbox::uuidsToStrings(routingResult.tripUuids);
          json["odTrips"].push_back(odTripJson);
        }
      }

      //i++;
      if (params.odTripsSampleSize > 0 && i + 1 >= params.odTripsSampleSize)
      {
        break;
      }
    }

    if (params.responseFormat != "csv")
    {
      json["maxSegmentHourlyDemand"] = maximumSegmentHourlyDemand;
      json["maxSegmentTotalDemand"]  = maximumSegmentTotalDemand;
      json["totalTravelTimeSeconds"] = totalTravelTimeSeconds;
      
      if (params.calculateProfiles == true)
      {

        lineProfilesJson = {};

        for (auto & lineCount : lineProfiles)
        {
          lineProfilesJson[boost::uuids::to_string(lineCount.first)] = lineCount.second;
        }

        json["lineProfiles"] = lineProfilesJson;

        pathProfilesJson = {};

        for (auto & pathProfile : pathProfiles)
        {
          pathProfilesJson[boost::uuids::to_string(pathProfile.first)] = pathProfile.second;
          //pathsOdTripsProfilesSequenceJson = {};
          /*for (auto & segmentProfile : pathProfile)
          {

            //pathsOdTripsProfilesOdTripUuids.clear();
            //for (auto & odTripUuid : std::get<1>(sequenceProfile.second))
            //{
            //  pathsOdTripsProfilesOdTripUuids.push_back()
            //}
            //pathsOdTripsProfilesSequenceJson[std::to_string(sequenceProfile.first)] = {{"demand", std::get<0>(sequenceProfile.second)}, {"odTripUuids", Toolbox::uuidsToStrings(std::get<1>(sequenceProfile.second))}};
          }*/
          //pathsOdTripsProfilesJson[boost::uuids::to_string(pathProfile.first)] = pathsProfile;
        }

        json["pathProfiles"] = pathProfilesJson;

      }
      
    }
    if (params.calculateAllOdTrips && params.responseFormat == "csv")
    {
      
      if (params.saveResultToFile)
      {
        std::cerr << "writing csv file" << std::endl;
        std::ofstream csvFile;
        //csvFile.imbue(std::locale("en_US.UTF8"));
        csvFile.open(params.calculationName + "__batch_" + std::to_string(params.batchNumber) + "_of_" + std::to_string(params.batchesCount) + ".csv", std::ios_base::trunc);
        csvFile << response;
        csvFile.close();
      }
    }
    else // json
    {
      response = json.dump(2);
    }

    return response;

  }

  

}
