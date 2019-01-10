#include "calculator.hpp"

namespace TrRouting
{
    
  std::tuple<int,int,int> Calculator::reverseCalculation()
  {
    int  i {0};
    int  connectionsCount = reverseConnections.size();
    int  reachableConnectionsCount {0};
    int  tripIndex {-1};
    int  nodeDepartureIndex {-1};
    int  nodeArrivalIndex {-1};
    int  tripExitConnectionIndex {-1};
    int  nodeDepartureTentativeTime {MAX_INT};
    int  nodeArrivalTentativeTime {MAX_INT};
    int  connectionDepartureTime {-1};
    int  connectionArrivalTime {-1};
    long long  footpathsRangeStart {-1};
    long long  footpathsRangeEnd {-1};
    long long  footpathIndex {-1};
    int  footpathNodeDepartureIndex {-1};
    int  footpathTravelTime {-1};
    int  tentativeAccessNodeDepartureTime {-1};
    bool reachedAtLeastOneAccessNode {false};
    int  bestAccessNodeIndex {-1};
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
      if (std::get<connectionIndexes::TIME_ARR>(connection) <= arrivalTimeSeconds - minEgressTravelTime)
      {
        
        tripIndex = std::get<connectionIndexes::TRIP>(connection);
        
        // enabled trips only here:
        if (tripsUsable[tripIndex] == 1 && tripsEnabled[tripIndex] != -1)
        {
          
          connectionArrivalTime = std::get<connectionIndexes::TIME_ARR>(connection);
          
          // no need to parse next connections if already reached destination from all egress nodes, except if max travel time is set, so we can get a reverse profile in the next loop calculation:
          if ( (!params.returnAllNodesResult && reachedAtLeastOneAccessNode && maxAccessTravelTime >= 0 && connectionArrivalTime < tentativeAccessNodeDepartureTime - maxAccessTravelTime) || (arrivalTimeSeconds - connectionArrivalTime > params.maxTotalTravelTimeSeconds))
          {
            break;
          }
          tripExitConnectionIndex  = tripsExitConnection[tripIndex];
          nodeArrivalIndex         = std::get<connectionIndexes::NODE_ARR>(connection);
          nodeArrivalTentativeTime = nodesReverseTentativeTime[nodeArrivalIndex];
          
          //std::cerr << "nodeArrivalTentativeTime: " << nodeArrivalTentativeTime << " connectionArrivalTime: " << connectionArrivalTime << " tripExitConnectionIndex: " << tripExitConnectionIndex << std::endl;
          
          // reachable connections only here:
          if (tripExitConnectionIndex != -1 || nodeArrivalTentativeTime >= connectionArrivalTime)
          {
            
            if (std::get<connectionIndexes::CAN_UNBOARD>(connection) == 1 && (tripExitConnectionIndex == -1 || (std::get<0>(reverseJourneys[nodeArrivalIndex]) == -1 && std::get<4>(reverseJourneys[nodeArrivalIndex]) >= 0 && std::get<4>(reverseJourneys[nodeArrivalIndex]) <= tripsExitConnectionTransferTravelTime[tripIndex]))) // <= to make sure we get the same result as forward calculation, which uses >
            {
              tripsExitConnection[tripIndex]                   = i;
              tripsExitConnectionTransferTravelTime[tripIndex] = std::get<4>(reverseJourneys[nodeArrivalIndex]);
            }
            
            if (std::get<connectionIndexes::CAN_BOARD>(connection) == 1 && tripsExitConnection[tripIndex] != -1)
            {
              // get footpaths for the arrival node to get transferable nodes:
              nodeDepartureIndex      = std::get<connectionIndexes::NODE_DEP>(connection);
              connectionDepartureTime = std::get<connectionIndexes::TIME_DEP>(connection);
              //footpathsRangeStart     = footpathsRanges[nodeDepartureIndex].first;
              //footpathsRangeEnd       = footpathsRanges[nodeDepartureIndex].second;
              if (!params.returnAllNodesResult && !reachedAtLeastOneAccessNode && nodesAccessTravelTime[nodeDepartureIndex] != -1) // check if the departure node is accessable
              {
                reachedAtLeastOneAccessNode      = true;
                tentativeAccessNodeDepartureTime = connectionDepartureTime;
              }
              footpathIndex = 0;
              for (int & transferableNodeIndex : nodes[nodeDepartureIndex].transferableNodesIdx)
              {
                footpathTravelTime         = (int)ceil((float)nodes[nodeDepartureIndex].transferableTravelTimesSeconds[footpathIndex] / params.walkingSpeedFactor);
                footpathNodeDepartureIndex = transferableNodeIndex;

                if (footpathTravelTime <= params.maxTransferWalkingTravelTimeSeconds)
                {
                  if (connectionDepartureTime - footpathTravelTime - params.minWaitingTimeSeconds >= nodesReverseTentativeTime[footpathNodeDepartureIndex])
                  {
                    nodesReverseTentativeTime[footpathNodeDepartureIndex] = connectionDepartureTime - footpathTravelTime - params.minWaitingTimeSeconds;
                    reverseJourneys[footpathNodeDepartureIndex]           = std::make_tuple(i, tripsExitConnection[tripIndex], nodeDepartureIndex, tripIndex, footpathTravelTime, (nodeDepartureIndex == footpathNodeDepartureIndex ? 1 : -1));
                  }
                  if (nodeDepartureIndex == footpathNodeDepartureIndex && (std::get<4>(reverseAccessJourneys[footpathNodeDepartureIndex]) == -1 || std::get<connectionIndexes::TIME_DEP>(reverseConnections[std::get<1>(reverseAccessJourneys[footpathNodeDepartureIndex])]) <= connectionDepartureTime))
                  {
                    reverseAccessJourneys[footpathNodeDepartureIndex] = std::make_tuple(i, tripsExitConnection[tripIndex], nodeDepartureIndex, tripIndex, footpathTravelTime, 1);
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
      std::cerr << "-- " << reachableConnectionsCount << " reverse connections parsed on " << connectionsCount << std::endl;

    
    int accessNodeDepartureTime {-1};
    int accessEnterConnection   {-1};
    int accessTravelTime        {-1};
    // find best access node:
    if (!params.returnAllNodesResult)
    {
      i = 0;
      for (auto & accessFootpath : accessFootpaths)
      {
        accessEnterConnection  = std::get<0>(reverseAccessJourneys[accessFootpath.first]);
        if (accessEnterConnection != -1)
        {
          accessTravelTime        = nodesAccessTravelTime[accessFootpath.first];
          accessNodeDepartureTime = std::get<connectionIndexes::TIME_DEP>(reverseConnections[accessEnterConnection]) - accessTravelTime - params.minWaitingTimeSeconds;
          //std::cerr << nodes[accessFootpath.first].name << ": " << accessTravelTime << " t: " << trips[std::get<connectionIndexes::TRIP>(reverseConnections[accessEnterConnection])].id << " - " << Toolbox::convertSecondsToFormattedTime(accessNodeDepartureTime) << std::endl;
          if (accessNodeDepartureTime >= 0 && arrivalTimeSeconds - accessNodeDepartureTime <= params.maxTotalTravelTimeSeconds && accessNodeDepartureTime > bestDepartureTime && accessNodeDepartureTime < MAX_INT)
          {
            bestDepartureTime    = accessNodeDepartureTime;
            bestAccessNodeIndex  = accessFootpath.first;
            bestAccessTravelTime = accessTravelTime;
          }
        }
        i++;
      }
      return std::make_tuple(bestDepartureTime, bestAccessNodeIndex, bestAccessTravelTime);
    }
    else
    {
      return std::make_tuple(-1, -1, -1);
    }


  }

}
