#include "spdlog/spdlog.h"

#include "calculator.hpp"
#include "toolbox.hpp"
#include "parameters.hpp"
#include "node.hpp"
#include "routing_result.hpp"
#include "transit_data.hpp"
#include "connection_set.hpp"

namespace TrRouting
{
  //TODO Need to divide in smaller chunks and reuse common bits between forwardCalculation and forwardCalculationAllNodes
  std::optional<std::tuple<int, std::reference_wrapper<const Node>>> Calculator::forwardCalculation(RouteParameters &parameters,
                                                                                                    std::unordered_map<Node::uid_t, JourneyStep> & forwardEgressJourneysSteps)
  {
    int   reachableConnectionsCount       {0};
    int   nodeDepartureTentativeTime      {MAX_INT};
    int   connectionDepartureTime         {-1};
    int   connectionArrivalTime           {-1};
    short connectionMinWaitingTimeSeconds {-1};
    //long long   footpathsRangeStart       {-1};
    //long long   footpathsRangeEnd         {-1};
    int   footpathTravelTime              {-1};
    int   footpathDistance                {-1};
    int   tentativeEgressNodeArrivalTime  {MAX_INT};
    bool  reachedAtLeastOneEgressNode     {false};
    bool  nodeWasAccessedFromOrigin       {false};
    int   bestArrivalTime                 {MAX_INT};
    
    int  connectionsCount  = connectionSet.get()->getForwardConnections().size();
    int  departureTimeHour = departureTimeSeconds / 3600;

    // main loop:
    auto lastConnection = connectionSet.get()->getForwardConnections().end(); // cache last connection for loop
    for(auto connection = connectionSet.get()->getForwardConnectionsBeginAtDepartureHour(departureTimeHour); connection != lastConnection; ++connection)
    {
      
      // ignore connections before departure time + minimum access travel time:
      if ((*connection).get().getDepartureTime() >= departureTimeSeconds + minAccessTravelTime)
      {
        const Trip & trip = (*connection).get().getTrip();

        // Cache the current query data overlay si we don't check the hashmap every time
        auto & currentTripQueryOverlay = tripsQueryOverlay.at(trip.uid);

        // enabled trips only here:
        if (!isTripDisabled(trip.uid))
        {
          connectionDepartureTime         = (*connection).get().getDepartureTime();
          connectionMinWaitingTimeSeconds = (*connection).get().getMinWaitingTimeOrDefault(parameters.getMinWaitingTimeSeconds());

          // no need to parse next connections if already reached destination from all egress nodes:
          // yes, we mean connectionDepartureTime and not connectionArrivalTime because travel time for each connections, otherwise you can catch a very short/long connection
          if (( reachedAtLeastOneEgressNode
              && maxEgressTravelTime >= 0
              && tentativeEgressNodeArrivalTime < MAX_INT
              && connectionDepartureTime /* ! not connectionArrivalTime */ > tentativeEgressNodeArrivalTime + maxEgressTravelTime
            ) || (connectionDepartureTime - departureTimeSeconds > parameters.getMaxTotalTravelTimeSeconds()))
          {
            break;
          }
          std::optional<std::reference_wrapper<const Connection>> tripEnterConnection = currentTripQueryOverlay.enterConnection;
          const Node &nodeDeparture = (*connection).get().getDepartureNode();

          // Extract node departure time if we have a result or use default value
          nodeDepartureTentativeTime = nodesTentativeTime.at(nodeDeparture.uid);

          // TODO Do we need to make sure the departure node exists in the forwardJourneySteps map? For the reverse calculation, we had to in order to fix issue https://github.com/chairemobilite/trRouting/issues/250 The issue may apply to forward too, but we have no example
          auto nodesAccessIte = nodesAccess.find(nodeDeparture.uid);          
          nodeWasAccessedFromOrigin  = parameters.getMaxFirstWaitingTimeSeconds() > 0 &&
            nodesAccessIte != nodesAccess.end() &&
            nodesAccessIte->second.time >= 0 &&
            !forwardJourneysSteps.at(nodeDeparture.uid).getFinalEnterConnection().has_value();

          // reachable connections only here:
          if (
            (
             tripEnterConnection.has_value()
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
            if ((*connection).get().canBoard() && (!tripEnterConnection.has_value()) )
            {
              currentTripQueryOverlay.usable = true;
              currentTripQueryOverlay.enterConnection = *connection;
              currentTripQueryOverlay.enterConnectionTransferTravelTime = forwardJourneysSteps.at(nodeDeparture.uid).getTransferTravelTime();
            }
            
            if ((*connection).get().canUnboard() && currentTripQueryOverlay.enterConnection.has_value())
            {
              // get footpaths for the arrival node to get transferable nodes:
              const Node &nodeArrival = (*connection).get().getArrivalNode();
              connectionArrivalTime           = (*connection).get().getArrivalTime();

              auto nodeArrivalInNodesEgressIte = nodesEgress.find(nodeArrival.uid);              
              if (!reachedAtLeastOneEgressNode && nodeArrivalInNodesEgressIte != nodesEgress.end() && nodeArrivalInNodesEgressIte->second.time != -1) // check if the arrival node is egressable
              {
                reachedAtLeastOneEgressNode    = true;
                tentativeEgressNodeArrivalTime = connectionArrivalTime;
              }

              for (const NodeTimeDistance & transferableNode : nodeArrival.transferableNodes)
              {
                // Extract tentative time for current transferable node if found
                int currentTransferablenNodesTentativeTime = nodesTentativeTime.at(transferableNode.node.uid);

                if (nodeArrival != transferableNode.node &&
                    currentTransferablenNodesTentativeTime < connectionArrivalTime)
                {
                  continue;
                }

                //TODO We should not do a direct == with float values
                footpathTravelTime = parameters.getWalkingSpeedFactor() == 1.0 ? transferableNode.time : (int)ceil((float)transferableNode.time / parameters.getWalkingSpeedFactor());

                if (footpathTravelTime <= parameters.getMaxTransferWalkingTravelTimeSeconds())
                {
                  if (footpathTravelTime + connectionArrivalTime < currentTransferablenNodesTentativeTime)
                  {
                    footpathDistance = transferableNode.distance;
                    nodesTentativeTime[transferableNode.node.uid] = footpathTravelTime + connectionArrivalTime;

                    //TODO DO we need a make_optional here??
                    forwardJourneysSteps.at(transferableNode.node.uid) = JourneyStep(currentTripQueryOverlay.enterConnection, *connection, std::cref(trip), footpathTravelTime, (nodeArrival == transferableNode.node), footpathDistance);
                  }

                  if (
                    nodeArrival == transferableNode.node
                    && 
                    (
                     //TODO Not fully sure this is equivalent to the ancient code
                     forwardEgressJourneysSteps.count(transferableNode.node.uid) == 0
                      ||
                     forwardEgressJourneysSteps.at(transferableNode.node.uid).getFinalExitConnection().value().get().getArrivalTime() > connectionArrivalTime
                    )
                  )
                  {
                    footpathDistance = transferableNode.distance;
                    forwardEgressJourneysSteps.insert_or_assign(transferableNode.node.uid, JourneyStep(currentTripQueryOverlay.enterConnection, *connection, std::cref(trip), footpathTravelTime, true, footpathDistance));
                  }
                }
              }
            }
            reachableConnectionsCount++;
          }
        }
      }
    }

    spdlog::debug("-- {} forward connections parsed on {}", reachableConnectionsCount, connectionsCount);

    if (reachableConnectionsCount == 0) {
      throw NoRoutingFoundException(NoRoutingReason::NO_SERVICE_FROM_ORIGIN);
    }

    int egressNodeArrivalTime {-1};
    std::optional<std::reference_wrapper<const Connection>> egressExitConnection;
    // find best egress node:

    std::optional<std::reference_wrapper<const NodeTimeDistance>> bestEgress;

    for (auto & egressFootpath : egressFootpaths)
    {
      if (forwardEgressJourneysSteps.count(egressFootpath.node.uid)) {
        egressExitConnection  = forwardEgressJourneysSteps.at(egressFootpath.node.uid).getFinalExitConnection();
        //TODO We seem to always insert a JourneyStep with a valid exit connection, so the if might be unnecessary
        if (egressExitConnection.has_value())
        {
          const NodeTimeDistance &egress = nodesEgress.at(egressFootpath.node.uid);
          egressNodeArrivalTime = egressExitConnection.value().get().getArrivalTime() + egress.time;

          if (egressNodeArrivalTime >= 0 && egressNodeArrivalTime - departureTimeSeconds <= parameters.getMaxTotalTravelTimeSeconds() && egressNodeArrivalTime < bestArrivalTime && egressNodeArrivalTime < MAX_INT)
          {
            bestArrivalTime      = egressNodeArrivalTime;
            bestEgress = egress;
          }
        } else {
          assert(false); //Should never happen, see TODO above
        }        
      }
    }

    if (bestEgress.has_value()) {
      return std::optional(std::make_tuple(bestArrivalTime, std::cref(bestEgress.value().get().node)));
    } else {
      return std::nullopt;
    }
  }


  void Calculator::forwardCalculationAllNodes(AccessibilityParameters &parameters,
                                               std::unordered_map<Node::uid_t, JourneyStep> & forwardEgressJourneysSteps)
  {
    int   reachableConnectionsCount       {0};
    int   nodeDepartureTentativeTime      {MAX_INT};
    int   connectionDepartureTime         {-1};
    int   connectionArrivalTime           {-1};
    short connectionMinWaitingTimeSeconds {-1};
    //long long   footpathsRangeStart       {-1};
    //long long   footpathsRangeEnd         {-1};
    int   footpathTravelTime              {-1};
    int   footpathDistance                {-1};
    bool  nodeWasAccessedFromOrigin       {false};

    int  connectionsCount  = connectionSet.get()->getForwardConnections().size();
    int  departureTimeHour = departureTimeSeconds / 3600;

    // main loop:
    auto lastConnection = connectionSet.get()->getForwardConnections().end(); // cache last connection for loop
    for(auto connection = connectionSet.get()->getForwardConnectionsBeginAtDepartureHour(departureTimeHour); connection != lastConnection; ++connection)
    {
      // ignore connections before departure time + minimum access travel time:
      if ((*connection).get().getDepartureTime() >= departureTimeSeconds + minAccessTravelTime)
      {
        const Trip & trip = (*connection).get().getTrip();

        // Cache the current query data overlay si we don't check the hashmap every time
        auto & currentTripQueryOverlay = tripsQueryOverlay.at(trip.uid);

        // enabled trips only here:
        if (!isTripDisabled(trip.uid))
        {
          connectionDepartureTime         = (*connection).get().getDepartureTime();
          connectionMinWaitingTimeSeconds = (*connection).get().getMinWaitingTimeOrDefault(parameters.getMinWaitingTimeSeconds());

          //TODO When the allNodes clean, is the comment still releveant?
          // no need to parse next connections if already reached destination from all egress nodes:
          // yes, we mean connectionDepartureTime and not connectionArrivalTime because travel time for each connections, otherwise you can catch a very short/long connection
          if (connectionDepartureTime - departureTimeSeconds > parameters.getMaxTotalTravelTimeSeconds())
          {
            break;
          }
          std::optional<std::reference_wrapper<const Connection>> tripEnterConnection = currentTripQueryOverlay.enterConnection;
          const Node &nodeDeparture = (*connection).get().getDepartureNode();

          // Extract node departure time if we have a result or use default value
          nodeDepartureTentativeTime = nodesTentativeTime.at(nodeDeparture.uid);

          auto nodesAccessIte = nodesAccess.find(nodeDeparture.uid);
          nodeWasAccessedFromOrigin  = parameters.getMaxFirstWaitingTimeSeconds() > 0 &&
            nodesAccessIte != nodesAccess.end() &&
            nodesAccessIte->second.time >= 0 &&
            !forwardJourneysSteps.at(nodeDeparture.uid).getFinalEnterConnection().has_value();

          // reachable connections only here:
          if (
            (
             tripEnterConnection.has_value()
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
            if ((*connection).get().canBoard() && ( !tripEnterConnection.has_value()))
            {
              currentTripQueryOverlay.usable = true;
              currentTripQueryOverlay.enterConnection = *connection;
              currentTripQueryOverlay.enterConnectionTransferTravelTime = forwardJourneysSteps.at(nodeDeparture.uid).getTransferTravelTime();
            }

            if ((*connection).get().canUnboard() && currentTripQueryOverlay.enterConnection.has_value())
            {
              // get footpaths for the arrival node to get transferable nodes:
              const Node &nodeArrival = (*connection).get().getArrivalNode();
              connectionArrivalTime           = (*connection).get().getArrivalTime();

              for (const NodeTimeDistance & transferableNode : nodeArrival.transferableNodes)
              {
                // Extract tentative time for current transferable node if found
                int currentTransferablenNodesTentativeTime = nodesTentativeTime.at(transferableNode.node.uid);

                if (nodeArrival != transferableNode.node &&
                    currentTransferablenNodesTentativeTime < connectionArrivalTime)
                {
                  continue;
                }

                //TODO We should not do a direct == with float values
                footpathTravelTime = parameters.getWalkingSpeedFactor() == 1.0 ? transferableNode.time : (int)ceil((float)transferableNode.time / parameters.getWalkingSpeedFactor());

                if (footpathTravelTime <= parameters.getMaxTransferWalkingTravelTimeSeconds())
                {
                  if (footpathTravelTime + connectionArrivalTime < currentTransferablenNodesTentativeTime)
                  {
                    footpathDistance = transferableNode.distance;
                    nodesTentativeTime[transferableNode.node.uid] = footpathTravelTime + connectionArrivalTime;

                    //TODO DO we need a make_optional here??
                    forwardJourneysSteps.at(transferableNode.node.uid) = JourneyStep(currentTripQueryOverlay.enterConnection, *connection, std::cref(trip), footpathTravelTime, (nodeArrival == transferableNode.node), footpathDistance);
                  }

                  if (
                    nodeArrival == transferableNode.node
                    &&
                    (
                     //TODO Not fully sure this is equivalent to the ancient code
                     forwardEgressJourneysSteps.count(transferableNode.node.uid) == 0
                      ||
                     forwardEgressJourneysSteps.at(transferableNode.node.uid).getFinalExitConnection().value().get().getArrivalTime() > connectionArrivalTime
                    )
                  )
                  {
                    footpathDistance = transferableNode.distance;
                    forwardEgressJourneysSteps.insert_or_assign(transferableNode.node.uid, JourneyStep(currentTripQueryOverlay.enterConnection, *connection, std::cref(trip), footpathTravelTime, true, footpathDistance));
                  }
                }
              }
            }
            reachableConnectionsCount++;
          }
        }
      }
    }

    spdlog::debug("-- {} forward connections parsed on {}", reachableConnectionsCount, connectionsCount);

    if (reachableConnectionsCount == 0) {
      throw NoRoutingFoundException(NoRoutingReason::NO_SERVICE_FROM_ORIGIN);
    }

  }

}
