#include "calculator.hpp"

namespace TrRouting
{
  
  RoutingResult Calculator::calculate() {
    
    long long calculationTime {algorithmCalculationTime.getDurationMicrosecondsNoStop()};
    
    reset();
    
    if(params.odTrip != NULL)
    {
      departureTimeSeconds = params.odTrip->departureTimeSeconds;
    }

    RoutingResult result;
    
    result.json = "";
    
    int i {0};
    int connectionsCount = forwardConnections.size();
    int reachableConnectionsCount {0};
    int tripIndex {-1};
    int stopDepartureIndex {-1};
    int stopArrivalIndex {-1};
    int tripEnterConnectionIndex {-1};
    int stopDepartureTentativeTime {MAX_INT};
    int stopArrivalTentativeTime {MAX_INT};
    int connectionDepartureTime {-1};
    int connectionArrivalTime {-1};
    int footpathsRangeStart {-1};
    int footpathsRangeEnd {-1};
    int footpathIndex {-1};
    int footpathStopArrivalIndex {-1};
    int footpathTravelTime {-1};
    int bestEgressStopIndex {-1};
    int bestArrivalTime {MAX_INT};
    int maxEgressTravelTime {-1};
    int minAccessTravelTime {MAX_INT};
    int tentativeEgressStopArrivalTime {MAX_INT};
    int stopsCount = stops.size();
    std::tuple<int,int,int,int,int,short> emptyJourney {-1,-1,-1,-1,-1,-1};
    
    // find first reachable connection:
    bool reachedAtLeastOneEgressStop {false};
    
    std::cerr << "-- reset and preparations -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
    
    // fetch stops footpaths accessible from origin using params or osrm fetcher if not provided:
    if(params.odTrip != NULL)
    {
      accessFootpaths = params.odTrip->accessFootpaths;
    }
    else if (params.accessStopIds.size() > 0 && params.accessStopTravelTimesSeconds.size() == params.accessStopIds.size())
    {
      i = 0;
      for (auto & accessStopId : params.accessStopIds)
      {
        accessFootpaths.push_back(std::make_pair(stopIndexesById[accessStopId], params.accessStopTravelTimesSeconds[i]));
        i++;
      }
    }
    else
    {
      accessFootpaths = OsrmFetcher::getAccessibleStopsFootpathsFromPoint(params.origin, stops, params.accessMode, params.maxAccessWalkingTravelTimeSeconds, params.walkingSpeedMetersPerSecond, params.osrmRoutingWalkingHost, params.osrmRoutingWalkingPort);
    }

    for (auto & accessFootpath : accessFootpaths)
    {
      stopsAccessTravelTime[accessFootpath.first] = accessFootpath.second;
      journeys[accessFootpath.first]              = std::make_tuple(-1, -1, -1, -1, accessFootpath.second, -1);
      stopsTentativeTime[accessFootpath.first]    = departureTimeSeconds + accessFootpath.second + params.minWaitingTimeSeconds;
      if (accessFootpath.second < minAccessTravelTime)
      {
        minAccessTravelTime = accessFootpath.second;
      }
      //result.json += "origin_stop: " + stops[accessFootpath.first].name + " - " + Toolbox::convertSecondsToFormattedTime(stopsTentativeTime[accessFootpath.first]) + "\n";
      //result.json += std::to_string(stops[accessFootpath.first].id) + ",";
    }
    
    if (!params.returnAllStopsResult)
    {
      // fetch stops footpaths accessible to destination using params or osrm fetcher if not provided:
      if(params.odTrip != NULL)
      {
        egressFootpaths = params.odTrip->egressFootpaths;
      }
      else if (params.egressStopIds.size() > 0 && params.egressStopTravelTimesSeconds.size() == params.egressStopIds.size())
      {
        egressFootpaths.reserve(params.egressStopIds.size());
        i = 0;
        for (auto & egressStopId : params.egressStopIds)
        {
          egressFootpaths.push_back(std::make_pair(stopIndexesById[egressStopId], params.egressStopTravelTimesSeconds[i]));
          i++;
        }
      }
      else
      {
        egressFootpaths = OsrmFetcher::getAccessibleStopsFootpathsFromPoint(params.destination, stops, params.accessMode, params.maxEgressWalkingTravelTimeSeconds, params.walkingSpeedMetersPerSecond, params.osrmRoutingWalkingHost, params.osrmRoutingWalkingPort);
      }
      for (auto & egressFootpath : egressFootpaths)
      {
        if (egressFootpath.second > maxEgressTravelTime)
        {
          maxEgressTravelTime = egressFootpath.second;
        }
        stopsEgressTravelTime[egressFootpath.first] = egressFootpath.second;
        //result.json += "origin_stop: " + stops[accessFootpath.first].name + " - " + Toolbox::convertSecondsToFormattedTime(stopsTentativeTime[accessFootpath.first]) + "\n";
        //result.json += std::to_string((int)(ceil(egressFootpath.second))) + ",";
      }
    }
    
    std::cerr << "-- access and egress footpaths -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
    
    std::cerr << "-- maxEgressTravelTime = " << maxEgressTravelTime << std::endl;
    
    // main loop:
    i = 0;
    for(auto & connection : forwardConnections)
    {

      // ignore connections before departure time + minimum access travel time:
      if (std::get<connectionIndexes::TIME_DEP>(connection) >= departureTimeSeconds + minAccessTravelTime)
      {
        tripIndex = std::get<connectionIndexes::TRIP>(connection);
        
        // enabled trips only here:
        if (tripsEnabled[tripIndex] != -1)
        {
          connectionDepartureTime = std::get<connectionIndexes::TIME_DEP>(connection);
          
          // no need to parse next connections if already reached destination from all egress stops:
          if ( (!params.returnAllStopsResult && reachedAtLeastOneEgressStop && maxEgressTravelTime >= 0 && connectionDepartureTime > tentativeEgressStopArrivalTime + maxEgressTravelTime) || (connectionDepartureTime - departureTimeSeconds > params.maxTotalTravelTimeSeconds))
          {
            break;
          }
          tripEnterConnectionIndex   = tripsEnterConnection[tripIndex];
          stopDepartureIndex         = std::get<connectionIndexes::STOP_DEP>(connection);
          stopDepartureTentativeTime = stopsTentativeTime[stopDepartureIndex];
          
          // reachable connections only here:
          if (tripEnterConnectionIndex != -1 || stopDepartureTentativeTime <= connectionDepartureTime)
          {
            
            //std::get<5>(journeys[stopDepartureIndex]) != 1 
            
            if (std::get<connectionIndexes::CAN_BOARD>(connection) == 1 && (tripEnterConnectionIndex == -1 || (std::get<0>(journeys[stopDepartureIndex]) == -1 && std::get<4>(journeys[stopDepartureIndex]) >= 0 && std::get<4>(journeys[stopDepartureIndex]) < tripsEnterConnectionTransferTravelTime[tripIndex])))
            //( tripEnterConnectionIndex == -1) || (std::get<5>(journeys[stopDepartureIndex]) != 1 && (std::get<0>(journeys[stopDepartureIndex]) == -1 || std::get<connectionIndexes::TRIP>(forwardConnections[std::get<0>(journeys[stopDepartureIndex])]) == tripIndex) && std::get<4>(journeys[stopDepartureIndex]) >= 0 && std::get<4>(journeys[stopDepartureIndex]) < tripsEnterConnectionTransferTravelTime[tripIndex]))
            //)
            {
              //if (tripEnterConnectionIndex != -1)
              //{
              //  std::cerr << "from_stop: " << stops[std::get<0>(footpaths[std::get<2>(journeys[stopDepartureIndex])])].name << " route:" << routes[routeIndexesById[trips[tripIndex].routeId]].shortname << " " << routes[routeIndexesById[trips[tripIndex].routeId]].longname << " old stop:" << stops[std::get<connectionIndexes::STOP_DEP>(forwardConnections[std::get<0>(journeys[stopDepartureIndex])])].name << " stop:" << stops[stopDepartureIndex].name << " tec:" <<  tripEnterConnectionIndex << " i:" <<  i << " jss:" <<  std::get<5>(journeys[stopDepartureIndex]) << " jenterc:" << std::get<0>(journeys[stopDepartureIndex]) << " jexitc:" << std::get<1>(journeys[stopDepartureIndex]) << " jtt:" << std::get<4>(journeys[stopDepartureIndex]) << " tectt:" << tripsEnterConnectionTransferTravelTime[tripIndex] << std::endl;
              //}
              tripsEnterConnection[tripIndex]                   = i;
              tripsEnterConnectionTransferTravelTime[tripIndex] = std::get<4>(journeys[stopDepartureIndex]);
            }
            
            if (std::get<connectionIndexes::CAN_UNBOARD>(connection) == 1 && tripsEnterConnection[tripIndex] != -1)
            {
              // get footpaths for the arrival stop to get transferable stops:
              stopArrivalIndex      = std::get<connectionIndexes::STOP_ARR>(connection);
              connectionArrivalTime = std::get<connectionIndexes::TIME_ARR>(connection);
              footpathsRangeStart   = footpathsRanges[stopArrivalIndex].first;
              footpathsRangeEnd     = footpathsRanges[stopArrivalIndex].second;
              if (!params.returnAllStopsResult && !reachedAtLeastOneEgressStop && stopsEgressTravelTime[stopArrivalIndex] != -1) // check if the arrival stop is egressable
              {
                reachedAtLeastOneEgressStop    = true;
                tentativeEgressStopArrivalTime = connectionArrivalTime;
              }
              if (footpathsRangeStart >= 0 && footpathsRangeEnd >= 0)
              {
                footpathIndex = footpathsRangeStart;
                while (footpathIndex <= footpathsRangeEnd)
                {
                  footpathStopArrivalIndex = std::get<1>(footpaths[footpathIndex]);
                  footpathTravelTime       = std::get<2>(footpaths[footpathIndex]);
                  
                  if (footpathTravelTime <= params.maxTransferWalkingTravelTimeSeconds && footpathTravelTime + params.minWaitingTimeSeconds + connectionArrivalTime < stopsTentativeTime[footpathStopArrivalIndex])
                  {
                    stopsTentativeTime[footpathStopArrivalIndex] = footpathTravelTime + connectionArrivalTime + params.minWaitingTimeSeconds;
                    journeys[footpathStopArrivalIndex]           = std::make_tuple(tripsEnterConnection[tripIndex], i, footpathIndex, tripIndex, footpathTravelTime, (stopArrivalIndex == footpathStopArrivalIndex ? 1 : -1));
                  }
                  footpathIndex++;
                }
              }
            }
            reachableConnectionsCount++;
          }
        }
      }
      i++;
    }
    
    std::cerr << "-- " << reachableConnectionsCount << " connections parsed on " << connectionsCount << std::endl;
    
    std::cerr << "-- main calculation -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
        
    std::vector<int> resultingStops(stopsCount);
    
    if (params.returnAllStopsResult)
    {
      resultingStops = std::vector<int>(stopsCount);
      std::iota (std::begin(resultingStops), std::end(resultingStops), 0); // generate sequencial indexes of each stops
      result.json += "{\n"
      "  \"stops\":\n  [\n";
    }
    else
    {
      i = 0;
      resultingStops = std::vector<int>(1);
      for (auto & arrivalTime : stopsTentativeTime)
      {
        if (stopsEgressTravelTime[i] >= 0 && arrivalTime < MAX_INT && arrivalTime + stopsEgressTravelTime[i] < bestArrivalTime)
        {
          bestArrivalTime     = arrivalTime + stopsEgressTravelTime[i];
          bestEgressStopIndex = i;
        }
        i++;
      }
      
      if (bestEgressStopIndex == -1) // no routing found
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
        return result;
      }
      else
      {
        resultingStops[0] = bestEgressStopIndex;
      }
      
      std::cerr << "-- find best journey -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
      calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
      
    }
    
    
    
