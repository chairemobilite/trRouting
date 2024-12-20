#include <boost/uuid/string_generator.hpp>
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
    
  std::optional<std::tuple<int, std::reference_wrapper<const Node>>> Calculator::reverseCalculation(RouteParameters &parameters,
                                                                                                    std::unordered_map<Node::uid_t, JourneyStep> & reverseAccessJourneysSteps)
  {
    int  reachableConnectionsCount        {0};
    std::optional<std::reference_wrapper<const Connection>> tripExitConnection;
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

    //TODO could be passed as a parameter
    auto & reverseConnections = connectionSet.get()->getReverseConnections();
    
    int  connectionsCount = reverseConnections.size();
    int  arrivalTimeHour  = arrivalTimeSeconds / 3600;

    // reverse calculation:

    // main loop for reverse connections:
    auto lastConnection = reverseConnections.end();
    for(auto connection = connectionSet.get()->getReverseConnectionsBeginAtArrivalHour(arrivalTimeHour + 1); connection != lastConnection; ++connection)
    {
      // ignore connections after arrival time - minimum egress travel time:
      if ((*connection).get().getArrivalTime() <= arrivalTimeSeconds - minEgressTravelTime)
      {
        
        const Trip & trip = (*connection).get().getTrip();
        
        // enabled trips only here:
        auto & currentTripQueryOverlay = tripsQueryOverlay.at(trip.uid);
        if (currentTripQueryOverlay.usable && !isTripDisabled(trip.uid))
        {

          connectionArrivalTime           = (*connection).get().getArrivalTime();

          // no need to parse next connections if already reached destination from all egress nodes, except if max travel time is set, so we can get a reverse profile in the next loop calculation:
          // yes, we mean connectionArrivalTime and not connectionDepartureTime because travel time for each connections, otherwise you can catch a very short/long connection
          if ( (reachedAtLeastOneAccessNode && maxAccessTravelTime >= 0 && connectionArrivalTime /* ! not connectionDepartureTime */ < tentativeAccessNodeDepartureTime - maxAccessTravelTime) || (arrivalTimeSeconds - connectionArrivalTime > parameters.getMaxTotalTravelTimeSeconds()))
          {
            break;
          }

          tripExitConnection   = currentTripQueryOverlay.exitConnection;
          const Node &nodeArrival = (*connection).get().getArrivalNode();

          // Extract node arrival time
          int nodeArrivalTentativeTime = nodesReverseTentativeTime.at(nodeArrival.uid);

          // reachable connections only here:
          if (
              tripExitConnection.has_value()
            ||
            nodeArrivalTentativeTime >= connectionArrivalTime
          )
          {
            
            // Make sure the arrival of the connection can unboard and is a candidate for the journey. Nodes are candidate if they can either reach the destination by foot, or a connection that can reach the destination within the specified parameters.
            if ((*connection).get().canUnboard())
            {
              // Extract journeyStep once from map
              const JourneyStep & reverseStepAtArrival = reverseJourneysSteps.at(nodeArrival.uid);
              if (!tripExitConnection.has_value()) // <= to make sure we get the same result as forward calculation, which uses >
              {
                currentTripQueryOverlay.exitConnection = *connection;
                currentTripQueryOverlay.exitConnectionTransferTravelTime = reverseStepAtArrival.getTransferTravelTime();
              }
              else if (
                       //TODO This was commented out in forward_calculation
                       reverseStepAtArrival.getFinalEnterConnection().has_value()
                       &&
                       reverseStepAtArrival.getTransferTravelTime() >= 0
                       && 
                       reverseStepAtArrival.getTransferTravelTime() < currentTripQueryOverlay.exitConnectionTransferTravelTime
                       )
              {
                journeyConnectionMinWaitingTimeSeconds = reverseStepAtArrival.getFinalEnterConnection().value().get().getMinWaitingTimeOrDefault(parameters.getMinWaitingTimeSeconds());

                if (connectionArrivalTime + journeyConnectionMinWaitingTimeSeconds <= nodeArrivalTentativeTime)
                {
                  currentTripQueryOverlay.exitConnection = *connection;
                  currentTripQueryOverlay.exitConnectionTransferTravelTime = reverseStepAtArrival.getTransferTravelTime();
                }
              }

            }
            
            if ((*connection).get().canBoard() && currentTripQueryOverlay.exitConnection.has_value())
            {
              // get footpaths for the arrival node to get transferable nodes:
              const Node &nodeDeparture = (*connection).get().getDepartureNode();
              connectionDepartureTime         = (*connection).get().getDepartureTime();
              connectionMinWaitingTimeSeconds = (*connection).get().getMinWaitingTimeOrDefault(parameters.getMinWaitingTimeSeconds());

              auto nodeDepartureInNodesAccessIte = nodesAccess.find(nodeDeparture.uid);
              if (!reachedAtLeastOneAccessNode &&  nodeDepartureInNodesAccessIte != nodesAccess.end() &&  nodeDepartureInNodesAccessIte->second.time != -1) // check if the departure node is accessable
              {
                reachedAtLeastOneAccessNode      = true;
                tentativeAccessNodeDepartureTime = connectionDepartureTime;

              }

              for (const NodeTimeDistance & transferableNode : nodeDeparture.reverseTransferableNodes)
              {

                if (nodeDeparture != transferableNode.node && nodesReverseTentativeTime.at(transferableNode.node.uid) > connectionDepartureTime - connectionMinWaitingTimeSeconds)
                {
                  footpathIndex++;
                  continue;
                }

                //TODO We should not do a direct == with float values
                footpathTravelTime = parameters.getWalkingSpeedFactor() == 1.0 ? transferableNode.time : (int)ceil((float)transferableNode.time / parameters.getWalkingSpeedFactor());

                if (footpathTravelTime <= parameters.getMaxTransferWalkingTravelTimeSeconds())
                {                  
                  if (connectionDepartureTime - footpathTravelTime - connectionMinWaitingTimeSeconds >= nodesReverseTentativeTime.at(transferableNode.node.uid))
                  {
                    footpathDistance = transferableNode.distance;
                    nodesReverseTentativeTime[transferableNode.node.uid] = connectionDepartureTime - footpathTravelTime - connectionMinWaitingTimeSeconds;
                    //TODO Do we need a make_optional<...>(connection) ??
                    reverseJourneysSteps.at(transferableNode.node.uid) =  JourneyStep(*connection, currentTripQueryOverlay.exitConnection, std::cref(trip), footpathTravelTime, (nodeDeparture == transferableNode.node), footpathDistance);
                  }
                  if (
                    nodeDeparture == transferableNode.node
                    && 
                    (
                     //TODO Really not sure this is equivalent
                     reverseAccessJourneysSteps.count(transferableNode.node.uid) == 0
                     || 
                     reverseAccessJourneysSteps.at(transferableNode.node.uid).getFinalEnterConnection().value().get().getDepartureTime() <= connectionDepartureTime - connectionMinWaitingTimeSeconds
                    )
                  )
                  {                    
                    if (
                      departureTimeSeconds == -1
                      ||
                      (nodeDepartureInNodesAccessIte != nodesAccess.end() &&
                       connectionDepartureTime - nodeDepartureInNodesAccessIte->second.time - connectionMinWaitingTimeSeconds >= departureTimeSeconds
                       ))
                    {
                      if (
                        departureTimeSeconds == -1
                        ||
                        parameters.getMaxFirstWaitingTimeSeconds() < connectionMinWaitingTimeSeconds
                        ||
                        connectionDepartureTime - departureTimeSeconds - nodeDepartureInNodesAccessIte->second.time <= parameters.getMaxFirstWaitingTimeSeconds()
                      )
                      {
                        reverseAccessJourneysSteps.insert_or_assign(transferableNode.node.uid, JourneyStep(*connection, currentTripQueryOverlay.exitConnection, std::cref(trip), 0, true, 0));
                      }
                    }
                  }
                }
              }
            }
            reachableConnectionsCount++;
          }
        }
      }
    }
    
    spdlog::debug("-- {}  reverse connections parsed on {}", reachableConnectionsCount, connectionsCount);

    if (reachableConnectionsCount == 0) {
      throw NoRoutingFoundException(NoRoutingReason::NO_SERVICE_TO_DESTINATION);
    }

    // find best access node:
    std::optional<std::reference_wrapper<const NodeTimeDistance>> bestAccess;

    for (auto & accessFootpath : accessFootpaths)
    {
      if (reverseAccessJourneysSteps.count(accessFootpath.node.uid)) {
        std::optional<std::reference_wrapper<const Connection>> accessEnterConnection  = reverseAccessJourneysSteps.at(accessFootpath.node.uid).getFinalEnterConnection();
        if (accessEnterConnection.has_value()) //TODO is this check required with the previous if(count()) added ?
        {
          const NodeTimeDistance & access = nodesAccess.at(accessFootpath.node.uid);
          int accessEnterConnectionMinWaitingTimeSeconds = accessEnterConnection.value().get().getMinWaitingTimeOrDefault(parameters.getMinWaitingTimeSeconds());

          int accessNodeDepartureTime                    = accessEnterConnection.value().get().getDepartureTime() - access.time - accessEnterConnectionMinWaitingTimeSeconds;
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

    if (bestAccess.has_value()) {
      return std::optional(std::make_tuple(bestDepartureTime, std::cref(bestAccess.value().get().node)));
    } else {
      return std::nullopt;
    }
  }


  void Calculator::reverseCalculationAllNodes(AccessibilityParameters &parameters,
                                              std::unordered_map<Node::uid_t, JourneyStep> & reverseAccessJourneysSteps)
  {
    int  reachableConnectionsCount        {0};
    std::optional<std::reference_wrapper<const Connection>> tripExitConnection;
    int  connectionDepartureTime          {-1};
    int  connectionArrivalTime            {-1};
    short connectionMinWaitingTimeSeconds {-1};
    short journeyConnectionMinWaitingTimeSeconds {-1};
    //long long  footpathsRangeStart        {-1};
    //long long  footpathsRangeEnd          {-1};
    int  footpathTravelTime               {-1};
    int  footpathDistance                 {-1};

    //TODO could be passed as a parameter
    auto & reverseConnections = connectionSet.get()->getReverseConnections();

    int  connectionsCount = reverseConnections.size();
    int  arrivalTimeHour  = arrivalTimeSeconds / 3600;

    // reverse calculation:

    // main loop for reverse connections:
    auto lastConnection = reverseConnections.end();
    for(auto connection = connectionSet.get()->getReverseConnectionsBeginAtArrivalHour(arrivalTimeHour + 1); connection != lastConnection; ++connection)
    {
      // ignore connections after arrival time - minimum egress travel time:
      if ((*connection).get().getArrivalTime() <= arrivalTimeSeconds)
      {
        const Trip & trip = (*connection).get().getTrip();

        // enabled trips only here:
        auto & currentTripQueryOverlay = tripsQueryOverlay.at(trip.uid);
        // FIXME Determine with the new connection cache if a trip could be disabled in the all nodes path
        if ((currentTripQueryOverlay.usable) && !isTripDisabled(trip.uid))
        {
          connectionArrivalTime           = (*connection).get().getArrivalTime();

          // no need to parse next connections if already reached destination from all egress nodes, except if max travel time is set, so we can get a reverse profile in the next loop calculation:
          // yes, we mean connectionArrivalTime and not connectionDepartureTime because travel time for each connections, otherwise you can catch a very short/long connection
          if ( arrivalTimeSeconds - connectionArrivalTime > parameters.getMaxTotalTravelTimeSeconds())
          {
            break;
          }

          tripExitConnection   = currentTripQueryOverlay.exitConnection;
          const Node &nodeArrival = (*connection).get().getArrivalNode();

          // Extract node arrival time
          int nodeArrivalTentativeTime = nodesReverseTentativeTime.at(nodeArrival.uid);

          // reachable connections only here:
          if (
              tripExitConnection.has_value()
            ||
            nodeArrivalTentativeTime >= connectionArrivalTime
          )
          {
            if ((*connection).get().canUnboard())
            {
              // Extract journeyStep once from map
              const JourneyStep & reverseStepAtArrival = reverseJourneysSteps.at(nodeArrival.uid);
              if (!tripExitConnection.has_value()) // <= to make sure we get the same result as forward calculation, which uses >
              {
                currentTripQueryOverlay.exitConnection = *connection;
                currentTripQueryOverlay.exitConnectionTransferTravelTime = reverseStepAtArrival.getTransferTravelTime();
              }
              else if (
                       //TODO This was commented out in forward_calculation
                       reverseStepAtArrival.getFinalEnterConnection().has_value()
                       &&
                       reverseStepAtArrival.getTransferTravelTime() >= 0
                       &&
                       reverseStepAtArrival.getTransferTravelTime() < currentTripQueryOverlay.exitConnectionTransferTravelTime
                       )
              {
                journeyConnectionMinWaitingTimeSeconds = reverseStepAtArrival.getFinalEnterConnection().value().get().getMinWaitingTimeOrDefault(parameters.getMinWaitingTimeSeconds());

                if (connectionArrivalTime + journeyConnectionMinWaitingTimeSeconds <= nodeArrivalTentativeTime)
                {
                  currentTripQueryOverlay.exitConnection = *connection;
                  currentTripQueryOverlay.exitConnectionTransferTravelTime = reverseStepAtArrival.getTransferTravelTime();
                }
              }

            }

            if ((*connection).get().canBoard() && currentTripQueryOverlay.exitConnection.has_value())
            {
              // get footpaths for the arrival node to get transferable nodes:
              const Node &nodeDeparture = (*connection).get().getDepartureNode();
              connectionDepartureTime         = (*connection).get().getDepartureTime();
              connectionMinWaitingTimeSeconds = (*connection).get().getMinWaitingTimeOrDefault(parameters.getMinWaitingTimeSeconds());

              auto nodeDepartureInNodesAccessIte = nodesAccess.find(nodeDeparture.uid);
              for (const NodeTimeDistance & transferableNode : nodeDeparture.reverseTransferableNodes)
              {

                if (nodeDeparture != transferableNode.node && nodesReverseTentativeTime.at(transferableNode.node.uid) > connectionDepartureTime - connectionMinWaitingTimeSeconds)
                {
                  continue;
                }

                //TODO We should not do a direct == with float values
                footpathTravelTime = parameters.getWalkingSpeedFactor() == 1.0 ? transferableNode.time : (int)ceil((float)transferableNode.time / parameters.getWalkingSpeedFactor());

                if (footpathTravelTime <= parameters.getMaxTransferWalkingTravelTimeSeconds())
                {
                  if (connectionDepartureTime - footpathTravelTime - connectionMinWaitingTimeSeconds >= nodesReverseTentativeTime.at(transferableNode.node.uid))
                  {
                    footpathDistance = transferableNode.distance;
                    nodesReverseTentativeTime[transferableNode.node.uid] = connectionDepartureTime - footpathTravelTime - connectionMinWaitingTimeSeconds;
                    //TODO Do we need a make_optional<...>(connection) ??
                    reverseJourneysSteps.at(transferableNode.node.uid) = JourneyStep(*connection, currentTripQueryOverlay.exitConnection, std::cref(trip), footpathTravelTime, (nodeDeparture == transferableNode.node), footpathDistance);
                  }
                  if (
                    nodeDeparture == transferableNode.node
                    &&
                    (
                     //TODO Really not sure this is equivalent
                     reverseAccessJourneysSteps.count(transferableNode.node.uid) == 0
                     ||
                     reverseAccessJourneysSteps.at(transferableNode.node.uid).getFinalEnterConnection().value().get().getDepartureTime() <= connectionDepartureTime - connectionMinWaitingTimeSeconds
                    )
                  )
                  {
                    if (
                      departureTimeSeconds == -1
                      ||
                      (nodeDepartureInNodesAccessIte != nodesAccess.end() &&
                       connectionDepartureTime - nodeDepartureInNodesAccessIte->second.time - connectionMinWaitingTimeSeconds >= departureTimeSeconds
                       ))
                    {
                      if (
                        departureTimeSeconds == -1
                        ||
                        parameters.getMaxFirstWaitingTimeSeconds() < connectionMinWaitingTimeSeconds
                        ||
                        connectionDepartureTime - departureTimeSeconds - nodeDepartureInNodesAccessIte->second.time <= parameters.getMaxFirstWaitingTimeSeconds()
                      )
                      {
                        reverseAccessJourneysSteps.insert_or_assign(transferableNode.node.uid, JourneyStep(*connection, currentTripQueryOverlay.exitConnection, std::cref(trip), 0, true, 0));
                      }
                    }
                  }
                }
              }
            }
            reachableConnectionsCount++;
          }
        }
      }
    }

    spdlog::debug("-- {}  reverse connections parsed on {}", reachableConnectionsCount, connectionsCount);

    if (reachableConnectionsCount == 0) {
      throw NoRoutingFoundException(NoRoutingReason::NO_SERVICE_TO_DESTINATION);
    }
  }
}
