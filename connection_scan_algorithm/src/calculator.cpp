#include "calculator.hpp"

namespace TrRouting
{
  
  RoutingResult Calculator::calculate() {
    
    long long calculationTime {algorithmCalculationTime.getDurationMicrosecondsNoStop()};
    
    reset();
    
    RoutingResult result;
    std::deque<std::tuple<int,int,int,int,int,short>> journey;
    std::tuple<int,int,int,int,int,short> subJourney;
    std::tuple<int,int,int,int,int,short> emptyJourney {-1,-1,-1,-1,-1,-1};
    
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
    int bestAccessStopIndex {-1};
    int bestEgressStopIndex {-1};
    int bestArrivalTime {MAX_INT};
    int maxEgressTravelTime {-1};
    int minAccessTravelTime {MAX_INT};
    int tentativeEgressStopArrivalTime = {MAX_INT};
    
    // find first reachable connection:
    bool reachedAtLeastOneEgressStop {false};
    
    std::cerr << "-- reset and preparations -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
        
    // fetch stops footpaths accessible from origin using params or osrm fetcher if not provided:
    if (params.accessStopIds.size() > 0 && params.accessStopTravelTimesSeconds.size() == params.accessStopIds.size())
    {
      accessFootpaths.reserve(params.accessStopIds.size());
      i = 0;
      for (auto & accessStopId : params.accessStopIds)
      {
        accessFootpaths.push_back(std::make_pair(stopIndexesById[accessStopId], params.accessStopTravelTimesSeconds[i]));
        i++;
      }
    }
    else
    {
      accessFootpaths = OsrmFetcher::getAccessibleStopsFootpathsFromPoint(params.origin, stops, params, params.accessMode, params.maxAccessWalkingTravelTimeSeconds);
    }
    for (auto & accessFootpath : accessFootpaths)
    {
      stopsAccessTravelTime[accessFootpath.first] = accessFootpath.second + params.minWaitingTimeSeconds;
      journeys[accessFootpath.first]              = std::make_tuple(-1, -1, -1, -1, accessFootpath.second,-1);
      stopsTentativeTime[accessFootpath.first]    = departureTimeSeconds + accessFootpath.second + params.minWaitingTimeSeconds;
      if (accessFootpath.second < minAccessTravelTime)
      {
        minAccessTravelTime = accessFootpath.second;
      }
      //result.json += "origin_stop: " + stops[accessFootpath.first].name + " - " + Toolbox::convertSecondsToFormattedTime(stopsTentativeTime[accessFootpath.first]) + "\n";
      //result.json += std::to_string(stops[accessFootpath.first].id) + ",";
    }
    
    // fetch stops footpaths accessible to destination using params or osrm fetcher if not provided:
    if (params.egressStopIds.size() > 0 && params.egressStopTravelTimesSeconds.size() == params.egressStopIds.size())
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
      egressFootpaths = OsrmFetcher::getAccessibleStopsFootpathsFromPoint(params.destination, stops, params, params.accessMode, params.maxEgressWalkingTravelTimeSeconds);
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
          if ((reachedAtLeastOneEgressStop && maxEgressTravelTime >= 0 && connectionDepartureTime > tentativeEgressStopArrivalTime + maxEgressTravelTime) || (connectionDepartureTime - departureTimeSeconds > params.maxTotalTravelTimeSeconds))
          {
            break;
          }
          tripEnterConnectionIndex   = tripsEnterConnection[tripIndex];
          stopDepartureIndex         = std::get<connectionIndexes::STOP_DEP>(connection);
          stopDepartureTentativeTime = stopsTentativeTime[stopDepartureIndex];
          
          // reachable connections only here:
          if(tripEnterConnectionIndex != -1 || (stopDepartureTentativeTime <= connectionDepartureTime))
          {
            if (tripEnterConnectionIndex == -1 || (std::get<5>(journeys[stopDepartureIndex]) != 1 && std::get<4>(journeys[stopDepartureIndex]) < tripsEnterConnectionTransferTravelTime[tripIndex])) // make sure we transfer with the shortest footpath
            {
              tripsEnterConnection[tripIndex]                   = i;
              tripsEnterConnectionTransferTravelTime[tripIndex] = std::get<4>(journeys[stopDepartureIndex]);
            }
            
            // get footpaths for the arrival stop to get transferable stops:
            stopArrivalIndex      = std::get<connectionIndexes::STOP_ARR>(connection);
            connectionArrivalTime = std::get<connectionIndexes::TIME_ARR>(connection);
            footpathsRangeStart   = footpathsRanges[stopArrivalIndex].first;
            footpathsRangeEnd     = footpathsRanges[stopArrivalIndex].second;
            if (!reachedAtLeastOneEgressStop && stopsEgressTravelTime[stopArrivalIndex] != -1) // check if the arrival stop is egressable
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
                
                if (footpathTravelTime <= params.maxTransferWalkingTravelTimeSeconds && footpathTravelTime + params.minWaitingTimeSeconds + connectionArrivalTime < stopsTentativeTime[footpathStopArrivalIndex]
                )
                {
                  stopsTentativeTime[footpathStopArrivalIndex] = footpathTravelTime + params.minWaitingTimeSeconds + connectionArrivalTime;
                  journeys[footpathStopArrivalIndex]           = std::make_tuple(tripsEnterConnection[tripIndex], i, footpathIndex, tripIndex, footpathTravelTime, (stopArrivalIndex == footpathStopArrivalIndex ? 1 : -1));
                }
                footpathIndex++;
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
    
    i = 0;
    int hour = -1;
    int minute = -1;
    
    result.json += "departure: " + Toolbox::convertSecondsToFormattedTime(departureTimeSeconds) + "\n";
    
    for (auto & arrivalTime : stopsTentativeTime)
    {
      if (stopsEgressTravelTime[i] >= 0 && arrivalTime < MAX_INT && arrivalTime + stopsEgressTravelTime[i] < bestArrivalTime)
      {
        bestArrivalTime     = arrivalTime + stopsEgressTravelTime[i];
        bestEgressStopIndex = i;
        //result.json += "stop " + stops[i].name + " code " + stops[i].code + " index " + std::to_string(i) + " - " + " arrival: " + std::to_string(arrivalTime / 3600) + ":" + std::to_string((arrivalTime % 3600) / 60) + "\n";
      }
      i++;
    }
    
    //return result;
    
    std::cerr << "-- find best journey -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
    
    // recreate journey:
    subJourney = journeys[bestEgressStopIndex];
    i = 0;
    while ((std::get<0>(subJourney) != -1 && std::get<1>(subJourney) != -1) && i < 20)
    {
      journey.push_front(subJourney);
      bestAccessStopIndex = std::get<connectionIndexes::STOP_DEP>(forwardConnections[std::get<0>(subJourney)]);
      subJourney          = journeys[bestAccessStopIndex];
      i++;
    }
    journey.push_back(std::make_tuple(-1,-1,-1,-1,stopsEgressTravelTime[bestEgressStopIndex],-1));
    journey.push_front(std::make_tuple(-1,-1,-1,-1,stopsAccessTravelTime[bestAccessStopIndex],-1));
    
    Stop  journeyStepStopDeparture;
    Stop  journeyStepStopArrival;
    Trip  journeyStepTrip;
    Route journeyStepRoute;
    std::tuple<int,int,int,int,int,int,int> journeyStepEnterConnection; // connection tuple: departureStopIndex, arrivalStopIndex, departureTimeSeconds, arrivalTimeSeconds, tripIndex, canBoard, canUnboard
    std::tuple<int,int,int,int,int,int,int> journeyStepExitConnection;
    int   journeyStepTravelTime {-1};
    int   transferTime {-1};
    int   waitingTime {-1};
    int   transferArrivalTime {-1};
    int   transferReadyTime {-1};
    int   departureTime {-1};
    int   arrivalTime {-1};
    int   inVehicleTime {-1};
    
    
    //result.json += "arrivalTime: " + std::to_string(bestArrivalTime) + " seconds\n";
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
    
    std::cerr << "-- journey generation -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
    
    result.json += "{\n"
    "  \"origin\": [" + std::to_string(params.origin.latitude) + "," + std::to_string(params.origin.longitude) + "],\n"
    "  \"destination\": [" + std::to_string(params.destination.latitude) + "," + std::to_string(params.destination.longitude) + "],\n"
    "  \"steps\":\n  [\n";
    
    for (auto & journeyStep : journey)
    {
      //result.json += "step:\n";
      
      if (std::get<0>(journeyStep) != -1 && std::get<1>(journeyStep))
      {
        // journey tuple: final enter connection, final exit connection, final footpath
        journeyStepEnterConnection = forwardConnections[std::get<0>(journeyStep)];
        journeyStepExitConnection  = forwardConnections[std::get<1>(journeyStep)];
        journeyStepStopDeparture   = stops[std::get<connectionIndexes::STOP_DEP>(journeyStepEnterConnection)];
        journeyStepStopArrival     = stops[std::get<connectionIndexes::STOP_ARR>(journeyStepExitConnection)];
        journeyStepTrip            = trips[std::get<3>(journeyStep)];
        journeyStepRoute           = routes[routeIndexesById[journeyStepTrip.routeId]];
        transferTime               = std::get<4>(journeyStep) + params.minWaitingTimeSeconds;
        departureTime              = std::get<connectionIndexes::TIME_DEP>(journeyStepEnterConnection);
        arrivalTime                = std::get<connectionIndexes::TIME_ARR>(journeyStepExitConnection);
        inVehicleTime              = arrivalTime - departureTime;
        waitingTime                = departureTime - transferArrivalTime;
        transferArrivalTime        = arrivalTime + transferTime;
        transferReadyTime          = transferArrivalTime + params.minWaitingTimeSeconds;
        
        result.json += "\n    {\n"
        "      \"action\": \"board\",\n"
        "      \"agencyAcronym\": \"" + journeyStepRoute.agencyAcronym + "\",\n"
        "      \"agencyName\": \"" + journeyStepRoute.agencyName + "\",\n"
        "      \"agencyId\": \"" + std::to_string(journeyStepRoute.agencyId) + "\",\n"
        "      \"routeShortname\": \"" + journeyStepRoute.shortname + "\",\n"
        "      \"routeLongname\": \"" + journeyStepRoute.longname + "\",\n"
        "      \"routeId\": \"" + std::to_string(journeyStepRoute.id) + "\",\n"
        "      \"routeTypeName\": \"" + journeyStepRoute.routeTypeName + "\",\n"
        "      \"routeTypeId\": \"" + std::to_string(journeyStepRoute.routeTypeId) + "\",\n"
        "      \"stopName\": \"" + journeyStepStopDeparture.name + "\",\n"
        "      \"stopCode\": \"" + journeyStepStopDeparture.code + "\",\n"
        "      \"stopId\": \"" + std::to_string(journeyStepStopDeparture.id) + "\",\n"
        "      \"stopCoordinates\": [" + std::to_string(journeyStepStopDeparture.point.latitude) + "," + std::to_string(journeyStepStopDeparture.point.longitude) + "],\n"
        "      \"departureTime\": \"" + Toolbox::convertSecondsToFormattedTime(departureTime) + "\",\n"
        "      \"waitingTimeSeconds\":"  + std::to_string(waitingTime) + ",\n"
        "      \"waitingTimeMinutes\":"  + std::to_string(Toolbox::convertSecondsToMinutes(waitingTime)) + "\n"
        "    },\n"
        
        "    {\n"
        "      \"action\": \"unboard\",\n"
        "      \"agencyAcronym\": \"" + journeyStepRoute.agencyAcronym + "\",\n"
        "      \"agencyName\": \"" + journeyStepRoute.agencyName + "\",\n"
        "      \"agencyId\": \"" + std::to_string(journeyStepRoute.agencyId) + "\",\n"
        "      \"routeShortname\": \"" + journeyStepRoute.shortname + "\",\n"
        "      \"routeLongname\": \"" + journeyStepRoute.longname + "\",\n"
        "      \"routeId\": \"" + std::to_string(journeyStepRoute.id) + "\",\n"
        "      \"routeTypeName\": \"" + journeyStepRoute.routeTypeName + "\",\n"
        "      \"routeTypeId\": \"" + std::to_string(journeyStepRoute.routeTypeId) + "\",\n"
        "      \"stopName\": \"" + journeyStepStopArrival.name + "\",\n"
        "      \"stopCode\": \"" + journeyStepStopArrival.code + "\",\n"
        "      \"stopId\": \"" + std::to_string(journeyStepStopArrival.id) + "\",\n"
        "      \"stopCoordinates\": [" + std::to_string(journeyStepStopArrival.point.latitude) + "," + std::to_string(journeyStepStopArrival.point.longitude) + "],\n"
        "      \"arrivalTime\": \"" + Toolbox::convertSecondsToFormattedTime(arrivalTime) + "\",\n"
        "      \"segmentInVehicleTimeSeconds\":"  + std::to_string(Toolbox::convertSecondsToMinutes(inVehicleTime)) + ",\n"
        "      \"segmentInVehicleTimeMinutes\":"  + std::to_string(inVehicleTime) + "\n"
        "    },\n"
        
        "    {\n"
        "      \"action\": \"walk\",\n"
        "      \"departureTime\": \"" + Toolbox::convertSecondsToFormattedTime(arrivalTime) + "\",\n"
        "      \"arrivingAtStopAt\":"  + Toolbox::convertSecondsToFormattedTime(transferArrivalTime) + ",\n"
        "      \"readyToBoardAt\":"  + Toolbox::convertSecondsToFormattedTime(transferReadyTime) + "\n"
        "    },";
        
        //result.json += "\n\n  enter stop: " + journeyStepStopDeparture.name + " [" + journeyStepStopDeparture.code + "] @ " + std::to_string(std::get<connectionIndexes::TIME_DEP>(journeyStepEnterConnection)) + " (" + Toolbox::convertSecondsToFormattedTime(std::get<connectionIndexes::TIME_DEP>(journeyStepEnterConnection)) + ") on route " + journeyStepRoute.shortname + " " + journeyStepRoute.longname + "\n";
        //result.json += "  exit stop: "  + journeyStepStopArrival.name   + " [" + journeyStepStopArrival.code   + "] @ " + std::to_string(std::get<connectionIndexes::TIME_ARR>(journeyStepExitConnection))  + " (" + Toolbox::convertSecondsToFormattedTime(std::get<connectionIndexes::TIME_ARR>(journeyStepExitConnection))  + ") on route " + journeyStepRoute.shortname + " " + journeyStepRoute.longname + "\n";
        //result.json += "  trip id: "  + std::to_string(journeyStepTrip.id) + "\n";
        //result.json += "  transfer: " + std::to_string(transferTime) + "\n\n\n";
      }
      else // access or egress journey step
      {
        if (transferArrivalTime == -1) // access
        {
          transferTime        = std::get<4>(journeyStep);
          transferArrivalTime = departureTimeSeconds + transferTime;
          transferReadyTime   = transferArrivalTime + params.minWaitingTimeSeconds;
          
          result.json += "\n    {\n"
          "      \"action\": \"walk\",\n"
          "      \"departureTime\": \"" + Toolbox::convertSecondsToFormattedTime(departureTimeSeconds) + "\",\n"
          "      \"arrivingAtStopAt\":"  + Toolbox::convertSecondsToFormattedTime(transferArrivalTime) + ",\n"
          "      \"readyToBoardAt\":"  + Toolbox::convertSecondsToFormattedTime(transferReadyTime) + "\n"
          "    },\n";
        }
        else // egress
        {
          
        }
        //result.json += "  walk: " + std::to_string(std::get<4>(journeyStep)) + " seconds\n";
      }
    }
    
    result.json.pop_back(); // remove trailing comma
    result.json += "\n  ],\n";
    
    bestArrivalTime -= transferTime; // we need to remove the last transfer time becuase it includes the minimum waiting time
    
    //result.json += "arrival: " + Toolbox::convertSecondsToFormattedTime(bestArrivalTime) + "\n";
    
    std::cerr << "-- journey conversion -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
    
    return result;
    
  }
  
}