    std::deque<std::tuple<int,int,int,int,int,short>> journey;
    std::tuple<int,int,int,int,int,short>             subJourney;

    Stop  journeyStepStopDeparture;
    Stop  journeyStepStopArrival;
    Trip  journeyStepTrip;
    Route journeyStepRoute;
    std::tuple<int,int,int,int,int,short,short,int> journeyStepEnterConnection; // connection tuple: departureStopIndex, arrivalStopIndex, departureTimeSeconds, arrivalTimeSeconds, tripIndex, canBoard, canUnboard, sequenceinTrip
    std::tuple<int,int,int,int,int,short,short,int> journeyStepExitConnection;
    std::vector<std::tuple<unsigned long long, unsigned long long, unsigned long long, int, int>> legs; // tuple: tripId, routeId, routePathId, boarding sequence, unboarding sequence
    int   journeyStepTravelTime    {-1};
    int   transferTime             {-1};
    int   waitingTime              {-1};
    int   transferArrivalTime      {-1};
    int   transferReadyTime        {-1};
    int   departureTime            {-1};
    int   arrivalTime              {-1};
    int   inVehicleTime            {-1};
    int   totalInVehicleTime       { 0};
    int   totalWalkingTime         { 0};
    int   totalWaitingTime         { 0};
    int   totalTransferWalkingTime { 0};
    int   totalTransferWaitingTime { 0};
    int   accessWalkingTime        {-1};
    int   egressWalkingTime        {-1};
    int   accessWaitingTime        {-1};
    int   firstDepartureTime       {-1};
    int   minimizedDepartureTime   {-1};
    int   numberOfTransfers        {-1};
    int   bestAccessStopIndex      {-1};
    int   reachableStopsCount      { 0};
    int   boardingSequence         {-1};
    int   unboardingSequence       {-1};
    
