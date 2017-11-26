#include "calculator.hpp"

namespace TrRouting
{
    
  std::tuple<int,int,int> Calculator::reverseCalculation()
  {
    int  i {0};
    int  connectionsCount = reverseConnections.size();
    int  reachableConnectionsCount {0};
    int  tripIndex {-1};
    int  stopDepartureIndex {-1};
    int  stopArrivalIndex {-1};
    int  tripExitConnectionIndex {-1};
    int  stopDepartureTentativeTime {MAX_INT};
    int  stopArrivalTentativeTime {MAX_INT};
    int  connectionDepartureTime {-1};
    int  connectionArrivalTime {-1};
    int  footpathsRangeStart {-1};
    int  footpathsRangeEnd {-1};
    int  footpathIndex {-1};
    int  footpathStopDepartureIndex {-1};
    int  footpathTravelTime {-1};
    int  tentativeAccessStopDepartureTime {-1};
    bool reachedAtLeastOneAccessStop {false};
    int  bestAccessStopIndex {-1};
    int  bestAccessTravelTime {-1};
    int  bestDepartureTime {-1};

    // reverse calculation:
    //int time1 {-1};
    //int time2 {-1};
    //int time3 {-1};
    //int timeC {-1};

    // main loop for reverse connections:
    i = 0;
    for(auto & connection : reverseConnections)
    {
      // ignore connections before departure time + minimum access travel time:
      if (std::get<connectionIndexes::TIME_ARR>(connection) <= arrivalTimeSeconds + minEgressTravelTime)
      {
        tripIndex = std::get<connectionIndexes::TRIP>(connection);
        
        // enabled trips only here:
        if (/*tripsUsable[tripIndex] != -1 && */tripsEnabled[tripIndex] != -1)
        {
          connectionArrivalTime = std::get<connectionIndexes::TIME_ARR>(connection);
          
          // no need to parse next connections if already reached destination from all egress stops, except if max travel time is set, so we can get a reverse profile in the next loop calculation:
          if ( (!params.returnAllStopsResult && reachedAtLeastOneAccessStop && maxAccessTravelTime >= 0 && connectionArrivalTime < tentativeAccessStopDepartureTime - maxAccessTravelTime) || (arrivalTimeSeconds - connectionArrivalTime > params.maxTotalTravelTimeSeconds))
          {
            break;
          }
          tripExitConnectionIndex  = tripsExitConnection[tripIndex];
          stopArrivalIndex         = std::get<connectionIndexes::STOP_ARR>(connection);
          stopArrivalTentativeTime = stopsReverseTentativeTime[stopArrivalIndex];
          
          // reachable connections only here:
          if (tripExitConnectionIndex != -1 || stopArrivalTentativeTime >= connectionArrivalTime)
          {
                        
            if (std::get<connectionIndexes::CAN_UNBOARD>(connection) == 1 && (tripExitConnectionIndex == -1 || (std::get<0>(reverseJourneys[stopArrivalIndex]) == -1 && std::get<4>(reverseJourneys[stopArrivalIndex]) >= 0 && std::get<4>(reverseJourneys[stopArrivalIndex]) < tripsExitConnectionTransferTravelTime[tripIndex])))
            {
              tripsExitConnection[tripIndex]                   = i;
              tripsExitConnectionTransferTravelTime[tripIndex] = std::get<4>(reverseJourneys[stopArrivalIndex]);
            }
            
            if (std::get<connectionIndexes::CAN_BOARD>(connection) == 1 && tripsExitConnection[tripIndex] != -1)
            {
              // get footpaths for the arrival stop to get transferable stops:
              stopDepartureIndex      = std::get<connectionIndexes::STOP_DEP>(connection);
              connectionDepartureTime = std::get<connectionIndexes::TIME_DEP>(connection);
              footpathsRangeStart     = footpathsRanges[stopDepartureIndex].first;
              footpathsRangeEnd       = footpathsRanges[stopDepartureIndex].second;
              if (!params.returnAllStopsResult && !reachedAtLeastOneAccessStop && stopsAccessTravelTime[stopDepartureIndex] != -1) // check if the departure stop is accessable
              {
                reachedAtLeastOneAccessStop      = true;
                tentativeAccessStopDepartureTime = connectionDepartureTime;
              }
              if (footpathsRangeStart >= 0 && footpathsRangeEnd >= 0)
              {
                footpathIndex = footpathsRangeStart;
                while (footpathIndex <= footpathsRangeEnd)
                {
                  footpathStopDepartureIndex = std::get<1>(footpaths[footpathIndex]);
                  footpathTravelTime         = std::get<2>(footpaths[footpathIndex]);
                  
                  if (footpathTravelTime <= params.maxTransferWalkingTravelTimeSeconds && connectionDepartureTime - footpathTravelTime - params.minWaitingTimeSeconds > stopsReverseTentativeTime[footpathStopDepartureIndex])
                  {
                    stopsReverseTentativeTime[footpathStopDepartureIndex] = connectionDepartureTime - footpathTravelTime - params.minWaitingTimeSeconds;
                    reverseJourneys[footpathStopDepartureIndex]           = std::make_tuple(i, tripsExitConnection[tripIndex], footpathIndex, tripIndex, footpathTravelTime, (stopDepartureIndex == footpathStopDepartureIndex ? 1 : -1));
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

    // find best egress stop:
    if (!params.returnAllStopsResult)
    {
      i = 0;
      for (auto & departureTime : stopsReverseTentativeTime)
      {
        if (stopsAccessTravelTime[i] >= 0 && departureTime >= 0 && departureTime - stopsAccessTravelTime[i] > bestDepartureTime)
        {
          bestDepartureTime    = departureTime - stopsAccessTravelTime[i];
          bestAccessStopIndex  = i;
          bestAccessTravelTime = stopsAccessTravelTime[i];
        }
        i++;
      }
      return std::make_tuple(bestDepartureTime, bestAccessStopIndex, bestAccessTravelTime);
    }
    else
    {
      return std::make_tuple(-1, -1, -1);
    }


  }

}