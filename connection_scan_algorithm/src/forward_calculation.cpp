#include "calculator.hpp"

namespace TrRouting
{
    
  std::tuple<int,int,int> Calculator::forwardCalculation()
  {
    int  i {0};
    int  connectionsCount = forwardConnections.size();
    int  reachableConnectionsCount {0};
    int  tripIndex {-1};
    int  nodeDepartureIndex {-1};
    int  nodeArrivalIndex {-1};
    int  tripEnterConnectionIndex {-1};
    int  nodeDepartureTentativeTime {MAX_INT};
    int  nodeArrivalTentativeTime {MAX_INT};
    int  connectionDepartureTime {-1};
    int  connectionArrivalTime {-1};
    /*long long  footpathsRangeStart {-1};
    long long  footpathsRangeEnd {-1};*/
    int  footpathIndex {-1};
    int  transferableNodeIndex {0};
    int  footpathNodeArrivalIndex {-1};
    int  footpathTravelTime {-1};
    int  tentativeEgressNodeArrivalTime {MAX_INT};
    bool reachedAtLeastOneEgressNode {false};
    int  bestEgressNodeIndex {-1};
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
          
          // no need to parse next connections if already reached destination from all egress nodes:
          if ( (!params.returnAllNodesResult && reachedAtLeastOneEgressNode && maxEgressTravelTime >= 0 && tentativeEgressNodeArrivalTime < MAX_INT && connectionDepartureTime > tentativeEgressNodeArrivalTime + maxEgressTravelTime) || (connectionDepartureTime - departureTimeSeconds > params.maxTotalTravelTimeSeconds))
          {
            break;
          }
          tripEnterConnectionIndex   = tripsEnterConnection[tripIndex];
          nodeDepartureIndex         = std::get<connectionIndexes::NODE_DEP>(connection);
          nodeDepartureTentativeTime = nodesTentativeTime[nodeDepartureIndex];
          
          // reachable connections only here:
          if (tripEnterConnectionIndex != -1 || nodeDepartureTentativeTime <= connectionDepartureTime)
          {
            
            //std::get<5>(forwardJourneys[nodeDepartureIndex]) != 1 
            
            if (std::get<connectionIndexes::CAN_BOARD>(connection) == 1 && (tripEnterConnectionIndex == -1 || (std::get<0>(forwardJourneys[nodeDepartureIndex]) == -1 && std::get<4>(forwardJourneys[nodeDepartureIndex]) >= 0 && std::get<4>(forwardJourneys[nodeDepartureIndex]) < tripsEnterConnectionTransferTravelTime[tripIndex])))
            //( tripEnterConnectionIndex == -1) || (std::get<5>(forwardJourneys[nodeDepartureIndex]) != 1 && (std::get<0>(forwardJourneys[nodeDepartureIndex]) == -1 || std::get<connectionIndexes::TRIP>(forwardConnections[std::get<0>(forwardJourneys[nodeDepartureIndex])]) == tripIndex) && std::get<4>(forwardJourneys[nodeDepartureIndex]) >= 0 && std::get<4>(forwardJourneys[nodeDepartureIndex]) < tripsEnterConnectionTransferTravelTime[tripIndex]))
            //)
            {
              //if (tripEnterConnectionIndex != -1)
              //{
              //  std::cerr << "from_node: " << nodes[std::get<0>(footpaths[std::get<2>(forwardJourneys[nodeDepartureIndex])])].name << " route:" << routes[routeIndexesById[trips[tripIndex].routeId]].shortname << " " << routes[routeIndexesById[trips[tripIndex].routeId]].longname << " old node:" << nodes[std::get<connectionIndexes::NODE_DEP>(forwardConnections[std::get<0>(forwardJourneys[nodeDepartureIndex])])].name << " node:" << nodes[nodeDepartureIndex].name << " tec:" <<  tripEnterConnectionIndex << " i:" <<  i << " jss:" <<  std::get<5>(forwardJourneys[nodeDepartureIndex]) << " jenterc:" << std::get<0>(forwardJourneys[nodeDepartureIndex]) << " jexitc:" << std::get<1>(forwardJourneys[nodeDepartureIndex]) << " jtt:" << std::get<4>(forwardJourneys[nodeDepartureIndex]) << " tectt:" << tripsEnterConnectionTransferTravelTime[tripIndex] << std::endl;
              //}
              tripsUsable[tripIndex]                            = 1;
              tripsEnterConnection[tripIndex]                   = i;
              tripsEnterConnectionTransferTravelTime[tripIndex] = std::get<4>(forwardJourneys[nodeDepartureIndex]);
            }
            
            if (std::get<connectionIndexes::CAN_UNBOARD>(connection) == 1 && tripsEnterConnection[tripIndex] != -1)
            {
              // get footpaths for the arrival node to get transferable nodes:
              nodeArrivalIndex      = std::get<connectionIndexes::NODE_ARR>(connection);
              connectionArrivalTime = std::get<connectionIndexes::TIME_ARR>(connection);
              //footpathsRangeStart   = footpathsRanges[nodeArrivalIndex].first;
              //footpathsRangeEnd     = footpathsRanges[nodeArrivalIndex].second;
              if (!params.returnAllNodesResult && !reachedAtLeastOneEgressNode && nodesEgressTravelTime[nodeArrivalIndex] != -1) // check if the arrival node is egressable
              {
                reachedAtLeastOneEgressNode    = true;
                tentativeEgressNodeArrivalTime = connectionArrivalTime;
              }
              footpathIndex = 0;
              for (int & transferableNodeIndex : nodes[nodeArrivalIndex].transferableNodesIdx)
              {
                footpathTravelTime       = nodes[nodeArrivalIndex].transferableTravelTimesSeconds[footpathIndex];
                footpathNodeArrivalIndex = transferableNodeIndex;

                if (footpathTravelTime <= params.maxTransferWalkingTravelTimeSeconds)
                {
                  if (footpathTravelTime + params.minWaitingTimeSeconds + connectionArrivalTime < nodesTentativeTime[footpathNodeArrivalIndex])
                  {
                    nodesTentativeTime[footpathNodeArrivalIndex] = footpathTravelTime + connectionArrivalTime + params.minWaitingTimeSeconds;
                    forwardJourneys[footpathNodeArrivalIndex]    = std::make_tuple(tripsEnterConnection[tripIndex], i, nodeArrivalIndex, tripIndex, footpathTravelTime, (nodeArrivalIndex == footpathNodeArrivalIndex ? 1 : -1));
                  }
                  if (nodeArrivalIndex == footpathNodeArrivalIndex && (std::get<4>(forwardEgressJourneys[footpathNodeArrivalIndex]) == -1 || std::get<connectionIndexes::TIME_ARR>(forwardConnections[std::get<1>(forwardEgressJourneys[footpathNodeArrivalIndex])]) > connectionArrivalTime))
                  {
                    forwardEgressJourneys[footpathNodeArrivalIndex] = std::make_tuple(tripsEnterConnection[tripIndex], i, nodeArrivalIndex, tripIndex, footpathTravelTime, 1);
                  }
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
    
    if (params.debugDisplay)
      std::cerr << "-- " << reachableConnectionsCount << " forward connections parsed on " << connectionsCount << std::endl;
    
    int egressNodeArrivalTime {-1};
    int egressExitConnection  {-1};
    int egressTravelTime      {-1};
    // find best egress node:
    if (!params.returnAllNodesResult)
    {
      i = 0;
      for (auto & egressFootpath : egressFootpaths)
      {
        //std::cerr << nodes[egressFootpath.first].name << std::endl;
        egressExitConnection  = std::get<1>(forwardEgressJourneys[egressFootpath.first]);
        if (egressExitConnection != -1)
        {
          egressTravelTime      = nodesEgressTravelTime[egressFootpath.first];
          egressNodeArrivalTime = std::get<connectionIndexes::TIME_ARR>(forwardConnections[egressExitConnection]) + egressTravelTime;
          //std::cerr << nodes[egressFootpath.first].name << ": " << egressTravelTime << " - " << Toolbox::convertSecondsToFormattedTime(egressNodeArrivalTime) << std::endl;
          if (egressNodeArrivalTime >= 0 && egressNodeArrivalTime - departureTimeSeconds <= params.maxTotalTravelTimeSeconds && egressNodeArrivalTime < bestArrivalTime && egressNodeArrivalTime < MAX_INT)
          {
            bestArrivalTime      = egressNodeArrivalTime;
            bestEgressNodeIndex  = egressFootpath.first;
            bestEgressTravelTime = egressTravelTime;
          }
        }
        i++;
      }
      return std::make_tuple(bestArrivalTime, bestEgressNodeIndex, bestEgressTravelTime);
    }
    else
    {
      return std::make_tuple(MAX_INT, -1, -1);
    }

  }

}
