#include "calculator.hpp"

namespace TrRouting
{
    
  std::tuple<int,int,int,int> Calculator::reverseCalculation()
  {

    int benchmarkingStart = params.debugDisplay ? algorithmCalculationTime.getEpoch() : 0;

    int  i                                {0};
    int  reachableConnectionsCount        {0};
    int  tripIndex                        {-1};
    int  lineIndex                        {-1};
    //int  blockIndex                       {-1};
    int  nodeDepartureIndex               {-1};
    int  nodeArrivalIndex                 {-1};
    int  tripExitConnectionIndex          {-1};
    int  nodeDepartureTentativeTime       {MAX_INT};
    int  nodeArrivalTentativeTime         {MAX_INT};
    int  connectionDepartureTime          {-1};
    int  connectionArrivalTime            {-1};
    short connectionMinWaitingTimeSeconds {-1};
    short journeyConnectionMinWaitingTimeSeconds {-1};
    //long long  footpathsRangeStart        {-1};
    //long long  footpathsRangeEnd          {-1};
    int  footpathIndex                    {-1};
    int  footpathTravelTime               {-1};
    int  footpathDistance                 {-1};
    int  tentativeAccessNodeDepartureTime {-1};
    bool reachedAtLeastOneAccessNode      {false};
    bool canTransferOnSameLine            {false};
    int  bestAccessNodeIndex              {-1};
    int  bestAccessTravelTime             {-1};
    int  bestAccessDistance               {-1};
    int  bestDepartureTime                {-1};

    boost::uuids::string_generator uuidGenerator;

    int  connectionsCount = reverseConnections.size();
    int  arrivalTimeHour  = arrivalTimeSeconds / 3600;

    // reverse calculation:

    //std::cout << reverseConnectionsIndexPerArrivalTimeHour[arrivalTimeHour] << ":" << arrivalTimeHour << std::endl;

    // main loop for reverse connections:
    i = (arrivalTimeHour + 1 >= reverseConnectionsIndexPerArrivalTimeHour.size()) ? 0 : reverseConnectionsIndexPerArrivalTimeHour[arrivalTimeHour + 1];
    
    auto lastConnection = reverseConnections.end();
    for(auto connection = reverseConnections.begin() + reverseConnectionsIndexPerArrivalTimeHour[arrivalTimeHour + 1]; connection != lastConnection; ++connection)
    {
      // ignore connections after arrival time - minimum egress travel time:
      if (std::get<connectionIndexes::TIME_ARR>(**connection) <= arrivalTimeSeconds - (params.returnAllNodesResult ? 0 : minEgressTravelTime))
      {
        
        tripIndex = std::get<connectionIndexes::TRIP>(**connection);
        
        // enabled trips only here:
        if (tripsUsable[tripIndex] == 1 && tripsEnabled[tripIndex] != -1)
        {

          connectionArrivalTime           = std::get<connectionIndexes::TIME_ARR>(**connection);

          // no need to parse next connections if already reached destination from all egress nodes, except if max travel time is set, so we can get a reverse profile in the next loop calculation:
          // yes, we mean connectionArrivalTime and not connectionDepartureTime because travel time for each connections, otherwise you can catch a very short/long connection
          if ( (!params.returnAllNodesResult && reachedAtLeastOneAccessNode && maxAccessTravelTime >= 0 && connectionArrivalTime /* ! not connectionDepartureTime */ < tentativeAccessNodeDepartureTime - maxAccessTravelTime) || (arrivalTimeSeconds - connectionArrivalTime > params.maxTotalTravelTimeSeconds))
          {
            break;
          }

          tripExitConnectionIndex   = tripsExitConnection[tripIndex];
          nodeArrivalIndex          = std::get<connectionIndexes::NODE_ARR>(**connection);
          nodeArrivalTentativeTime  = nodesReverseTentativeTime[nodeArrivalIndex];
          
          //std::cerr << "nodeArrivalTentativeTime: " << nodeArrivalTentativeTime << " connectionArrivalTime: " << connectionArrivalTime << " tripExitConnectionIndex: " << tripExitConnectionIndex << std::endl;
          
          // reachable connections only here:
          if (
            tripExitConnectionIndex != -1
            ||
            nodeArrivalTentativeTime >= connectionArrivalTime
          )
          {
            
            if (std::get<connectionIndexes::CAN_UNBOARD>(**connection) == 1)
            {

              if (tripExitConnectionIndex == -1) // <= to make sure we get the same result as forward calculation, which uses >
              {
                tripsExitConnection[tripIndex]                   = i;
                tripsExitConnectionTransferTravelTime[tripIndex] = std::get<journeyStepIndexes::TRANSFER_TRAVEL_TIME>(reverseJourneysSteps[nodeArrivalIndex]);
              }
              else if (
                std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(reverseJourneysSteps[nodeArrivalIndex]) != -1
                &&
                std::get<journeyStepIndexes::TRANSFER_TRAVEL_TIME>(reverseJourneysSteps[nodeArrivalIndex]) >= 0 
                && 
                std::get<journeyStepIndexes::TRANSFER_TRAVEL_TIME>(reverseJourneysSteps[nodeArrivalIndex]) < tripsExitConnectionTransferTravelTime[tripIndex]
              )
              {
                journeyConnectionMinWaitingTimeSeconds = params.minWaitingTimeSeconds;
                if(std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(*reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(reverseJourneysSteps[nodeArrivalIndex])].get()) >= 0)
                {
                  journeyConnectionMinWaitingTimeSeconds = std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(*reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(reverseJourneysSteps[nodeArrivalIndex])].get());
                }

                if (connectionArrivalTime + journeyConnectionMinWaitingTimeSeconds <= nodeArrivalTentativeTime)
                {
                  tripsExitConnection[tripIndex]                   = i;
                  tripsExitConnectionTransferTravelTime[tripIndex] = std::get<journeyStepIndexes::TRANSFER_TRAVEL_TIME>(reverseJourneysSteps[nodeArrivalIndex]);
                }
              }

            }
            
            if (std::get<connectionIndexes::CAN_BOARD>(**connection) == 1 && tripsExitConnection[tripIndex] != -1)
            {
              // get footpaths for the arrival node to get transferable nodes:
              nodeDepartureIndex              = std::get<connectionIndexes::NODE_DEP>(**connection);
              connectionDepartureTime         = std::get<connectionIndexes::TIME_DEP>(**connection);
              connectionMinWaitingTimeSeconds = std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(**connection) >= 0 ? std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(**connection) : params.minWaitingTimeSeconds;

              if (!params.returnAllNodesResult && !reachedAtLeastOneAccessNode && nodesAccessTravelTime[nodeDepartureIndex] != -1) // check if the departure node is accessable
              {
                /*if (
                  initialDepartureTimeSeconds == -1
                  ||
                  params.maxFirstWaitingTimeSeconds < connectionMinWaitingTimeSeconds
                  ||
                  connectionDepartureTime - initialDepartureTimeSeconds + nodesAccessTravelTime[nodeDepartureIndex] <= params.maxFirstWaitingTimeSeconds
                )
                {*/
                  reachedAtLeastOneAccessNode      = true;
                  tentativeAccessNodeDepartureTime = connectionDepartureTime;
                //}
              }
              footpathIndex = 0;
              for (int & transferableNodeIndex : nodes[nodeDepartureIndex].get()->reverseTransferableNodesIdx)
              {
                if (nodeDepartureIndex != transferableNodeIndex && nodesReverseTentativeTime[transferableNodeIndex] > connectionDepartureTime - connectionMinWaitingTimeSeconds)
                {
                  footpathIndex++;
                  continue;
                }
                
                footpathTravelTime = params.walkingSpeedFactor == 1.0 ? nodes[nodeDepartureIndex].get()->reverseTransferableTravelTimesSeconds[footpathIndex] : (int)ceil((float)nodes[nodeDepartureIndex].get()->reverseTransferableTravelTimesSeconds[footpathIndex] / params.walkingSpeedFactor);

                if (footpathTravelTime <= params.maxTransferWalkingTravelTimeSeconds)
                {
                  if (connectionDepartureTime - footpathTravelTime - connectionMinWaitingTimeSeconds >= nodesReverseTentativeTime[transferableNodeIndex])
                  {
                    footpathDistance = nodes[nodeDepartureIndex].get()->reverseTransferableDistancesMeters[footpathIndex];
                    nodesReverseTentativeTime[transferableNodeIndex] = connectionDepartureTime - footpathTravelTime - connectionMinWaitingTimeSeconds;
                    reverseJourneysSteps[transferableNodeIndex]           = std::make_tuple(i, tripsExitConnection[tripIndex], nodeDepartureIndex, tripIndex, footpathTravelTime, (nodeDepartureIndex == transferableNodeIndex ? 1 : -1), footpathDistance, -1);
                  }
                  if (
                    nodeDepartureIndex == transferableNodeIndex
                    && 
                    (
                      std::get<journeyStepIndexes::TRANSFER_TRAVEL_TIME>(reverseAccessJourneysSteps[transferableNodeIndex]) == -1 
                      || 
                      std::get<connectionIndexes::TIME_DEP>(*reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(reverseAccessJourneysSteps[transferableNodeIndex])].get()) <= connectionDepartureTime - connectionMinWaitingTimeSeconds
                    )
                  )
                  {
                    if (
                      initialDepartureTimeSeconds == -1
                      ||
                      connectionDepartureTime - nodesAccessTravelTime[nodeDepartureIndex] - connectionMinWaitingTimeSeconds >= initialDepartureTimeSeconds
                    )
                    {
                      if (
                        initialDepartureTimeSeconds == -1
                        ||
                        params.maxFirstWaitingTimeSeconds < connectionMinWaitingTimeSeconds
                        ||
                        connectionDepartureTime - initialDepartureTimeSeconds - nodesAccessTravelTime[nodeDepartureIndex] <= params.maxFirstWaitingTimeSeconds
                      )
                      {
                        reverseAccessJourneysSteps[transferableNodeIndex] = std::make_tuple(i, tripsExitConnection[tripIndex], nodeDepartureIndex, tripIndex, 0, 1, 0, std::get<journeyStepIndexes::NEAREST_NODE_DISTANCE_METERS>(reverseJourneysSteps[transferableNodeIndex]));
                      }
                    }
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

    int accessNodeDepartureTime                    {-1};
    int accessEnterConnection                      {-1};
    int accessTravelTime                           {-1};
    int accessDistance                             {-1};
    int accessEnterConnectionMinWaitingTimeSeconds {-1};
    // find best access node:
    if (!params.returnAllNodesResult)
    {
      i = 0;
      for (auto & accessFootpath : accessFootpaths)
      {
        accessEnterConnection  = std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(reverseAccessJourneysSteps[std::get<0>(accessFootpath)]);
        if (accessEnterConnection != -1)
        {
          accessTravelTime                           = nodesAccessTravelTime[std::get<0>(accessFootpath)];
          accessDistance                             = nodesAccessDistance[std::get<0>(accessFootpath)];
          accessEnterConnectionMinWaitingTimeSeconds = std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(*reverseConnections[accessEnterConnection]) >= 0 ? std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(*reverseConnections[accessEnterConnection]) : params.minWaitingTimeSeconds;
          accessNodeDepartureTime                    = std::get<connectionIndexes::TIME_DEP>(*reverseConnections[accessEnterConnection]) - accessTravelTime - accessEnterConnectionMinWaitingTimeSeconds;

          if (accessNodeDepartureTime >= 0 && arrivalTimeSeconds - accessNodeDepartureTime <= params.maxTotalTravelTimeSeconds && accessNodeDepartureTime > bestDepartureTime && accessNodeDepartureTime < MAX_INT)
          {
            bestDepartureTime    = accessNodeDepartureTime;
            bestAccessNodeIndex  = std::get<0>(accessFootpath);
            bestAccessTravelTime = accessTravelTime;
            bestAccessDistance   = accessDistance;
          }
        }
        i++;
      }

      if (params.debugDisplay)
        benchmarking["reverse_calculation"] += algorithmCalculationTime.getEpoch() - benchmarkingStart;

      return std::make_tuple(bestDepartureTime, bestAccessNodeIndex, bestAccessTravelTime, bestAccessDistance);
    }
    else
    {
      if (params.debugDisplay)
        benchmarking["reverse_calculation"] += algorithmCalculationTime.getEpoch() - benchmarkingStart;
      
      return std::make_tuple(-1, -1, -1, -1);
    }


  }

}
