#include "calculator.hpp"

namespace TrRouting
{
    
  std::tuple<int,int,int,int> Calculator::forwardCalculation()
  {

    int benchmarkingStart  = algorithmCalculationTime.getEpoch();

    int   i                               {0};
    int   reachableConnectionsCount       {0};
    int   tripIndex                       {-1};
    int   lineIndex                       {-1};
    //int   blockIndex                      {-1};
    int   nodeDepartureIndex              {-1};
    int   nodeArrivalIndex                {-1};
    int   tripEnterConnectionIndex        {-1};
    int   nodeDepartureTentativeTime      {MAX_INT};
    int   nodeArrivalTentativeTime        {MAX_INT};
    int   connectionDepartureTime         {-1};
    int   connectionArrivalTime           {-1};
    short connectionMinWaitingTimeSeconds {-1};
    //long long   footpathsRangeStart       {-1};
    //long long   footpathsRangeEnd         {-1};
    int   footpathIndex                   {-1};
    int   footpathTravelTime              {-1};
    int   footpathDistance                {-1};
    int   tentativeEgressNodeArrivalTime  {MAX_INT};
    bool  reachedAtLeastOneEgressNode     {false};
    bool  canTransferOnSameLine           {false};
    bool  nodeWasAccessedFromOrigin       {false};
    int   bestEgressNodeIndex             {-1};
    int   bestEgressTravelTime            {-1};
    int   bestEgressDistance              {-1};
    int   bestArrivalTime                 {MAX_INT};
    
    int  connectionsCount  = forwardConnections.size();
    int  departureTimeHour = departureTimeSeconds / 3600;

    //std::cout << forwardConnectionsIndexPerDepartureTimeHour[departureTimeHour] << ":" << departureTimeHour << std::endl;

    // main loop:
    i = forwardConnectionsIndexPerDepartureTimeHour[departureTimeHour];
    auto lastConnection = forwardConnections.end(); // cache last connection for loop
    for(auto connection = forwardConnections.begin() + forwardConnectionsIndexPerDepartureTimeHour[departureTimeHour]; connection != lastConnection; ++connection)
    {
      
      // ignore connections before departure time + minimum access travel time:
      if (std::get<connectionIndexes::TIME_DEP>(**connection) >= departureTimeSeconds + minAccessTravelTime)
      {
        tripIndex = std::get<connectionIndexes::TRIP>(**connection);

        // enabled trips only here:
        if (tripsEnabled[tripIndex] != -1)
        {
          connectionDepartureTime         = std::get<connectionIndexes::TIME_DEP>(**connection);
          connectionMinWaitingTimeSeconds = std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(**connection) >= 0 ? std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(**connection) : params.minWaitingTimeSeconds;

          // no need to parse next connections if already reached destination from all egress nodes:
          // yes, we mean connectionDepartureTime and not connectionArrivalTime because travel time for each connections, otherwise you can catch a very short/long connection
          if (( !params.returnAllNodesResult
              && reachedAtLeastOneEgressNode
              && maxEgressTravelTime >= 0
              && tentativeEgressNodeArrivalTime < MAX_INT
              && connectionDepartureTime /* ! not connectionArrivalTime */ > tentativeEgressNodeArrivalTime + maxEgressTravelTime
            ) || (connectionDepartureTime - departureTimeSeconds > params.maxTotalTravelTimeSeconds))
          {
            break;
          }

          tripEnterConnectionIndex   = tripsEnterConnection[tripIndex]; // -1 if trip has not yet been used
          nodeDepartureIndex         = std::get<connectionIndexes::NODE_DEP>(**connection);
          nodeDepartureTentativeTime = nodesTentativeTime[nodeDepartureIndex];
          nodeWasAccessedFromOrigin  = params.maxFirstWaitingTimeSeconds > 0 && nodesAccessTravelTime[nodeDepartureIndex] >= 0 && std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(forwardJourneysSteps[nodeDepartureIndex]) == -1;

          // reachable connections only here:
          if (
            (
              tripEnterConnectionIndex != -1 
              || 
              nodeDepartureTentativeTime <= connectionDepartureTime - connectionMinWaitingTimeSeconds
            )
            &&
            (
              !nodeWasAccessedFromOrigin
              ||
              connectionDepartureTime - nodeDepartureTentativeTime <= params.maxFirstWaitingTimeSeconds
            )
          )
          {

            
            
            /* Difficult to deal with blocks and no transfer between same line in CSA algorithm! */
            /*lineIndex             = std::get<connectionIndexes::LINE>(**connection);
            canTransferOnSameLine = std::get<connectionIndexes::CAN_TRANSFER_SAME_LINE>(**connection);
            blockIndex            = std::get<connectionIndexes::BLOCK>(**connection);*/

            // TODO: add constrain for sameLineTransfer (check trip allowSameLineTransfers)
            if (
              std::get<connectionIndexes::CAN_BOARD>(**connection) == 1 
              && 
              (
                tripEnterConnectionIndex == -1 
                /*|| 
                (
                  std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(forwardJourneysSteps[nodeDepartureIndex]) == -1 
                  &&
                  std::get<journeyStepIndexes::TRANSFER_TRAVEL_TIME>(forwardJourneysSteps[nodeDepartureIndex]) >= 0 
                  && 
                  std::get<journeyStepIndexes::TRANSFER_TRAVEL_TIME>(forwardJourneysSteps[nodeDepartureIndex]) < tripsEnterConnectionTransferTravelTime[tripIndex]
                )*/
              )
            )
            {
              tripsUsable[tripIndex]                            = 1;
              tripsEnterConnection[tripIndex]                   = i;
              tripsEnterConnectionTransferTravelTime[tripIndex] = std::get<journeyStepIndexes::TRANSFER_TRAVEL_TIME>(forwardJourneysSteps[nodeDepartureIndex]);
            }
            
            if (std::get<connectionIndexes::CAN_UNBOARD>(**connection) == 1 && tripsEnterConnection[tripIndex] != -1)
            {
              // get footpaths for the arrival node to get transferable nodes:
              nodeArrivalIndex                = std::get<connectionIndexes::NODE_ARR>(**connection);
              connectionArrivalTime           = std::get<connectionIndexes::TIME_ARR>(**connection);

              if (!params.returnAllNodesResult && !reachedAtLeastOneEgressNode && nodesEgressTravelTime[nodeArrivalIndex] != -1) // check if the arrival node is egressable
              {
                reachedAtLeastOneEgressNode    = true;
                tentativeEgressNodeArrivalTime = connectionArrivalTime;
              }
              footpathIndex = 0;
              for (int & transferableNodeIndex : nodes[nodeArrivalIndex].get()->transferableNodesIdx)
              {
                if (nodeArrivalIndex != transferableNodeIndex && nodesTentativeTime[transferableNodeIndex] < connectionArrivalTime)
                {
                  footpathIndex++;
                  continue;
                }
  
                footpathTravelTime = params.walkingSpeedFactor == 1.0 ? nodes[nodeArrivalIndex].get()->transferableTravelTimesSeconds[footpathIndex] : (int)ceil((float)nodes[nodeArrivalIndex].get()->transferableTravelTimesSeconds[footpathIndex] / params.walkingSpeedFactor);

                if (footpathTravelTime <= params.maxTransferWalkingTravelTimeSeconds)
                {
                  if (footpathTravelTime + connectionArrivalTime < nodesTentativeTime[transferableNodeIndex])
                  {
                    footpathDistance = nodes[nodeArrivalIndex].get()->transferableDistancesMeters[footpathIndex];
                    nodesTentativeTime[transferableNodeIndex] = footpathTravelTime + connectionArrivalTime;
                    forwardJourneysSteps[transferableNodeIndex]    = std::make_tuple(tripsEnterConnection[tripIndex], i, nodeArrivalIndex, tripIndex, footpathTravelTime, (nodeArrivalIndex == transferableNodeIndex ? 1 : -1), footpathDistance, -1);
                  }
                  if (
                    nodeArrivalIndex == transferableNodeIndex 
                    && 
                    (
                      std::get<journeyStepIndexes::TRANSFER_TRAVEL_TIME>(forwardEgressJourneysSteps[transferableNodeIndex]) == -1 
                      ||
                      std::get<connectionIndexes::TIME_ARR>(*forwardConnections[std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION>(forwardEgressJourneysSteps[transferableNodeIndex])]) > connectionArrivalTime
                    )
                  )
                  {
                    footpathDistance = nodes[nodeArrivalIndex].get()->transferableDistancesMeters[footpathIndex];
                    forwardEgressJourneysSteps[transferableNodeIndex] = std::make_tuple(tripsEnterConnection[tripIndex], i, nodeArrivalIndex, tripIndex, footpathTravelTime, 1, footpathDistance, -1);
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
    int egressDistance        {-1};
    // find best egress node:
    if (!params.returnAllNodesResult)
    {
      i = 0;
      for (auto & egressFootpath : egressFootpaths)
      {
        //std::cerr << nodes[std::get<0>(egressFootpath)].get()->name << std::endl;
        egressExitConnection  = std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION>(forwardEgressJourneysSteps[std::get<0>(egressFootpath)]);
        if (egressExitConnection != -1)
        {
          egressTravelTime      = nodesEgressTravelTime[std::get<0>(egressFootpath)];
          egressDistance        = nodesEgressDistance[std::get<0>(egressFootpath)];
          egressNodeArrivalTime = std::get<connectionIndexes::TIME_ARR>(*forwardConnections[egressExitConnection]) + egressTravelTime;
          //std::cerr << nodes[std::get<0>(egressFootpath)].get()->name << ": " << egressTravelTime << " - " << Toolbox::convertSecondsToFormattedTime(egressNodeArrivalTime) << std::endl;
          if (egressNodeArrivalTime >= 0 && egressNodeArrivalTime - departureTimeSeconds <= params.maxTotalTravelTimeSeconds && egressNodeArrivalTime < bestArrivalTime && egressNodeArrivalTime < MAX_INT)
          {
            bestArrivalTime      = egressNodeArrivalTime;
            bestEgressNodeIndex  = std::get<0>(egressFootpath);
            bestEgressTravelTime = egressTravelTime;
            bestEgressDistance   = egressDistance;
          }
        }
        i++;
      }

      if (params.debugDisplay)
        benchmarking["forward_calculation"] += algorithmCalculationTime.getEpoch() - benchmarkingStart;

      return std::make_tuple(bestArrivalTime, bestEgressNodeIndex, bestEgressTravelTime, bestEgressDistance);
    }
    else
    {
      if (params.debugDisplay)
        benchmarking["forward_calculation"] += algorithmCalculationTime.getEpoch() - benchmarkingStart;

      return std::make_tuple(MAX_INT, -1, -1, -1);
    }

  }

}
