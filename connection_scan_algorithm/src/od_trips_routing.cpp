#include "calculator.hpp"
#include "toolbox.hpp"
#include "constants.hpp"
#include "parameters.hpp"
#include "path.hpp"
#include "person.hpp"
#include "line.hpp"
#include "trip.hpp"
#include "routing_result.hpp"

namespace TrRouting
{
  class StepToOdTripJsonVisitor: public StepVisitor<int> {
  private:
    std::vector<std::string> lineShortnames;
    std::vector<std::string> agencyAcronyms;
    std::vector<std::string> modeShortnames;
    std::vector<boost::uuids::uuid> agencyUuids;
    std::vector<boost::uuids::uuid> lineUuids;
    std::vector<boost::uuids::uuid> boardingNodesUuids;
    std::vector<boost::uuids::uuid> unboardingNodesUuids;
  public:
    int getResult() { return 1; }
    std::vector<std::string> getLineShortnames() { return lineShortnames; }
    std::vector<std::string> getAgencyAcronyms() { return agencyAcronyms; }
    std::vector<std::string> getModeShortnames() { return modeShortnames; }
    std::vector<boost::uuids::uuid> getAgencyUuids() { return agencyUuids; }
    std::vector<boost::uuids::uuid> getLineUuids() { return lineUuids; }
    std::vector<boost::uuids::uuid> getBoardingNodesUuids() { return boardingNodesUuids; }
    std::vector<boost::uuids::uuid> getUnboardingNodesUuids() { return unboardingNodesUuids; }
    void visitBoardingStep(const BoardingStep& step) override;
    void visitUnboardingStep(const UnboardingStep& step) override;
    void visitWalkingStep(const WalkingStep& step) override;
  };

  void StepToOdTripJsonVisitor::visitBoardingStep(const BoardingStep& step)
  {
    lineShortnames.push_back(step.lineShortname);
    agencyAcronyms.push_back(step.agencyAcronym);
    modeShortnames.push_back(step.mode);
    agencyUuids.push_back(step.agencyUuid);
    lineUuids.push_back(step.lineUuid);
    boardingNodesUuids.push_back(step.nodeUuid);
  }

  void StepToOdTripJsonVisitor::visitUnboardingStep(const UnboardingStep& step)
  {
    unboardingNodesUuids.push_back(step.nodeUuid);
  }

  void StepToOdTripJsonVisitor::visitWalkingStep(const WalkingStep& step)
  {
    // Nothing to do for walking steps
  }

  class ResultToOdTripJsonVisitor: public ResultVisitor<nlohmann::json> {
  private:
    nlohmann::json response;
    // TODO Instead of returning total travel time and legs, this visitor should do what needs to be done
    std::vector<std::tuple<int, int, int, int, int>> legs;
    int totalTravelTime;
    RouteParameters& params;
  public:
    ResultToOdTripJsonVisitor(RouteParameters& _params): params(_params) {
      // Nothing to initialize
    }
    nlohmann::json getResult() { return response; }
    std::vector<std::tuple<int, int, int, int, int>> getLegs() { return legs; }
    int getTotalTravelTime() { return totalTravelTime; }
    void visitSingleCalculationResult(const SingleCalculationResult& result) override;
    void visitAlternativesResult(const AlternativesResult& result) override;
    void visitAllNodesResult(const AllNodesResult& result) override;
  };

  void ResultToOdTripJsonVisitor::visitSingleCalculationResult(const SingleCalculationResult& result)
  {
    nlohmann::json odTripJson;
    odTripJson["status"]                        = STATUS_SUCCESS;
    odTripJson["originLat"]                     = params.getOrigin()->latitude;
    odTripJson["originLon"]                     = params.getOrigin()->longitude;
    odTripJson["destinationLat"]                = params.getDestination()->latitude;
    odTripJson["destinationLon"]                = params.getDestination()->longitude;
    odTripJson["travelTimeSeconds"]             = result.totalTravelTime;
    odTripJson["initialLostTimeAtDepartureSeconds"] = result.departureTime - params.getTimeOfTrip();
    odTripJson["departureTimeSeconds"]          = result.departureTime;
    odTripJson["arrivalTimeSeconds"]            = result.arrivalTime;
    odTripJson["numberOfTransfers"]             = result.numberOfTransfers;
    odTripJson["inVehicleTravelTimeSeconds"]    = result.totalInVehicleTime;
    odTripJson["transferTravelTimeSeconds"]     = result.transferWalkingTime;
    odTripJson["waitingTimeSeconds"]            = result.totalWaitingTime;
    odTripJson["accessTravelTimeSeconds"]       = result.accessTravelTime;
    odTripJson["egressTravelTimeSeconds"]       = result.egressTravelTime;
    odTripJson["transferWaitingTimeSeconds"]    = result.transferWaitingTime;
    odTripJson["firstWaitingTimeSeconds"]       = result.firstWaitingTime;
    odTripJson["nonTransitTravelTimeSeconds"]   = result.totalNonTransitTravelTime;

    // convert the steps
    StepToOdTripJsonVisitor stepVisitor = StepToOdTripJsonVisitor();
    for (auto &step : result.steps) {
      step.get()->accept(stepVisitor);
    }
    odTripJson["linesShortnames"]               = stepVisitor.getLineShortnames();
    odTripJson["agenciesAcronyms"]              = stepVisitor.getAgencyAcronyms();
    odTripJson["modesShortnames"]               = stepVisitor.getModeShortnames();
    odTripJson["agencyUuids"]                   = Toolbox::uuidsToStrings(stepVisitor.getAgencyUuids());
    odTripJson["lineUuids"]                     = Toolbox::uuidsToStrings(stepVisitor.getLineUuids());
    odTripJson["boardingNodeUuids"]             = Toolbox::uuidsToStrings(stepVisitor.getBoardingNodesUuids());
    odTripJson["unboardingNodeUuids"]           = Toolbox::uuidsToStrings(stepVisitor.getUnboardingNodesUuids());
    response = odTripJson;
    legs = result.legs;
    totalTravelTime = result.totalTravelTime;
  }

