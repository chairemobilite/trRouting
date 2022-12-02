#include <boost/uuid/string_generator.hpp>
#include "spdlog/spdlog.h"

#include "calculator.hpp"
#include "toolbox.hpp"
#include "parameters.hpp"
#include "node.hpp"
#include "routing_result.hpp"
#include "transit_data.hpp"

namespace TrRouting
{
    
  std::optional<std::tuple<int, std::reference_wrapper<const Node>>> Calculator::reverseCalculation(RouteParameters &parameters,
                                                                                                    std::unordered_map<Node::uid_t, JourneyStep> & reverseAccessJourneysSteps)
  {
    assert(!params.returnAllNodesResult); // Just make sure we are in the right code path

    int benchmarkingStart = algorithmCalculationTime.getEpoch();

    int  reachableConnectionsCount        {0};
    std::optional<std::shared_ptr<Connection>> tripExitConnection;
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
    auto & reverseConnections = transitData.getReverseConnections();
    
    int  connectionsCount = reverseConnections.size();
    int  arrivalTimeHour  = arrivalTimeSeconds / 3600;

    // reverse calculation:

    // main loop for reverse connections:
    auto lastConnection = reverseConnections.end();
    for(auto connection = transitData.getReverseConnectionsBeginAtArrivalHour(arrivalTimeHour + 1); connection != lastConnection; ++connection)
    {
      // ignore connections after arrival time - minimum egress travel time:
      if ((**connection).getArrivalTime() <= arrivalTimeSeconds - minEgressTravelTime)
      {
        
        const Trip & trip = (**connection).getTrip();
        
        // enabled trips only here:
        auto & currentTripQueryOverlay = tripsQueryOverlay[trip.uid];
        if (currentTripQueryOverlay.usable && tripsEnabled[trip.uid])
        {

          connectionArrivalTime           = (**connection).getArrivalTime();

          // no need to parse next connections if already reached destination from all egress nodes, except if max travel time is set, so we can get a reverse profile in the next loop calculation:
          // yes, we mean connectionArrivalTime and not connectionDepartureTime because travel time for each connections, otherwise you can catch a very short/long connection
          if ( (reachedAtLeastOneAccessNode && maxAccessTravelTime >= 0 && connectionArrivalTime /* ! not connectionDepartureTime */ < tentativeAccessNodeDepartureTime - maxAccessTravelTime) || (arrivalTimeSeconds - connectionArrivalTime > parameters.getMaxTotalTravelTimeSeconds()))
          {
            break;
          }

          tripExitConnection   = currentTripQueryOverlay.exitConnection;
          const Node &nodeArrival = (**connection).getArrivalNode();

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
              tripExitConnection.has_value()
            ||
            nodeArrivalTentativeTime >= connectionArrivalTime
          )
          {
            
            if ((**connection).canUnboard())
            {
              // TODO probably extract the reverseJourneysSteps[nodeArrival.uuid] instead of doing lookup multiple the time
              if (!tripExitConnection.has_value()) // <= to make sure we get the same result as forward calculation, which uses >
              {
                currentTripQueryOverlay.exitConnection = *connection;
                currentTripQueryOverlay.exitConnectionTransferTravelTime = reverseJourneysSteps.at(nodeArrival.uid).getTransferTravelTime();
              }
              else if (
                       //TODO This was commented out in forward_calculation
                       reverseJourneysSteps.at(nodeArrival.uid).getFinalEnterConnection().has_value()
                       &&
                       reverseJourneysSteps.at(nodeArrival.uid).getTransferTravelTime() >= 0
                       && 
                       reverseJourneysSteps.at(nodeArrival.uid).getTransferTravelTime() < currentTripQueryOverlay.exitConnectionTransferTravelTime
                       )
              {
                journeyConnectionMinWaitingTimeSeconds = reverseJourneysSteps.at(nodeArrival.uid).getFinalEnterConnection().value()->getMinWaitingTimeOrDefault(parameters.getMinWaitingTimeSeconds());

                if (connectionArrivalTime + journeyConnectionMinWaitingTimeSeconds <= nodeArrivalTentativeTime)
                {
                  currentTripQueryOverlay.exitConnection = *connection;
                  currentTripQueryOverlay.exitConnectionTransferTravelTime = reverseJourneysSteps.at(nodeArrival.uid).getTransferTravelTime();
                }
              }

            }
            
            if ((**connection).canBoard() && currentTripQueryOverlay.exitConnection.has_value())
            {
              // get footpaths for the arrival node to get transferable nodes:
              const Node &nodeDeparture = (**connection).getDepartureNode();
              connectionDepartureTime         = (**connection).getDepartureTime();
              connectionMinWaitingTimeSeconds = (**connection).getMinWaitingTimeOrDefault(parameters.getMinWaitingTimeSeconds());

              auto nodeDepartureInNodesAccessIte = nodesAccess.find(nodeDeparture.uid);
              if (!reachedAtLeastOneAccessNode &&  nodeDepartureInNodesAccessIte != nodesAccess.end() &&  nodeDepartureInNodesAccessIte->second.time != -1) // check if the departure node is accessable
              {
                reachedAtLeastOneAccessNode      = true;
                tentativeAccessNodeDepartureTime = connectionDepartureTime;

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
                    //TODO Do we need a make_optional<...>(connection) ??
                    reverseJourneysSteps.insert_or_assign(transferableNode.node.uid, JourneyStep(*connection, currentTripQueryOverlay.exitConnection, std::cref(trip), footpathTravelTime, (nodeDeparture.uuid == transferableNode.node.uuid), footpathDistance));
                  }
                  if (
                    nodeDeparture.uuid == transferableNode.node.uuid
                    && 
                    (
                     //TODO Really not sure this is equivalent
                     reverseAccessJourneysSteps.count(transferableNode.node.uid) == 0
                     || 
                     reverseAccessJourneysSteps.at(transferableNode.node.uid).getFinalEnterConnection().value()->getDepartureTime() <= connectionDepartureTime - connectionMinWaitingTimeSeconds
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
                footpathIndex++;
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
        std::optional<std::shared_ptr<Connection>> accessEnterConnection  = reverseAccessJourneysSteps.at(accessFootpath.node.uid).getFinalEnterConnection();
        if (accessEnterConnection.has_value()) //TODO is this check required with the previous if(count()) added ?
        {
          const NodeTimeDistance & access = nodesAccess.at(accessFootpath.node.uid);
          int accessEnterConnectionMinWaitingTimeSeconds = accessEnterConnection.value()->getMinWaitingTimeOrDefault(parameters.getMinWaitingTimeSeconds());

          int accessNodeDepartureTime                    = accessEnterConnection.value()->getDepartureTime() - access.time - accessEnterConnectionMinWaitingTimeSeconds;
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


  void Calculator::reverseCalculationAllNodes(RouteParameters &parameters,
                                              std::unordered_map<Node::uid_t, JourneyStep> & reverseAccessJourneysSteps)
  {
    assert(params.returnAllNodesResult); // Just make sure we are in the right code path

    int benchmarkingStart = algorithmCalculationTime.getEpoch();

    int  reachableConnectionsCount        {0};
    std::optional<std::shared_ptr<Connection>> tripExitConnection;
    int  connectionDepartureTime          {-1};
    int  connectionArrivalTime            {-1};
    short connectionMinWaitingTimeSeconds {-1};
    short journeyConnectionMinWaitingTimeSeconds {-1};
    //long long  footpathsRangeStart        {-1};
    //long long  footpathsRangeEnd          {-1};
    int  footpathIndex                    {-1};
    int  footpathTravelTime               {-1};
    int  footpathDistance                 {-1};

    //TODO could be passed as a parameter
    auto & reverseConnections = transitData.getReverseConnections();

    int  connectionsCount = reverseConnections.size();
    int  arrivalTimeHour  = arrivalTimeSeconds / 3600;

    // reverse calculation:

    // main loop for reverse connections:
    auto lastConnection = reverseConnections.end();
    for(auto connection = transitData.getReverseConnectionsBeginAtArrivalHour(arrivalTimeHour + 1); connection != lastConnection; ++connection)
    {
      // ignore connections after arrival time - minimum egress travel time:
      if ((**connection).getArrivalTime() <= arrivalTimeSeconds)
      {
        const Trip & trip = (**connection).getTrip();

        // enabled trips only here:
        auto & currentTripQueryOverlay = tripsQueryOverlay[trip.uid];
        if (currentTripQueryOverlay.usable && tripsEnabled[trip.uid])
        {
          connectionArrivalTime           = (**connection).getArrivalTime();

          // no need to parse next connections if already reached destination from all egress nodes, except if max travel time is set, so we can get a reverse profile in the next loop calculation:
          // yes, we mean connectionArrivalTime and not connectionDepartureTime because travel time for each connections, otherwise you can catch a very short/long connection
          if ( arrivalTimeSeconds - connectionArrivalTime > parameters.getMaxTotalTravelTimeSeconds())
          {
            break;
          }

          tripExitConnection   = currentTripQueryOverlay.exitConnection;
          const Node &nodeArrival = (**connection).getArrivalNode();

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
              tripExitConnection.has_value()
            ||
            nodeArrivalTentativeTime >= connectionArrivalTime
          )
          {
            if ((**connection).canUnboard())
            {
              // TODO probably extract the reverseJourneysSteps[nodeArrival.uuid] instead of doing lookup multiple the time
              if (!tripExitConnection.has_value()) // <= to make sure we get the same result as forward calculation, which uses >
              {
                currentTripQueryOverlay.exitConnection = *connection;
                currentTripQueryOverlay.exitConnectionTransferTravelTime = reverseJourneysSteps.at(nodeArrival.uid).getTransferTravelTime();
              }
              else if (
                       //TODO This was commented out in forward_calculation
                       reverseJourneysSteps.at(nodeArrival.uid).getFinalEnterConnection().has_value()
                       &&
                       reverseJourneysSteps.at(nodeArrival.uid).getTransferTravelTime() >= 0
                       &&
                       reverseJourneysSteps.at(nodeArrival.uid).getTransferTravelTime() < currentTripQueryOverlay.exitConnectionTransferTravelTime
                       )
              {
                journeyConnectionMinWaitingTimeSeconds = reverseJourneysSteps.at(nodeArrival.uid).getFinalEnterConnection().value()->getMinWaitingTimeOrDefault(parameters.getMinWaitingTimeSeconds());

                if (connectionArrivalTime + journeyConnectionMinWaitingTimeSeconds <= nodeArrivalTentativeTime)
                {
                  currentTripQueryOverlay.exitConnection = *connection;
                  currentTripQueryOverlay.exitConnectionTransferTravelTime = reverseJourneysSteps.at(nodeArrival.uid).getTransferTravelTime();
                }
              }

            }

            if ((**connection).canBoard() && currentTripQueryOverlay.exitConnection.has_value())
            {
              // get footpaths for the arrival node to get transferable nodes:
              const Node &nodeDeparture = (**connection).getDepartureNode();
              connectionDepartureTime         = (**connection).getDepartureTime();
              connectionMinWaitingTimeSeconds = (**connection).getMinWaitingTimeOrDefault(parameters.getMinWaitingTimeSeconds());

              auto nodeDepartureInNodesAccessIte = nodesAccess.find(nodeDeparture.uid);
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
                    //TODO Do we need a make_optional<...>(connection) ??
                    reverseJourneysSteps.insert_or_assign(transferableNode.node.uid, JourneyStep(*connection, currentTripQueryOverlay.exitConnection, std::cref(trip), footpathTravelTime, (nodeDeparture.uuid == transferableNode.node.uuid), footpathDistance));
                  }
                  if (
                    nodeDeparture.uuid == transferableNode.node.uuid
                    &&
                    (
                     //TODO Really not sure this is equivalent
                     reverseAccessJourneysSteps.count(transferableNode.node.uid) == 0
                     ||
                     reverseAccessJourneysSteps.at(transferableNode.node.uid).getFinalEnterConnection().value()->getDepartureTime() <= connectionDepartureTime - connectionMinWaitingTimeSeconds
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
                footpathIndex++;
              }
            }
            reachableConnectionsCount++;
          }
        }
      }
    }

    spdlog::debug("-- {}  reverse connections parsed on {}", reachableConnectionsCount, connectionsCount);

    benchmarking["reverse_calculation"] += algorithmCalculationTime.getEpoch() - benchmarkingStart;
  }
}
