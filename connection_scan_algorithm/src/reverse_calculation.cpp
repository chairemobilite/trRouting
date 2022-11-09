#include <boost/uuid/string_generator.hpp>
#include "spdlog/spdlog.h"

#include "calculator.hpp"
#include "toolbox.hpp"
#include "parameters.hpp"
#include "node.hpp"
#include "routing_result.hpp"

namespace TrRouting
{
    
  std::optional<std::tuple<int, std::reference_wrapper<const Node>>> Calculator::reverseCalculation(RouteParameters &parameters)
  {

    int benchmarkingStart = algorithmCalculationTime.getEpoch();

    int  i                                {0};
    int  reachableConnectionsCount        {0};
    int  tripExitConnectionIndex          {-1};
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
    int  bestDepartureTime                {-1};

    int  connectionsCount = reverseConnections.size();
    int  arrivalTimeHour  = arrivalTimeSeconds / 3600;

    // reverse calculation:

    // main loop for reverse connections:
    i = (arrivalTimeHour + 1 >= reverseConnectionsIndexPerArrivalTimeHour.size()) ? 0 : reverseConnectionsIndexPerArrivalTimeHour[arrivalTimeHour + 1];
    
    auto lastConnection = reverseConnections.end();
    for(auto connection = reverseConnections.begin() + reverseConnectionsIndexPerArrivalTimeHour[arrivalTimeHour + 1]; connection != lastConnection; ++connection)
    {
      // ignore connections after arrival time - minimum egress travel time:
      if (std::get<connectionIndexes::TIME_ARR>(**connection) <= arrivalTimeSeconds - (params.returnAllNodesResult ? 0 : minEgressTravelTime))
      {
        
        const Trip & trip = std::get<connectionIndexes::TRIP>(**connection);
        
        // enabled trips only here:
        auto & currentTripQueryOverlay = tripsQueryOverlay[trip.uid];
        if (currentTripQueryOverlay.usable && tripsEnabled[trip.uid])
        {

          connectionArrivalTime           = std::get<connectionIndexes::TIME_ARR>(**connection);

          // no need to parse next connections if already reached destination from all egress nodes, except if max travel time is set, so we can get a reverse profile in the next loop calculation:
          // yes, we mean connectionArrivalTime and not connectionDepartureTime because travel time for each connections, otherwise you can catch a very short/long connection
          if ( (!params.returnAllNodesResult && reachedAtLeastOneAccessNode && maxAccessTravelTime >= 0 && connectionArrivalTime /* ! not connectionDepartureTime */ < tentativeAccessNodeDepartureTime - maxAccessTravelTime) || (arrivalTimeSeconds - connectionArrivalTime > parameters.getMaxTotalTravelTimeSeconds()))
          {
            break;
          }

          tripExitConnectionIndex   = currentTripQueryOverlay.exitConnection;
          const Node &nodeArrival = std::get<connectionIndexes::NODE_ARR>(**connection);

          // Extract node arrival time
          int nodeArrivalTentativeTime  = -1;
          auto ite = nodesReverseTentativeTime.find(nodeArrival.uid);
          if (ite != nodesReverseTentativeTime.end()) {
            nodeArrivalTentativeTime = ite->second;
          } else {
            nodeArrivalTentativeTime = -1;
          }

          // reachable connections only here:
          if (
            tripExitConnectionIndex != -1
            ||
            nodeArrivalTentativeTime >= connectionArrivalTime
          )
          {
            
            if (std::get<connectionIndexes::CAN_UNBOARD>(**connection) == 1)
            {
              // TODO probably extract the reverseJourneysSteps[nodeArrival.uuid] instead of doing lookup multiple the time
              if (tripExitConnectionIndex == -1) // <= to make sure we get the same result as forward calculation, which uses >
              {
                currentTripQueryOverlay.exitConnection = i;
                currentTripQueryOverlay.exitConnectionTransferTravelTime = std::get<journeyStepIndexes::TRANSFER_TRAVEL_TIME>(reverseJourneysSteps.at(nodeArrival.uid));
              }
              else if (
                       //TODO This was commented out in forward_calculation
                       std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(reverseJourneysSteps.at(nodeArrival.uid)) != -1
                       &&
                       std::get<journeyStepIndexes::TRANSFER_TRAVEL_TIME>(reverseJourneysSteps.at(nodeArrival.uid)) >= 0
                       && 
                       std::get<journeyStepIndexes::TRANSFER_TRAVEL_TIME>(reverseJourneysSteps.at(nodeArrival.uid)) < currentTripQueryOverlay.exitConnectionTransferTravelTime
                       )
              {
                journeyConnectionMinWaitingTimeSeconds = parameters.getMinWaitingTimeSeconds();
                if(std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(*reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(reverseJourneysSteps.at(nodeArrival.uid))].get()) >= 0)
                {
                  journeyConnectionMinWaitingTimeSeconds = std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(*reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(reverseJourneysSteps.at(nodeArrival.uid))].get());
                }

                if (connectionArrivalTime + journeyConnectionMinWaitingTimeSeconds <= nodeArrivalTentativeTime)
                {
                  currentTripQueryOverlay.exitConnection = i;
                  currentTripQueryOverlay.exitConnectionTransferTravelTime = std::get<journeyStepIndexes::TRANSFER_TRAVEL_TIME>(reverseJourneysSteps.at(nodeArrival.uid));
                }
              }

            }
            
            if (std::get<connectionIndexes::CAN_BOARD>(**connection) == 1 && currentTripQueryOverlay.exitConnection != -1)
            {
              // get footpaths for the arrival node to get transferable nodes:
              const Node &nodeDeparture = std::get<connectionIndexes::NODE_DEP>(**connection);
              connectionDepartureTime         = std::get<connectionIndexes::TIME_DEP>(**connection);
              connectionMinWaitingTimeSeconds = std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(**connection) >= 0 ? std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(**connection) : parameters.getMinWaitingTimeSeconds();

              auto nodeDepartureInNodesAccessIte = nodesAccess.find(nodeDeparture.uid);
              if (!params.returnAllNodesResult && !reachedAtLeastOneAccessNode &&  nodeDepartureInNodesAccessIte != nodesAccess.end() &&  nodeDepartureInNodesAccessIte->second.time != -1) // check if the departure node is accessable
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
              for (const NodeTimeDistance & transferableNode : nodeDeparture.reverseTransferableNodes)
              {

                auto trite = nodesReverseTentativeTime.find(transferableNode.node.uid);

                if (nodeDeparture.uuid != transferableNode.node.uuid && trite != nodesReverseTentativeTime.end() && trite->second > connectionDepartureTime - connectionMinWaitingTimeSeconds)
                {
                  footpathIndex++;
                  continue;
                }
                
                footpathTravelTime = params.walkingSpeedFactor == 1.0 ? nodeDeparture.reverseTransferableNodes[footpathIndex].time : (int)ceil((float)nodeDeparture.reverseTransferableNodes[footpathIndex].time / params.walkingSpeedFactor);

                if (footpathTravelTime <= parameters.getMaxTransferWalkingTravelTimeSeconds())
                {                  
                  if ((trite == nodesReverseTentativeTime.end()) || //TODO Unclear if it's equivalent than previous code
                      connectionDepartureTime - footpathTravelTime - connectionMinWaitingTimeSeconds >= trite->second)
                  {
                    footpathDistance = nodeDeparture.reverseTransferableNodes.at(footpathIndex).distance;
                    nodesReverseTentativeTime[transferableNode.node.uid] = connectionDepartureTime - footpathTravelTime - connectionMinWaitingTimeSeconds;
                    reverseJourneysSteps[transferableNode.node.uid] = std::make_tuple(i, currentTripQueryOverlay.exitConnection, std::cref(trip), footpathTravelTime, (nodeDeparture.uuid == transferableNode.node.uuid ? 1 : -1), footpathDistance);
                  }
                  if (
                    nodeDeparture.uuid == transferableNode.node.uuid
                    && 
                    (
                     //TODO Really not sure this is equivalent
                     reverseAccessJourneysSteps.count(transferableNode.node.uid) == 0
                     || 
                     std::get<connectionIndexes::TIME_DEP>(*reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(reverseAccessJourneysSteps.at(transferableNode.node.uid))].get()) <= connectionDepartureTime - connectionMinWaitingTimeSeconds
                    )
                  )
                  {                    
                    if (
                      initialDepartureTimeSeconds == -1
                      ||
                      (nodeDepartureInNodesAccessIte != nodesAccess.end() &&
                       connectionDepartureTime - nodeDepartureInNodesAccessIte->second.time - connectionMinWaitingTimeSeconds >= initialDepartureTimeSeconds
                       ))
                    {
                      if (
                        initialDepartureTimeSeconds == -1
                        ||
                        parameters.getMaxFirstWaitingTimeSeconds() < connectionMinWaitingTimeSeconds
                        ||
                        connectionDepartureTime - initialDepartureTimeSeconds - nodeDepartureInNodesAccessIte->second.time <= parameters.getMaxFirstWaitingTimeSeconds()
                      )
                      {
                        reverseAccessJourneysSteps[transferableNode.node.uid] = std::make_tuple(i, currentTripQueryOverlay.exitConnection, std::cref(trip), 0, 1, 0);
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
    
    spdlog::debug("-- {}  reverse connections parsed on {}", reachableConnectionsCount, connectionsCount);

    if (!params.returnAllNodesResult && reachableConnectionsCount == 0) {
      throw NoRoutingFoundException(NoRoutingReason::NO_SERVICE_TO_DESTINATION);
    }

    // find best access node:
    if (!params.returnAllNodesResult)
    {
      std::optional<std::reference_wrapper<const NodeTimeDistance>> bestAccess;

      for (auto & accessFootpath : accessFootpaths)
      {
        if (reverseAccessJourneysSteps.count(accessFootpath.node.uid)) {
          int accessEnterConnection  = std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(reverseAccessJourneysSteps.at(accessFootpath.node.uid));
          if (accessEnterConnection != -1) //TODO is this check required with the previous if(count()) added ?
          {
            const NodeTimeDistance & access = nodesAccess.at(accessFootpath.node.uid);
            int accessEnterConnectionMinWaitingTimeSeconds = std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(*reverseConnections[accessEnterConnection]) >= 0 ? std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(*reverseConnections[accessEnterConnection]) : parameters.getMinWaitingTimeSeconds();

            int accessNodeDepartureTime                    = std::get<connectionIndexes::TIME_DEP>(*reverseConnections[accessEnterConnection]) - access.time - accessEnterConnectionMinWaitingTimeSeconds;
            if ((accessNodeDepartureTime >= 0) &&
                (arrivalTimeSeconds - accessNodeDepartureTime <= parameters.getMaxTotalTravelTimeSeconds()) &&
                (accessNodeDepartureTime > bestDepartureTime) &&
                (accessNodeDepartureTime < MAX_INT))          
            {
              bestDepartureTime    = accessNodeDepartureTime;
              bestAccess = access;
            }
          }
        }
      }

      benchmarking["reverse_calculation"] += algorithmCalculationTime.getEpoch() - benchmarkingStart;

      if (bestAccess.has_value()) {
        return std::optional(std::make_tuple(bestDepartureTime, std::cref(bestAccess.value().get().node)));
      } else {
        return std::nullopt;
      }      
        
    }
    else
    {
      benchmarking["reverse_calculation"] += algorithmCalculationTime.getEpoch() - benchmarkingStart;

      return std::nullopt;
    }
  }
}
