#include "calculator.hpp"
#include "json.hpp"

namespace TrRouting
{
    
  RoutingResult Calculator::forwardJourney(int bestArrivalTime, int bestEgressStopIndex, int bestEgressTravelTime)
  {
    RoutingResult result;
    nlohmann::json json;
    int stopsCount {1};
    std::vector<int> resultingStops;
    int i {0};
    bool foundRoute {false};

    if (params.returnAllStopsResult)
    {
      stopsCount = stops.size();
      resultingStops = std::vector<int>(stopsCount);
      std::iota (std::begin(resultingStops), std::end(resultingStops), 0); // generate sequencial indexes of each stops
    }
    else
    {
      if (bestArrivalTime < MAX_INT) // no route found
      {
        foundRoute = false;
      }
      std::vector<int> resultingStops;
      resultingStops = std::vector<int>(stopsCount);
    }

    if (false && (foundRoute || params.returnAllStopsResult))
    {

      std::deque<std::tuple<int,int,int,int,int,short>> journey;
      std::tuple<int,int,int,int,int,short>             subJourney;
      std::tuple<int,int,int,int,int,short>             emptyJourney {-1,-1,-1,-1,-1,-1};
      Stop  journeyStepStopDeparture;
      Stop  journeyStepStopArrival;
      Trip  journeyStepTrip;
      Route journeyStepRoute;
      std::tuple<int,int,int,int,int,short,short,int> journeyStepEnterConnection; // connection tuple: departureStopIndex, arrivalStopIndex, departureTimeSeconds, arrivalTimeSeconds, tripIndex, canBoard, canUnboard, sequenceinTrip
      std::tuple<int,int,int,int,int,short,short,int> journeyStepExitConnection;
      std::vector<std::tuple<unsigned long long, unsigned long long, unsigned long long, int, int>> legs; // tuple: tripId, routeId, routePathId, boarding sequence, unboarding sequence
      
      int totalInVehicleTime       { 0}; int transferArrivalTime      {-1}; int firstDepartureTime       {-1};
      int totalWalkingTime         { 0}; int transferReadyTime        {-1}; int minimizedDepartureTime   {-1};
      int totalWaitingTime         { 0}; int departureTime            {-1}; int numberOfTransfers        {-1};
      int totalTransferWalkingTime { 0}; int arrivalTime              {-1}; int boardingSequence         {-1};
      int totalTransferWaitingTime { 0}; int inVehicleTime            {-1}; int unboardingSequence       {-1};
      int journeyStepTravelTime    {-1}; int accessWalkingTime        {-1}; int bestAccessStopIndex      {-1};
      int transferTime             {-1}; int egressWalkingTime        {-1}; int reachableStopsCount      { 0};
      int waitingTime              {-1}; int accessWaitingTime        {-1}; 
  
      for (auto & resultingStopIndex : resultingStops)
      {
        
        //std::cerr << stops[resultingStopIndex].name;
        
        legs.clear();
        journey.clear();
        
        totalInVehicleTime       = 0;  transferArrivalTime      = -1; firstDepartureTime       = -1;
        totalWalkingTime         = 0;  transferReadyTime        = -1; minimizedDepartureTime   = -1;
        totalWaitingTime         = 0;  departureTime            = -1; numberOfTransfers        = -1;
        totalTransferWalkingTime = 0;  arrivalTime              = -1; boardingSequence         = -1;
        totalTransferWaitingTime = 0;  inVehicleTime            = -1; unboardingSequence       = -1;
        journeyStepTravelTime    = -1; accessWalkingTime        = -1; bestAccessStopIndex      = -1;
        transferTime             = -1; egressWalkingTime        = -1; 
        waitingTime              = -1; accessWaitingTime        = -1; 
        
        // recreate journey:
        subJourney = forwardJourneys[resultingStopIndex];
  
        if (subJourney == emptyJourney) // ignore stops with no route
        {
          continue;
        }
        
        i = 0;
        while ((std::get<0>(subJourney) != -1 && std::get<1>(subJourney) != -1))
        {
          journey.push_front(subJourney);
          bestAccessStopIndex = std::get<connectionIndexes::STOP_DEP>(forwardConnections[std::get<0>(subJourney)]);
          //std::cerr << "sequence: " << std::get<connectionIndexes::SEQUENCE>(forwardConnections[std::get<0>(subJourney)]) << " sequence2: " << std::get<connectionIndexes::SEQUENCE>(forwardConnections[std::get<1>(subJourney)]) << " stop:" << stops[bestAccessStopIndex].name << " tenterc:" <<  std::get<0>(subJourney) << " texitc:" <<  std::get<1>(subJourney) << " jss:" <<  std::get<5>(subJourney) << " jtt:" << std::get<4>(subJourney) << " tectt:" << tripsEnterConnectionTransferTravelTime[std::get<3>(subJourney)] << std::endl;
          //std::cerr << stops[bestAccessStopIndex].name << " > " << stops[std::get<connectionIndexes::STOP_ARR>(forwardConnections[std::get<1>(subJourney)])].name << std::endl;
          subJourney = forwardJourneys[bestAccessStopIndex];
          i++;
        }
        
        if (!params.returnAllStopsResult)
        {
          journey.push_back(std::make_tuple(-1,-1,-1,-1,stopsEgressTravelTime[resultingStopIndex],-1));
        }
        journey.push_front(std::make_tuple(-1,-1,-1,-1,stopsAccessTravelTime[bestAccessStopIndex],-1));
        
        std::string stepsJson = "  \"steps\":\n  [\n";
       
        i = 0;
        int journeyStepsCount = journey.size();
        for (auto & journeyStep : journey)
        {
          
          if (std::get<0>(journeyStep) != -1 && std::get<1>(journeyStep) != -1)
          {
            // journey tuple: final enter connection, final exit connection, final footpath
            journeyStepEnterConnection = forwardConnections[std::get<0>(journeyStep)];
            journeyStepExitConnection  = forwardConnections[std::get<1>(journeyStep)];
            journeyStepStopDeparture   = stops[std::get<connectionIndexes::STOP_DEP>(journeyStepEnterConnection)];
            journeyStepStopArrival     = stops[std::get<connectionIndexes::STOP_ARR>(journeyStepExitConnection)];
            journeyStepTrip            = trips[std::get<3>(journeyStep)];
            journeyStepRoute           = routes[routeIndexesById[journeyStepTrip.routeId]];
            transferTime               = std::get<4>(journeyStep);
            departureTime              = std::get<connectionIndexes::TIME_DEP>(journeyStepEnterConnection);
            arrivalTime                = std::get<connectionIndexes::TIME_ARR>(journeyStepExitConnection);
            boardingSequence           = std::get<connectionIndexes::SEQUENCE>(journeyStepEnterConnection);
            unboardingSequence         = std::get<connectionIndexes::SEQUENCE>(journeyStepExitConnection);
            inVehicleTime              = arrivalTime - departureTime;
            waitingTime                = departureTime - transferArrivalTime;
            transferArrivalTime        = arrivalTime + transferTime;
            transferReadyTime          = transferArrivalTime;
            totalInVehicleTime         += inVehicleTime;
            totalWaitingTime           += waitingTime;
            numberOfTransfers          += 1;
            legs.push_back(std::make_tuple(journeyStepTrip.id, journeyStepTrip.routeId, journeyStepTrip.routePathId, boardingSequence, unboardingSequence));
            
            if (i == 1) // first leg
            {
              accessWaitingTime  = waitingTime;
              firstDepartureTime = departureTime;
            }
            else
            {
              totalTransferWaitingTime += waitingTime;
            }
            
            if (!params.returnAllStopsResult)
            {
              stepsJson += "    {\n"
              "      \"action\": \"board\",\n"
              "      \"agencyAcronym\": \""             + journeyStepRoute.agencyAcronym + "\",\n"
              "      \"agencyName\": \""                + journeyStepRoute.agencyName + "\",\n"
              "      \"agencyId\": "                    + std::to_string(journeyStepRoute.agencyId) + ",\n"
              "      \"routeShortname\": \""            + journeyStepRoute.shortname + "\",\n"
              "      \"routeLongname\": \""             + journeyStepRoute.longname + "\",\n"
              "      \"routeId\": "                     + std::to_string(journeyStepRoute.id) + ",\n"
              "      \"routeTypeName\": \""             + journeyStepRoute.routeTypeName + "\",\n"
              "      \"routeTypeId\": "                 + std::to_string(journeyStepRoute.routeTypeId) + ",\n"
              "      \"tripId\": "                      + std::to_string(journeyStepTrip.id) + ",\n"
              "      \"sequenceInTrip\": "              + std::to_string(boardingSequence) + ",\n"
              "      \"stopName\": \""                  + journeyStepStopDeparture.name + "\",\n"
              "      \"stopCode\": \""                  + journeyStepStopDeparture.code + "\",\n"
              "      \"stopId\": "                      + std::to_string(journeyStepStopDeparture.id) + ",\n"
              "      \"stopCoordinates\": ["            + std::to_string(journeyStepStopDeparture.point.latitude) + "," + std::to_string(journeyStepStopDeparture.point.longitude) + "],\n"
              "      \"departureTime\": \""             + Toolbox::convertSecondsToFormattedTime(departureTime) + "\",\n"
              "      \"departureTimeSeconds\": "        + std::to_string(departureTime) + ",\n"
              //"      \"enterConnectionI\": \"" + std::to_string(std::get<0>(journeyStep)) + "\",\n"
              "      \"waitingTimeSeconds\":"           + std::to_string(waitingTime) + ",\n"
              "      \"waitingTimeMinutes\":"           + std::to_string(Toolbox::convertSecondsToMinutes(waitingTime)) + "\n"
              "    },\n"
              "    {\n"
              "      \"action\": \"unboard\",\n"
              "      \"agencyAcronym\": \""             + journeyStepRoute.agencyAcronym + "\",\n"
              "      \"agencyName\": \""                + journeyStepRoute.agencyName + "\",\n"
              "      \"agencyId\": "                    + std::to_string(journeyStepRoute.agencyId) + ",\n"
              "      \"routeShortname\": \""            + journeyStepRoute.shortname + "\",\n"
              "      \"routeLongname\": \""             + journeyStepRoute.longname + "\",\n"
              "      \"routeId\": "                     + std::to_string(journeyStepRoute.id) + ",\n"
              "      \"routeTypeName\": \""             + journeyStepRoute.routeTypeName + "\",\n"
              "      \"routeTypeId\": "                 + std::to_string(journeyStepRoute.routeTypeId) + ",\n"
              "      \"tripId\": "                      + std::to_string(journeyStepTrip.id) + ",\n"
              "      \"sequenceInTrip\": "              + std::to_string(unboardingSequence) + ",\n"
              "      \"stopName\": \""                  + journeyStepStopArrival.name + "\",\n"
              "      \"stopCode\": \""                  + journeyStepStopArrival.code + "\",\n"
              "      \"stopId\": "                      + std::to_string(journeyStepStopArrival.id) + ",\n"
              "      \"stopCoordinates\": ["            + std::to_string(journeyStepStopArrival.point.latitude) + "," + std::to_string(journeyStepStopArrival.point.longitude) + "],\n"
              "      \"arrivalTime\": \""               + Toolbox::convertSecondsToFormattedTime(arrivalTime) + "\",\n"
              "      \"arrivalTimeSeconds\": "          + std::to_string(arrivalTime) + ",\n"
              //"      \"exitConnectionI\": \"" + std::to_string(std::get<1>(journeyStep)) + "\",\n"
              "      \"segmentInVehicleTimeMinutes\":"  + std::to_string(Toolbox::convertSecondsToMinutes(inVehicleTime)) + ",\n"
              "      \"segmentInVehicleTimeSeconds\":"  + std::to_string(inVehicleTime) + "\n"
              "    },\n";
              
            }
            if (i < journeyStepsCount - 2) // if not the last transit leg
            {
              totalTransferWalkingTime += transferTime;
              totalWalkingTime         += transferTime;
              if (!params.returnAllStopsResult)
              {
                stepsJson += "    {\n"
                "      \"action\": \"walking\",\n"
                "      \"type\": \"transfer\",\n"
                "      \"travelTimeSeconds\": "    + std::to_string(transferTime) + ",\n"
                "      \"travelTimeMinutes\": "    + std::to_string(Toolbox::convertSecondsToMinutes(transferTime)) + ",\n"
                "      \"departureTime\": \""      + Toolbox::convertSecondsToFormattedTime(arrivalTime) + "\",\n"
                "      \"arrivalTime\": \""        + Toolbox::convertSecondsToFormattedTime(transferArrivalTime) + "\",\n"
                "      \"departureTimeSeconds\": " + std::to_string(arrivalTime) + ",\n"
                "      \"arrivalTimeSeconds\": "   + std::to_string(transferArrivalTime) + ",\n"
                "      \"readyToBoardAt\": \""     + Toolbox::convertSecondsToFormattedTime(transferReadyTime + params.minWaitingTimeSeconds) + "\"\n"
                "    },\n";
              }
            }
          }
          else // access or egress journey step
          {
            
            transferTime = std::get<4>(journeyStep);
            if (i == 0) // access
            {
              transferArrivalTime  = departureTimeSeconds + transferTime;
              transferReadyTime    = transferArrivalTime;
              totalWalkingTime    += transferTime;
              accessWalkingTime    = transferTime;
              if (!params.returnAllStopsResult)
              {
                stepsJson += "    {\n"
                "      \"action\": \"walking\",\n"
                "      \"type\": \"access\",\n"
                "      \"travelTimeSeconds\": "    + std::to_string(transferTime) + ",\n"
                "      \"travelTimeMinutes\": "    + std::to_string(Toolbox::convertSecondsToMinutes(transferTime)) + ",\n"
                "      \"departureTime\": \""      + Toolbox::convertSecondsToFormattedTime(departureTimeSeconds) + "\",\n"
                "      \"arrivalTime\": \""        + Toolbox::convertSecondsToFormattedTime(transferArrivalTime) + "\",\n"
                "      \"departureTimeSeconds\": " + std::to_string(departureTimeSeconds) + ",\n"
                "      \"arrivalTimeSeconds\": "   + std::to_string(transferArrivalTime) + ",\n"
                "      \"readyToBoardAt\": \""     + Toolbox::convertSecondsToFormattedTime(transferReadyTime + params.minWaitingTimeSeconds) + "\"\n"
                "    },\n";
              }
            }
            else // egress
            {
              totalWalkingTime   += transferTime;
              egressWalkingTime   = transferTime;
              transferArrivalTime = arrivalTime + transferTime;
              if (!params.returnAllStopsResult)
              {
                stepsJson += "    {\n"
                "      \"action\": \"walking\",\n"
                "      \"type\": \"egress\",\n"
                "      \"travelTimeSeconds\": "    + std::to_string(transferTime) + ",\n"
                "      \"travelTimeMinutes\": "    + std::to_string(Toolbox::convertSecondsToMinutes(transferTime)) + ",\n"
                "      \"departureTime\": \""      + Toolbox::convertSecondsToFormattedTime(arrivalTime) + "\",\n"
                "      \"arrivalTime\": \""        + Toolbox::convertSecondsToFormattedTime(arrivalTime + transferTime) + "\",\n"
                "      \"departureTimeSeconds\": " + std::to_string(arrivalTime) + ",\n"
                "      \"arrivalTimeSeconds\": "   + std::to_string(arrivalTime + transferTime) + "\n"
                "    }\n";
              }
              arrivalTime = transferArrivalTime;
            }
          }
          i++;
        }
        
        minimizedDepartureTime = firstDepartureTime - accessWalkingTime - params.minWaitingTimeSeconds;
        
        if (params.returnAllStopsResult)
        {
          arrivalTime = stopsTentativeTime[resultingStopIndex] - params.minWaitingTimeSeconds;
          if (arrivalTime - departureTimeSeconds <= params.maxTotalTravelTimeSeconds)
          {
            reachableStopsCount++;
            result.json += "    { "
            " \"id\": " + std::to_string(stops[resultingStopIndex].id) + ", "
            " \"arrivalTime\": \""   + Toolbox::convertSecondsToFormattedTime(arrivalTime) + "\", "
            " \"totalTravelTimeSeconds\": " + std::to_string(arrivalTime - departureTimeSeconds) + ", "
            " \"numberOfTransfers\": " + std::to_string(numberOfTransfers) + "},\n";
          }
        }
        else if (!params.returnAllStopsResult)
        {
          if (numberOfTransfers >= 0)
          {
            result.json += "{\n  \"status\": \"success\",\n"
            "  \"origin\": ["      + std::to_string(params.origin.latitude) + "," + std::to_string(params.origin.longitude) + "],\n"
            "  \"destination\": [" + std::to_string(params.destination.latitude) + "," + std::to_string(params.destination.longitude) + "],\n"
            //"  \"totalWalkingTimeMinutesIfWalkingOnly\": "        + std::to_string(totalOnlyWalkingTimeMinutes) + ",\n"
            "  \"departureTime\": \""                             + Toolbox::convertSecondsToFormattedTime(departureTimeSeconds) + "\",\n"
            "  \"arrivalTime\": \""                               + Toolbox::convertSecondsToFormattedTime(arrivalTime) + "\",\n"
            "  \"departureTimeSeconds\": "                        + std::to_string(departureTimeSeconds) + ",\n"
            "  \"arrivalTimeSeconds\": "                          + std::to_string(arrivalTime) + ",\n"
            "  \"totalTravelTimeMinutes\": "                      + std::to_string(Toolbox::convertSecondsToMinutes(arrivalTime - departureTimeSeconds)) + ",\n"
            "  \"totalTravelTimeSeconds\": "                      + std::to_string(arrivalTime - departureTimeSeconds) + ",\n"
            "  \"totalInVehicleTimeMinutes\": "                   + std::to_string(Toolbox::convertSecondsToMinutes(totalInVehicleTime)) + ",\n"
            "  \"totalInVehicleTimeSeconds\": "                   + std::to_string(totalInVehicleTime) + ",\n"
            "  \"totalNonTransitTravelTimeMinutes\": "            + std::to_string(Toolbox::convertSecondsToMinutes(totalWalkingTime)) + ",\n"
            "  \"totalNonTransitTravelTimeSeconds\": "            + std::to_string(totalWalkingTime) + ",\n"
            "  \"numberOfBoardings\": "                           + std::to_string(numberOfTransfers + 1) + ",\n"
            "  \"numberOfTransfers\": "                           + std::to_string(numberOfTransfers) + ",\n"
            //"  \"maxNumberOfTransfers\": "                        + std::to_string(params.maxNumberOfTransfers) + ",\n"
            "  \"transferWalkingTimeMinutes\": "                  + std::to_string(Toolbox::convertSecondsToMinutes(totalTransferWalkingTime)) + ",\n"
            "  \"transferWalkingTimeSeconds\": "                  + std::to_string(totalTransferWalkingTime) + ",\n"
            "  \"accessTravelTimeMinutes\": "                     + std::to_string(Toolbox::convertSecondsToMinutes(accessWalkingTime)) + ",\n"
            "  \"accessTravelTimeSeconds\": "                     + std::to_string(accessWalkingTime) + ",\n"
            "  \"egressTravelTimeMinutes\": "                     + std::to_string(Toolbox::convertSecondsToMinutes(egressWalkingTime)) + ",\n"
            "  \"egressTravelTimeSeconds\": "                     + std::to_string(egressWalkingTime) + ",\n"
            "  \"transferWaitingTimeMinutes\": "                  + std::to_string(Toolbox::convertSecondsToMinutes(totalTransferWaitingTime)) + ",\n"
            "  \"transferWaitingTimeSeconds\": "                  + std::to_string(totalTransferWaitingTime) + ",\n"
            "  \"firstWaitingTimeMinutes\": "                     + std::to_string(Toolbox::convertSecondsToMinutes(accessWaitingTime)) + ",\n"
            "  \"firstWaitingTimeSeconds\": "                     + std::to_string(accessWaitingTime) + ",\n"
            "  \"totalWaitingTimeMinutes\": "                     + std::to_string(Toolbox::convertSecondsToMinutes(totalWaitingTime)) + ",\n"
            "  \"totalWaitingTimeSeconds\": "                     + std::to_string(totalWaitingTime) + ",\n"
            "  \"departureTimeToMinimizeFirstWaitingTime\": \""   + Toolbox::convertSecondsToFormattedTime(minimizedDepartureTime) + "\",\n"
            "  \"minimizedTotalTravelTimeMinutes\": "             + std::to_string(Toolbox::convertSecondsToMinutes(arrivalTime - minimizedDepartureTime)) + ",\n"
            "  \"minimizedTotalTravelTimeSeconds\": "             + std::to_string(arrivalTime - minimizedDepartureTime) + ",\n"
            "  \"minimumWaitingTimeBeforeEachBoardingMinutes\": " + std::to_string(Toolbox::convertSecondsToMinutes(params.minWaitingTimeSeconds)) + ",\n"
            "  \"minimumWaitingTimeBeforeEachBoardingSeconds\": " + std::to_string(params.minWaitingTimeSeconds) + ",\n";
            result.json += stepsJson + "\n  ]\n}";
            result.travelTimeSeconds    = arrivalTime - departureTimeSeconds;
            result.arrivalTimeSeconds   = arrivalTime;
            result.departureTimeSeconds = departureTimeSeconds;
            result.numberOfTransfers    = numberOfTransfers;
            result.legs                 = legs;
            result.status               = "success";
            
          }
          else
          {
            result.status = "no_routing_found";
            result.travelTimeSeconds = -1;
            result.json += "{\n"
            "  \"status\": \"no_routing_found\",\n"
            "  \"origin\": ["                                     + std::to_string(params.origin.latitude) + "," + std::to_string(params.origin.longitude) + "],\n"
            "  \"destination\": ["                                + std::to_string(params.destination.latitude) + "," + std::to_string(params.destination.longitude) + "],\n"
            //"  \"totalWalkingTimeMinutesIfWalkingOnly\": "        + std::to_string(totalOnlyWalkingTimeMinutes) + ",\n"
            "  \"departureTime\": "                                + Toolbox::convertSecondsToFormattedTime(departureTimeSeconds) + "\n"
            "\n}";
          }
          
        }
        
      }

    }
    else // no route found
    {
      json["status"]               = "no_routing_found";
      json["origin"]               = { params.origin.longitude, params.origin.latitude };
      json["destination"]          = { params.destination.longitude, params.destination.latitude };
      json["departureTime"]        = Toolbox::convertSecondsToFormattedTime(departureTimeSeconds);
      json["departureTimeSeconds"] = departureTimeSeconds;
      result.status                = "no_routing_found";
      result.travelTimeSeconds     = -1;
    }



    //if (params.returnAllStopsResult)
    //{
    //  result.json.pop_back(); result.json.pop_back(); // remove trailing comma and newline
    //  result.json += "\n  ],\n"
    //  "  \"numberOfReachableStops\": "  + std::to_string(reachableStopsCount) + ",\n"
    //  "  \"percentOfReachableStops\": " + std::to_string(round(10000 * (float)reachableStopsCount / (float)(stopsCount))/100.0) + "\n"
    //  "}";
    //}



    result.json = json.dump(2); // number of spaces in indent for human readable json, use dump() to put all json content on the same line
    return result;
  }

}