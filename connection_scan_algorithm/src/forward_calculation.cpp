#include "spdlog/spdlog.h"

#include "calculator.hpp"
#include "toolbox.hpp"
#include "parameters.hpp"
#include "node.hpp"
#include "routing_result.hpp"
#include "transit_data.hpp"

namespace TrRouting
{
    
  std::optional<std::tuple<int, std::reference_wrapper<const Node>>> Calculator::forwardCalculation(RouteParameters &parameters)
  {

    int benchmarkingStart  = algorithmCalculationTime.getEpoch();

    int   reachableConnectionsCount       {0};
    int   tripEnterConnectionIndex        {-1};
    int   nodeDepartureTentativeTime      {MAX_INT};
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
    bool  nodeWasAccessedFromOrigin       {false};
    int   bestArrivalTime                 {MAX_INT};
    
    int  connectionsCount  = transitData.getForwardConnections().size();
    int  departureTimeHour = departureTimeSeconds / 3600;

    // main loop:
    //TODO Better identify this i variable
    ////int i = forwardConnectionsIndexPerDepartureTimeHour[departureTimeHour];
    auto lastConnection = transitData.getForwardConnections().end(); // cache last connection for loop
    //TODO Let's ignore the forwardConnectionsIndexPerDepartureTimeHour, it's a small optimization which complexify the code
    // We can replace it by a transitData.GetFCbeginItePerHour function which will cache the result internally 
    ////for(auto connection = transitData.getForwardConnections().begin() + forwardConnectionsIndexPerDepartureTimeHour[departureTimeHour]; connection != lastConnection; ++connection)
    int i = 0;
    for(auto connection = transitData.getForwardConnections().begin(); connection != lastConnection; ++connection)
    {
      
      // ignore connections before departure time + minimum access travel time:
      if ((**connection).getDepartureTime() >= departureTimeSeconds + minAccessTravelTime)
      {
        const Trip & trip = (**connection).getTrip();

        // Cache the current query data overlay si we don't check the hashmap every time
        auto & currentTripQueryOverlay = tripsQueryOverlay[trip.uid];

        // enabled trips only here:
        if (tripsEnabled[trip.uid])
        {
          connectionDepartureTime         = (**connection).getDepartureTime();
          connectionMinWaitingTimeSeconds = (**connection).getMinWaitingTimeOrDefault(parameters.getMinWaitingTimeSeconds());

          // no need to parse next connections if already reached destination from all egress nodes:
          // yes, we mean connectionDepartureTime and not connectionArrivalTime because travel time for each connections, otherwise you can catch a very short/long connection
          if (( !params.returnAllNodesResult
              && reachedAtLeastOneEgressNode
              && maxEgressTravelTime >= 0
              && tentativeEgressNodeArrivalTime < MAX_INT
              && connectionDepartureTime /* ! not connectionArrivalTime */ > tentativeEgressNodeArrivalTime + maxEgressTravelTime
            ) || (connectionDepartureTime - departureTimeSeconds > parameters.getMaxTotalTravelTimeSeconds()))
          {
            break;
          }

          tripEnterConnectionIndex   = currentTripQueryOverlay.enterConnection; // -1 if trip has not yet been used
          const Node &nodeDeparture = (**connection).getDepartureNode();

          // Extract node departure time if we have a result or set a default value
          auto ite = nodesTentativeTime.find(nodeDeparture.uid);
          if (ite != nodesTentativeTime.end()) {
            nodeDepartureTentativeTime = ite->second;
          } else{
            nodeDepartureTentativeTime = MAX_INT;
          }

          auto nodesAccessIte = nodesAccess.find(nodeDeparture.uid);          
          nodeWasAccessedFromOrigin  = parameters.getMaxFirstWaitingTimeSeconds() > 0 &&
            nodesAccessIte != nodesAccess.end() &&
            nodesAccessIte->second.time >= 0 &&
            std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(forwardJourneysSteps.at(nodeDeparture.uid)) == -1;

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
              connectionDepartureTime - nodeDepartureTentativeTime <= parameters.getMaxFirstWaitingTimeSeconds()
            )
          )
          {

            
            
            // TODO: add constrain for sameLineTransfer (check trip allowSameLineTransfers)
            if (
                (**connection).canBoard()
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
              currentTripQueryOverlay.usable = true;
              currentTripQueryOverlay.enterConnection = i;
              currentTripQueryOverlay.enterConnectionTransferTravelTime = std::get<journeyStepIndexes::TRANSFER_TRAVEL_TIME>(forwardJourneysSteps.at(nodeDeparture.uid));
            }
            
            if ((**connection).canUnboard() && currentTripQueryOverlay.enterConnection != -1)
            {
              // get footpaths for the arrival node to get transferable nodes:
              const Node &nodeArrival = (**connection).getArrivalNode();
              connectionArrivalTime           = (**connection).getArrivalTime();

              auto nodeArrivalInNodesEgressIte = nodesEgress.find(nodeArrival.uid);              
              if (!params.returnAllNodesResult && !reachedAtLeastOneEgressNode && nodeArrivalInNodesEgressIte != nodesEgress.end() && nodeArrivalInNodesEgressIte->second.time != -1) // check if the arrival node is egressable
              {
                reachedAtLeastOneEgressNode    = true;
                tentativeEgressNodeArrivalTime = connectionArrivalTime;
              }

              footpathIndex = 0;
              for (const NodeTimeDistance & transferableNode : nodeArrival.transferableNodes)
              {
                // Extract tentative time for current transferable node if found
                int currentTransferablenNodesTentativeTime = 0;
                auto nodesIte = nodesTentativeTime.find(transferableNode.node.uid);
                if (nodesIte != nodesTentativeTime.end()) {
                  currentTransferablenNodesTentativeTime = nodesIte->second;
                } else {
                  currentTransferablenNodesTentativeTime = MAX_INT;
                }
                
                if (nodeArrival.uuid != transferableNode.node.uuid &&
                    currentTransferablenNodesTentativeTime < connectionArrivalTime)
                {
                  footpathIndex++;
                  continue;
                }
  
                footpathTravelTime = params.walkingSpeedFactor == 1.0 ? nodeArrival.transferableNodes[footpathIndex].time : (int)ceil((float)nodeArrival.transferableNodes[footpathIndex].time / params.walkingSpeedFactor);

                if (footpathTravelTime <= parameters.getMaxTransferWalkingTravelTimeSeconds())
                {
                  if (footpathTravelTime + connectionArrivalTime < currentTransferablenNodesTentativeTime)
                  {
                    footpathDistance = nodeArrival.transferableNodes[footpathIndex].distance;
                    nodesTentativeTime[transferableNode.node.uid] = footpathTravelTime + connectionArrivalTime;

                    forwardJourneysSteps.insert({transferableNode.node.uid, std::make_tuple(currentTripQueryOverlay.enterConnection, i, std::cref(trip), footpathTravelTime, (nodeArrival.uuid == transferableNode.node.uuid ? 1 : -1), footpathDistance)});
                  }

                  if (
                    nodeArrival == transferableNode.node
                    && 
                    (
                     //TODO Not fully sure this is equivalent to the ancient code
                     forwardEgressJourneysSteps.count(transferableNode.node.uid) == 0
                      ||
                     (*transitData.getForwardConnections()[std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION>(forwardEgressJourneysSteps.at(transferableNode.node.uid))]).getArrivalTime() > connectionArrivalTime
                    )
                  )
                  {
                    footpathDistance = nodeArrival.transferableNodes[footpathIndex].distance;
                    forwardEgressJourneysSteps.insert({transferableNode.node.uid, std::make_tuple(currentTripQueryOverlay.enterConnection, i, std::cref(trip), footpathTravelTime, 1, footpathDistance)});
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

    spdlog::debug("-- {} forward connections parsed on {}", reachableConnectionsCount, connectionsCount);

    if (!params.returnAllNodesResult && reachableConnectionsCount == 0) {
      throw NoRoutingFoundException(NoRoutingReason::NO_SERVICE_FROM_ORIGIN);
    }

    int egressNodeArrivalTime {-1};
    int egressExitConnection  {-1};
    // find best egress node:
    if (!params.returnAllNodesResult)
    {
      std::optional<std::reference_wrapper<const NodeTimeDistance>> bestEgress;      

      for (auto & egressFootpath : egressFootpaths)
      {
        if (forwardEgressJourneysSteps.count(egressFootpath.node.uid)) {
          egressExitConnection  = std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION>(forwardEgressJourneysSteps.at(egressFootpath.node.uid));
          //TODO Would this be always true with the new previous if
          if (egressExitConnection != -1)
          {
            const NodeTimeDistance &egress = nodesEgress.at(egressFootpath.node.uid);
            egressNodeArrivalTime = (*transitData.getForwardConnections()[egressExitConnection]).getArrivalTime() + egress.time;

            if (egressNodeArrivalTime >= 0 && egressNodeArrivalTime - departureTimeSeconds <= parameters.getMaxTotalTravelTimeSeconds() && egressNodeArrivalTime < bestArrivalTime && egressNodeArrivalTime < MAX_INT)
            {
              bestArrivalTime      = egressNodeArrivalTime;
              bestEgress = egress;
            }
          }
        }
      }

      benchmarking["forward_calculation"] += algorithmCalculationTime.getEpoch() - benchmarkingStart;

      if (bestEgress.has_value()) {
        return std::optional(std::make_tuple(bestArrivalTime, std::cref(bestEgress.value().get().node)));
      } else {
        return std::nullopt;
      }     
    }
    else
    {
      benchmarking["forward_calculation"] += algorithmCalculationTime.getEpoch() - benchmarkingStart;

      // Nothing to return
      return std::nullopt;
    }

  }

}
