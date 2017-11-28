#include "calculator.hpp"

namespace TrRouting
{
    
  std::tuple<int,int,int> Calculator::forwardCalculation()
  {
    int  i {0};
    int  connectionsCount = forwardConnections.size();
    int  reachableConnectionsCount {0};
    int  tripIndex {-1};
    int  stopDepartureIndex {-1};
    int  stopArrivalIndex {-1};
    int  tripEnterConnectionIndex {-1};
    int  stopDepartureTentativeTime {MAX_INT};
    int  stopArrivalTentativeTime {MAX_INT};
    int  connectionDepartureTime {-1};
    int  connectionArrivalTime {-1};
    int  footpathsRangeStart {-1};
    int  footpathsRangeEnd {-1};
    int  footpathIndex {-1};
    int  footpathStopArrivalIndex {-1};
    int  footpathTravelTime {-1};
    int  tentativeEgressStopArrivalTime {MAX_INT};
    bool reachedAtLeastOneEgressStop {false};
    int  bestEgressStopIndex {-1};
    int  bestEgressTravelTime {-1};
    int  bestArrivalTime {MAX_INT};
    
    
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
          if ( (!params.returnAllStopsResult && reachedAtLeastOneEgressStop && maxEgressTravelTime >= 0 && tentativeEgressStopArrivalTime < MAX_INT && connectionDepartureTime > tentativeEgressStopArrivalTime + maxEgressTravelTime) || (connectionDepartureTime - departureTimeSeconds > params.maxTotalTravelTimeSeconds))
          {
            break;
          }
          tripEnterConnectionIndex   = tripsEnterConnection[tripIndex];
          stopDepartureIndex         = std::get<connectionIndexes::STOP_DEP>(connection);
          stopDepartureTentativeTime = stopsTentativeTime[stopDepartureIndex];
          
          // reachable connections only here:
          if (tripEnterConnectionIndex != -1 || stopDepartureTentativeTime <= connectionDepartureTime)
          {
            
            //std::get<5>(forwardJourneys[stopDepartureIndex]) != 1 
            
            if (std::get<connectionIndexes::CAN_BOARD>(connection) == 1 && (tripEnterConnectionIndex == -1 || (std::get<0>(forwardJourneys[stopDepartureIndex]) == -1 && std::get<4>(forwardJourneys[stopDepartureIndex]) >= 0 && std::get<4>(forwardJourneys[stopDepartureIndex]) < tripsEnterConnectionTransferTravelTime[tripIndex])))
            //( tripEnterConnectionIndex == -1) || (std::get<5>(forwardJourneys[stopDepartureIndex]) != 1 && (std::get<0>(forwardJourneys[stopDepartureIndex]) == -1 || std::get<connectionIndexes::TRIP>(forwardConnections[std::get<0>(forwardJourneys[stopDepartureIndex])]) == tripIndex) && std::get<4>(forwardJourneys[stopDepartureIndex]) >= 0 && std::get<4>(forwardJourneys[stopDepartureIndex]) < tripsEnterConnectionTransferTravelTime[tripIndex]))
            //)
            {
              //if (tripEnterConnectionIndex != -1)
              //{
              //  std::cerr << "from_stop: " << stops[std::get<0>(footpaths[std::get<2>(forwardJourneys[stopDepartureIndex])])].name << " route:" << routes[routeIndexesById[trips[tripIndex].routeId]].shortname << " " << routes[routeIndexesById[trips[tripIndex].routeId]].longname << " old stop:" << stops[std::get<connectionIndexes::STOP_DEP>(forwardConnections[std::get<0>(forwardJourneys[stopDepartureIndex])])].name << " stop:" << stops[stopDepartureIndex].name << " tec:" <<  tripEnterConnectionIndex << " i:" <<  i << " jss:" <<  std::get<5>(forwardJourneys[stopDepartureIndex]) << " jenterc:" << std::get<0>(forwardJourneys[stopDepartureIndex]) << " jexitc:" << std::get<1>(forwardJourneys[stopDepartureIndex]) << " jtt:" << std::get<4>(forwardJourneys[stopDepartureIndex]) << " tectt:" << tripsEnterConnectionTransferTravelTime[tripIndex] << std::endl;
              //}
              tripsUsable[tripIndex]                            = 1;
              tripsEnterConnection[tripIndex]                   = i;
              tripsEnterConnectionTransferTravelTime[tripIndex] = std::get<4>(forwardJourneys[stopDepartureIndex]);
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
                  if (footpathTravelTime <= params.maxTransferWalkingTravelTimeSeconds)
                  {
                    if (footpathTravelTime + params.minWaitingTimeSeconds + connectionArrivalTime < stopsTentativeTime[footpathStopArrivalIndex])
                    {
                      stopsTentativeTime[footpathStopArrivalIndex] = footpathTravelTime + connectionArrivalTime + params.minWaitingTimeSeconds;
                      forwardJourneys[footpathStopArrivalIndex]    = std::make_tuple(tripsEnterConnection[tripIndex], i, footpathIndex, tripIndex, footpathTravelTime, (stopArrivalIndex == footpathStopArrivalIndex ? 1 : -1));
                    }
                    if (stopArrivalIndex == footpathStopArrivalIndex && (std::get<4>(forwardEgressJourneys[footpathStopArrivalIndex]) == -1 || std::get<connectionIndexes::TIME_ARR>(forwardConnections[std::get<1>(forwardEgressJourneys[footpathStopArrivalIndex])]) > connectionArrivalTime))
                    {
                      forwardEgressJourneys[footpathStopArrivalIndex] = std::make_tuple(tripsEnterConnection[tripIndex], i, footpathIndex, tripIndex, footpathTravelTime, 1);
                    }
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
    
    std::cerr << "-- " << reachableConnectionsCount << " forward connections parsed on " << connectionsCount << std::endl;
    
    int egressStopArrivalTime {-1};
    int egressExitConnection  {-1};
    int egressTravelTime      {-1};
    // find best egress stop:
    if (!params.returnAllStopsResult)
    {
      i = 0;
      for (auto & egressFootpath : egressFootpaths)
      {
        //std::cerr << stops[egressFootpath.first].name << std::endl;
        egressExitConnection  = std::get<1>(forwardEgressJourneys[egressFootpath.first]);
        if (egressExitConnection != -1)
        {
          egressTravelTime      = stopsEgressTravelTime[egressFootpath.first];
          egressStopArrivalTime = std::get<connectionIndexes::TIME_ARR>(forwardConnections[egressExitConnection]) + egressTravelTime;
          std::cerr << stops[egressFootpath.first].name << ": " << egressTravelTime << " - " << Toolbox::convertSecondsToFormattedTime(egressStopArrivalTime) << std::endl;
          if (egressStopArrivalTime >= 0 && egressStopArrivalTime < MAX_INT && egressStopArrivalTime < bestArrivalTime)
          {
            bestArrivalTime      = egressStopArrivalTime;
            bestEgressStopIndex  = egressFootpath.first;
            bestEgressTravelTime = egressTravelTime;
          }
        }
        i++;
      }
      return std::make_tuple(bestArrivalTime, bestEgressStopIndex, bestEgressTravelTime);
    }
    else
    {
      return std::make_tuple(MAX_INT, -1, -1);
    }

  }

}