    std::cerr << "-- start parsing stops -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
    
    for (auto & resultingStopIndex : resultingStops)
    {
      
      //std::cerr << stops[resultingStopIndex].name;
      
      legs.clear();
      journey.clear();
      journeyStepTravelTime    = -1;
      transferTime             = -1;
      waitingTime              = -1;
      transferArrivalTime      = -1;
      transferReadyTime        = -1;
      departureTime            = -1;
      arrivalTime              = -1;
      inVehicleTime            = -1;
      totalInVehicleTime       = 0;
      totalWalkingTime         = 0;
      totalWaitingTime         = 0;
      totalTransferWalkingTime = 0;
      totalTransferWaitingTime = 0;
      accessWalkingTime        = -1;
      egressWalkingTime        = -1;
      accessWaitingTime        = -1;
      firstDepartureTime       = -1;
      minimizedDepartureTime   = -1;
      numberOfTransfers        = -1;
      bestAccessStopIndex      = -1;
      boardingSequence         = -1;
      unboardingSequence       = -1;
      
      // recreate journey:
      subJourney = journeys[resultingStopIndex];
      
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
        subJourney = journeys[bestAccessStopIndex];
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
      
      if (params.returnAllStopsResult && numberOfTransfers >= 0)
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
    
    if (params.returnAllStopsResult)
    {
      result.json.pop_back(); result.json.pop_back(); // remove trailing comma and newline
      result.json += "\n  ],\n"
      "  \"numberOfReachableStops\": "  + std::to_string(reachableStopsCount) + ",\n"
      "  \"percentOfReachableStops\": " + std::to_string(round(10000 * (float)reachableStopsCount / (float)(stopsCount))/100.0) + "\n"
      "}";
    }
    
    std::cerr << "-- journey conversion -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
    
    return result;
    
  }
  
}
