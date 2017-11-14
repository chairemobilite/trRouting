#include "calculator.hpp"

namespace TrRouting
{
  
  RoutingResult Calculator::calculate() {
    
    long long calculationTime {algorithmCalculationTime.getDurationMicrosecondsNoStop()};
    
    reset();
    
    RoutingResult    result;
    std::vector<int> journey;
    
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
    
    std::cerr << "-- reset and preparations -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
    
    // fetch stops footpaths accessible from origin:
    accessFootpaths = OsrmFetcher::getAccessibleStopsFootpathsFromPoint(params.origin, stops, params, params.accessMode, params.maxAccessWalkingTravelTimeSeconds);
    for (auto & accessFootpath : accessFootpaths)
    {
      stopsTentativeTime[accessFootpath.first] = departureTimeSeconds + accessFootpath.second;
      result.json += "origin_stop: " + stops[accessFootpath.first].name + " - " + std::to_string(stopsTentativeTime[accessFootpath.first] / 3600) + ":" + std::to_string((stopsTentativeTime[accessFootpath.first] % 3600) / 60) + "\n";
    }
    
    egressFootpaths = OsrmFetcher::getAccessibleStopsFootpathsFromPoint(params.destination, stops, params, params.egressMode, params.maxEgressWalkingTravelTimeSeconds);
    for (auto & egressFootpath : egressFootpaths)
    {
      stopsEgressTravelTime[egressFootpath.first] = egressFootpath.second;
    }
    
    for(auto & connection : forwardConnections)
    {
      
      tripIndex                  = std::get<connectionIndexes::TRIP>(connection);
      stopDepartureIndex         = std::get<connectionIndexes::STOP_DEP>(connection);
      connectionDepartureTime    = std::get<connectionIndexes::TIME_DEP>(connection);
      tripEnterConnectionIndex   = tripsEnterConnection[tripIndex];
      stopDepartureTentativeTime = stopsTentativeTime[stopDepartureIndex];
      
      if(tripsEnabled[tripIndex] && (tripEnterConnectionIndex != -1 || stopDepartureTentativeTime <= connectionDepartureTime))
      {
        
        connectionArrivalTime = std::get<connectionIndexes::TIME_ARR>(connection);
        
        if (tripEnterConnectionIndex == -1)
        {
          tripsEnterConnection[tripIndex] = i;
        }
        
        // get footpaths for the arrival stop to get transferable stops:
        stopArrivalIndex    = std::get<connectionIndexes::STOP_ARR>(connection);
        footpathsRangeStart = footpathsRanges[stopArrivalIndex].first;
        footpathsRangeEnd   = footpathsRanges[stopArrivalIndex].second;
        if (footpathsRangeStart >= 0 && footpathsRangeEnd >= 0)
        {
          stopArrivalTentativeTime = stopsTentativeTime[stopArrivalIndex];
          footpathIndex = footpathsRangeStart;
          while (footpathIndex <= footpathsRangeEnd)
          {
            footpathStopArrivalIndex = std::get<1>(footpaths[footpathIndex]);
            footpathTravelTime       = std::get<2>(footpaths[footpathIndex]);
            if (footpathTravelTime + connectionArrivalTime < stopArrivalTentativeTime)
            {
              stopsTentativeTime[stopArrivalIndex] = footpathTravelTime + connectionArrivalTime;
              journeys[stopArrivalIndex]           = std::make_tuple(tripIndex, i, footpathIndex);
            }
            footpathIndex++;
          }
        }
        reachableConnectionsCount++;
      }
      
      i++;
      
    }
    
    std::cerr << "-- " << reachableConnectionsCount << " connections parsed on " << connectionsCount << std::endl;
    
    std::cerr << "-- main calculation -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
    
    i = 0;
    int hour = -1;
    int minute = -1;
    
    result.json += "departure: " + std::to_string(departureTimeSeconds / 3600) + ":" + std::to_string((departureTimeSeconds % 3600) / 60) + "\n";
    
    for (auto & arrivalTime : stopsTentativeTime)
    {
      if (arrivalTime < MAX_INT)
      {
        result.json += "stop " + stops[i].name + " index " + std::to_string(i) + " - " + " arrival: " + std::to_string(arrivalTime / 3600) + ":" + std::to_string((arrivalTime % 3600) / 60) + "\n";
      }
      i++;
    }
    
    std::cerr << "-- result preparation -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
    
    return result;
    
  }
  
}