  void ResultToOdTripJsonVisitor::visitAlternativesResult(const AlternativesResult& result)
  {
    nlohmann::json json;
    // This type of result should not be visited here
    response = json;
  }

  void ResultToOdTripJsonVisitor::visitAllNodesResult(const AllNodesResult& result)
  {
    nlohmann::json json;
    // This type of result should not be visited here
    response = json;
  }

  nlohmann::json noRoutingFoundResultToJson(RouteParameters& params)
  {
    nlohmann::json json;
    json["status"]                     = STATUS_NO_ROUTING_FOUND;
    json["origin"]                     = { params.getOrigin()->latitude, params.getOrigin()->longitude };
    json["destination"]                = { params.getDestination()->latitude, params.getDestination()->longitude };
    json["departureTime"]              = Toolbox::convertSecondsToFormattedTime(params.getTimeOfTrip());
    json["departureTimeSeconds"]       = params.getTimeOfTrip();
    return json;
  }

  std::string Calculator::odTripsRouting(RouteParameters &parameters)
  {
    if (params.debugDisplay)
      std::cout << "  preparing odTripsRouting" << std::endl;

    std::unique_ptr<RoutingResult>  routingResult;
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

    if (params.debugDisplay)
      std::cout << "  starting odTripsRouting" << std::endl;

    int stopAtI {-1};
    std::vector<int> odTripIndexes(odTripsCount);
    std::cerr << odTripsCount << std::endl;
    std::iota(odTripIndexes.begin(), odTripIndexes.end(), 0);

    if (odTripsCount == 0)
    {
      return json.dump(2);
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
        RouteParameters odTripParameters = RouteParameters(std::make_unique<Point>(odTrip->origin.get()->latitude, odTrip->origin.get()->longitude),
          std::make_unique<Point>(odTrip->destination.get()->latitude, odTrip->destination.get()->longitude),
          parameters.getScenario(),
          parameters.getTimeOfTrip(),
          parameters.getMinWaitingTimeSeconds(),
          parameters.getMaxTotalTravelTimeSeconds(),
          parameters.getMaxAccessWalkingTravelTimeSeconds(),
          parameters.getMaxEgressWalkingTravelTimeSeconds(),
          parameters.getMaxTransferWalkingTravelTimeSeconds(),
          parameters.getMaxFirstWaitingTimeSeconds(),
          parameters.isWithAlternatives(),
          parameters.isForwardCalculation());

        float correctedExpansionFactor = odTrip->expansionFactor / params.odTripsSampleRatio;

        try {
          routingResult = calculate(odTripParameters, true, resetFilters); // reset filters only on first calculation
          resetFilters  = false;
          countOdTripsCalculated++;
          ResultToOdTripJsonVisitor visitor = ResultToOdTripJsonVisitor(odTripParameters);
          odTripJson = routingResult.get()->accept(visitor);
          
          atLeastOneOdTrip = true;
          if (visitor.getLegs().size() > 0)
          {
            totalTravelTimeSeconds += correctedExpansionFactor * visitor.getTotalTravelTime();
            for (auto & leg : visitor.getLegs())
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

        } catch (NoRoutingFoundException& e) {
          odTripJson = noRoutingFoundResultToJson(odTripParameters);
        }

        // Add additional fields to response
        odTripJson["uuid"] = boost::uuids::to_string(odTrip->uuid);
        odTripJson["internalId"]                    = odTrip->internalId;
        odTripJson["originActivity"]                = odTrip->originActivity;
        odTripJson["destinationActivity"]           = odTrip->destinationActivity;
        odTripJson["declaredMode"]                  = odTrip->mode;
        odTripJson["expansionFactor"]               = correctedExpansionFactor;
        odTripJson["onlyWalkingTravelTimeSeconds"]  = odTrip->walkingTravelTimeSeconds;
        odTripJson["onlyCyclingTravelTimeSeconds"]  = odTrip->cyclingTravelTimeSeconds;
        odTripJson["onlyDrivingTravelTimeSeconds"]  = odTrip->drivingTravelTimeSeconds;
        odTripJson["declaredDepartureTimeSeconds"]  = odTrip->departureTimeSeconds;
        odTripJson["declaredArrivalTimeSeconds"]    = odTrip->arrivalTimeSeconds;
        json["odTrips"].push_back(odTripJson);
      }

      //i++;
      if (params.odTripsSampleSize > 0 && i + 1 >= params.odTripsSampleSize)
      {
        break;
      }
    }

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
    return json.dump(2);

  }



}
