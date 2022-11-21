
#include "spdlog/spdlog.h"
#include "calculator.hpp"
#include "constants.hpp"
#include "parameters.hpp"
#include "node.hpp"
#include "trip.hpp"
#include "line.hpp"
#include "mode.hpp"
#include "path.hpp"
#include "agency.hpp"
#include "routing_result.hpp"
#include "toolbox.hpp" //MAX_INT
#include "transit_data.hpp"

namespace TrRouting
{

  std::unique_ptr<RoutingResult> Calculator::forwardJourneyStep(RouteParameters &parameters, int bestArrivalTime, std::optional<std::reference_wrapper<const Node>> bestEgressNode, const std::unordered_map<Node::uid_t, JourneyStep> & forwardEgressJourneysSteps)
  {

    // FIXME: One code path supports both Single calculation and all nodes, we shouldn't have to deal with both of these types here
    std::unique_ptr<SingleCalculationResult> singleResult = std::make_unique<SingleCalculationResult>();
    std::unique_ptr<AllNodesResult> allNodesResult = std::make_unique<AllNodesResult>();

    int              nodesCount           {1};
    int              reachableNodesCount  {0};
    bool             foundLine            {false};

    std::vector<std::reference_wrapper<const Node>> resultingNodes;
    if (params.returnAllNodesResult)
    {
      nodesCount     = transitData.getNodes().size();
      // Add references to all nodes in the resultingNodes vector
      // TODO this seems heavy. To reconsidered when we split function and consider the
      // allNodes case separately 
      for(auto nodeIte = transitData.getNodes().begin(); nodeIte != transitData.getNodes().end(); nodeIte++) {
        resultingNodes.push_back(std::cref(nodeIte->second));
      }
    }
    else if (bestArrivalTime < MAX_INT) // line found
    {
      foundLine = true;
      resultingNodes.push_back(bestEgressNode.value());
    } else {
      throw NoRoutingFoundException(NoRoutingReason::NO_ROUTING_FOUND);
    }

    //TODO This if seems unnecessary, we throw before if this condition would be false
    if (foundLine || params.returnAllNodesResult)
    {
      std::deque<JourneyStep> journey;

      std::shared_ptr<Connection> journeyStepEnterConnection;
      std::shared_ptr<Connection> journeyStepExitConnection;
      std::vector<boost::uuids::uuid>                   unboardingNodeUuids;
      std::vector<boost::uuids::uuid>                   boardingNodeUuids;
      std::vector<int>                                  inVehicleTravelTimesSeconds; // the in vehicle travel time for each segment
      std::vector<Leg>  legs;

      int totalInVehicleTime       { 0}; int transferArrivalTime    {-1}; int firstDepartureTime     {-1};
      int totalWalkingTime         { 0}; int transferReadyTime      {-1}; int minimizedDepartureTime {-1};
      int totalWaitingTime         { 0}; int departureTime          {-1}; int numberOfTransfers      {-1};
      int totalTransferWalkingTime { 0}; int arrivalTime            {-1}; int boardingSequence       {-1};
      int totalTransferWaitingTime { 0}; int inVehicleTime          {-1};
      int totalDistance            { 0}; int distance               {-1};
      int inVehicleDistance        {-1}; int totalInVehicleDistance { 0}; int totalWalkingDistance   { 0};
      int totalTransferDistance    {-1}; int accessDistance         { 0}; int egressDistance         { 0};
      int accessWalkingTime      {-1};
      int transferTime             {-1}; int egressWalkingTime      {-1};
      int waitingTime              {-1}; int accessWaitingTime      {-1};

      for (const Node & resultingNode : resultingNodes)
      {

        legs.clear();
        journey.clear();

        boardingNodeUuids.clear();
        unboardingNodeUuids.clear();
        inVehicleTravelTimesSeconds.clear();

        totalInVehicleTime       =  0; transferArrivalTime    = -1; firstDepartureTime     = -1;
        totalWalkingTime         =  0; transferReadyTime      = -1; minimizedDepartureTime = -1;
        totalWaitingTime         =  0; departureTime          = -1; numberOfTransfers      = -1;
        totalTransferWalkingTime =  0; arrivalTime            = -1; boardingSequence       = -1;
        totalTransferWaitingTime =  0; inVehicleTime          = -1;
        totalDistance            =  0; distance               = -1;
        inVehicleDistance        =  0; totalInVehicleDistance =  0; totalWalkingDistance   =  0;
        totalTransferDistance    =  0; accessDistance         =  0; egressDistance         =  0;
        accessWalkingTime      = -1;
        transferTime             = -1; egressWalkingTime      = -1;
        waitingTime              = -1; accessWaitingTime      = -1;

        // recreate journey:
        if (forwardEgressJourneysSteps.count(resultingNode.uid) == 0) { // ignore nodes with no line
          continue;
        }
        JourneyStep resultingNodeJourneyStep = forwardEgressJourneysSteps.at(resultingNode.uid);

        std::optional<std::reference_wrapper<const Node>> bestAccessNode;
        while (resultingNodeJourneyStep.hasConnections())
        {
          journey.push_front(resultingNodeJourneyStep);
          bestAccessNode = resultingNodeJourneyStep.getFinalEnterConnection().value()->getDepartureNode();
          resultingNodeJourneyStep = forwardJourneysSteps.at(bestAccessNode.value().get().uid);
        }

        if (!params.returnAllNodesResult)
        {
          journey.push_back(JourneyStep(std::nullopt,
                                        std::nullopt,
                                        std::nullopt,
                                        nodesEgress.at(resultingNode.uid).distance,
                                        false,
                                        nodesEgress.at(resultingNode.uid).distance));
        }
        journey.push_front(JourneyStep(std::nullopt,
                                       std::nullopt,
                                       std::nullopt,
                                       nodesAccess.at(bestAccessNode.value().get().uid).time,
                                       false,
                                       nodesAccess.at(bestAccessNode.value().get().uid).distance));

        //std::string stepsJson = "  \"steps\":\n  [\n";

        size_t i = 0;
        size_t journeyStepsCount = journey.size();
        for (auto & journeyStep : journey)
        {

          if (journeyStep.hasConnections())
          {
            // journey tuple: final enter connection, final exit connection, final footpath
            journeyStepEnterConnection = journeyStep.getFinalEnterConnection().value();
            journeyStepExitConnection  = journeyStep.getFinalExitConnection().value();
            const Node &journeyStepNodeDeparture   = journeyStepEnterConnection->getDepartureNode();
            const Node &journeyStepNodeArrival     = journeyStepExitConnection->getArrivalNode();
            // Calling value() direct as we assume if we got here, we have a valid journeyStep
            const Trip &journeyStepTrip            = journeyStep.getFinalTrip().value().get();
            transferTime               = journeyStep.getTransferTravelTime();
            distance                   = journeyStep.getTransferDistance();
            inVehicleDistance          = 0;
            departureTime              = journeyStepEnterConnection->getDepartureTime();
            arrivalTime                = journeyStepExitConnection->getArrivalTime();
            boardingSequence           = journeyStepEnterConnection->getSequenceInTrip();
            int unboardingSequence     = journeyStepExitConnection->getSequenceInTrip();
            inVehicleTime              = arrivalTime   - departureTime;
            waitingTime                = departureTime - transferArrivalTime;
            transferArrivalTime        = arrivalTime   + transferTime;
            transferReadyTime          = transferArrivalTime;

            if (journey.size() > i + 1 && journey[i+1].getFinalEnterConnection().has_value())
            {
              transferReadyTime += journey[i+1].getFinalEnterConnection().value()->getMinWaitingTimeOrDefault(parameters.getMinWaitingTimeSeconds());
            }

            totalInVehicleTime         += inVehicleTime;
            totalWaitingTime           += waitingTime;
            if (Mode::TRANSFERABLE != journeyStepTrip.line.mode.shortname)
            {
              numberOfTransfers += 1;
            }
            inVehicleTravelTimesSeconds.push_back(inVehicleTime);
            boardingNodeUuids.push_back(journeyStepNodeDeparture.uuid);
            unboardingNodeUuids.push_back(journeyStepNodeArrival.uuid);
            legs.push_back(std::make_tuple(std::ref(journeyStepTrip), std::ref(journeyStepTrip.line), std::ref(journeyStepTrip.path), boardingSequence - 1, unboardingSequence - 1));

            if (unboardingSequence - 1 < journeyStepTrip.path.segmentsDistanceMeters.size()) // check if distances are available for this path
            {
              for (int seqI = boardingSequence - 1; seqI < unboardingSequence; seqI++)
              {
                inVehicleDistance += journeyStepTrip.path.segmentsDistanceMeters[seqI];
              }
              totalDistance += inVehicleDistance;
              if (Mode::TRANSFERABLE == journeyStepTrip.line.mode.shortname)
              {
                totalWalkingDistance     += inVehicleDistance;
                totalWalkingTime         += inVehicleTime;
                totalTransferDistance    += inVehicleDistance;
                totalTransferWalkingTime += inVehicleTime;
              }
              else
              {
                totalInVehicleDistance += inVehicleDistance;
              }
            }
            else
            {
              inVehicleDistance      = -1;
              totalDistance          = -1;
              totalInVehicleDistance = -1;
            }

            // TODO confirm that i == 1 is the right index to compare to
            if (i == 1) // first leg
            {
              accessWaitingTime      = waitingTime;
              firstDepartureTime     = departureTime;
              minimizedDepartureTime = firstDepartureTime - accessWalkingTime - journey[1].getFinalEnterConnection().value()->getMinWaitingTimeOrDefault(parameters.getMinWaitingTimeSeconds());
            }
            else
            {
              totalTransferWaitingTime += waitingTime;
            }

            if (!params.returnAllNodesResult)
            {
              singleResult.get()->steps.push_back(std::make_unique<BoardingStep>(
                journeyStepTrip.agency.uuid, //TODO change boardingstep constructor to take the agency object directly
                journeyStepTrip.agency.acronym,
                journeyStepTrip.agency.name,
                journeyStepTrip.line.uuid,
                journeyStepTrip.line.shortname,
                journeyStepTrip.line.longname,
                journeyStepTrip.path.uuid,
                journeyStepTrip.line.mode.name,
                journeyStepTrip.line.mode.shortname,
                journeyStepTrip.uuid,
                boardingSequence,
                boardingSequence,
                journeyStepNodeDeparture.uuid,
                journeyStepNodeDeparture.code,
                journeyStepNodeDeparture.name,
                *journeyStepNodeDeparture.point.get(),
                minimizedDepartureTime,
                waitingTime
              ));

              singleResult.get()->steps.push_back(std::make_unique<UnboardingStep>(
                journeyStepTrip.agency.uuid, //TODO change boardingstep constructor to take the agency object directly
                journeyStepTrip.agency.acronym,
                journeyStepTrip.agency.name,
                journeyStepTrip.line.uuid,
                journeyStepTrip.line.shortname,
                journeyStepTrip.line.longname,
                journeyStepTrip.path.uuid,
                journeyStepTrip.line.mode.name,
                journeyStepTrip.line.mode.shortname,
                journeyStepTrip.uuid,
                unboardingSequence,
                unboardingSequence + 1,
                journeyStepNodeArrival.uuid,
                journeyStepNodeArrival.code,
                journeyStepNodeArrival.name,
                *journeyStepNodeArrival.point.get(),
                arrivalTime,
                inVehicleTime,
                inVehicleDistance
              ));
            }
            if (i < journeyStepsCount - 2) // if not the last transit leg
            {
              totalTransferWalkingTime += transferTime;
              totalWalkingTime         += transferTime;
              if (totalDistance != -1)
              {
                totalDistance += distance;
              }
              totalWalkingDistance     += distance;
              totalTransferDistance    += distance;
              if (!params.returnAllNodesResult)
              {
                singleResult.get()->steps.push_back(std::make_unique<WalkingStep>(
                  walking_step_type::TRANSFER,
                  transferTime,
                  distance,
                  arrivalTime,
                  transferArrivalTime,
                  transferReadyTime
                ));
              }
            }
          }
          else // access or egress journey step
          {

            transferTime          = journeyStep.getTransferTravelTime();
            distance              = journeyStep.getTransferDistance();
            if (totalDistance != -1)
            {
              totalDistance += distance;
            }
            totalWalkingDistance += distance;
            if (i == 0) // access
            {
              transferArrivalTime  = departureTimeSeconds + transferTime;
              transferReadyTime    = transferArrivalTime;

              if (journey.size() > i + 1 && journey[i+1].getFinalEnterConnection().has_value())
              {
                transferReadyTime += journey[i+1].getFinalEnterConnection().value()->getMinWaitingTimeOrDefault(parameters.getMinWaitingTimeSeconds());
              }

              totalWalkingTime    += transferTime;
              accessWalkingTime    = transferTime;
              accessDistance       = distance;
              if (!params.returnAllNodesResult)
              {
                singleResult.get()->steps.push_back(std::make_unique<WalkingStep>(
                  walking_step_type::ACCESS,
                  transferTime,
                  distance,
                  departureTimeSeconds,
                  transferArrivalTime,
                  transferReadyTime
                ));
              }
            }
            else // egress
            {
              totalWalkingTime   += transferTime;
              egressWalkingTime   = transferTime;
              transferArrivalTime = arrivalTime + transferTime;
              egressDistance      = distance;
              if (!params.returnAllNodesResult)
              {
                singleResult.get()->steps.push_back(std::make_unique<WalkingStep>(
                  walking_step_type::EGRESS,
                  transferTime,
                  distance,
                  arrivalTime,
                  arrivalTime + transferTime
                ));
              }
              arrivalTime = transferArrivalTime;
            }
          }
          i++;
        }

        if (params.returnAllNodesResult)
        {
          if (forwardEgressJourneysSteps.at(resultingNode.uid).getFinalEnterConnection().has_value())
          {
            arrivalTime = forwardEgressJourneysSteps.at(resultingNode.uid).getFinalExitConnection().value()->getArrivalTime();
            if (arrivalTime - departureTimeSeconds <= parameters.getMaxTotalTravelTimeSeconds())
            {
              reachableNodesCount++;

              AccessibleNodes node = AccessibleNodes(
                resultingNode.uuid,
                arrivalTime,
                arrivalTime - departureTimeSeconds,
                numberOfTransfers
              );
              allNodesResult.get()->nodes.push_back(node);
            }
          }
        }
        else
        {
          singleResult.get()->departureTime = minimizedDepartureTime;
          singleResult.get()->arrivalTime = arrivalTime;
          singleResult.get()->totalTravelTime = arrivalTime - minimizedDepartureTime;
          singleResult.get()->totalDistance = totalDistance;
          singleResult.get()->totalInVehicleTime = totalInVehicleTime;
          singleResult.get()->totalInVehicleDistance = totalInVehicleDistance;
          singleResult.get()->totalNonTransitTravelTime = totalWalkingTime;
          singleResult.get()->totalNonTransitDistance = totalWalkingDistance;
          singleResult.get()->numberOfBoardings = numberOfTransfers + 1;
          singleResult.get()->numberOfTransfers = numberOfTransfers == -1 ? 0 : numberOfTransfers;
          singleResult.get()->transferWalkingTime = totalTransferWalkingTime;
          singleResult.get()->transferWalkingDistance = totalTransferDistance;
          singleResult.get()->accessTravelTime = accessWalkingTime;
          singleResult.get()->accessDistance = accessDistance;
          singleResult.get()->egressTravelTime = egressWalkingTime;
          singleResult.get()->egressDistance = egressDistance;
          singleResult.get()->transferWaitingTime = totalTransferWaitingTime;
          singleResult.get()->firstWaitingTime = accessWaitingTime;
          singleResult.get()->totalWaitingTime = totalWaitingTime;
          singleResult.get()->legs = legs;
        }
      }
    }

    spdlog::debug("-- forward result: {}", (params.returnAllNodesResult ? " allNodes " : "single calculation"));

    if (params.returnAllNodesResult)
    {
      allNodesResult.get()->numberOfReachableNodes = reachableNodesCount;
      // Get a number with 2 decimals. FIXME: Let the formatting be done at another level?
      allNodesResult.get()->percentOfReachableNodes = round(10000 * (float)reachableNodesCount / (float)(nodesCount))/100.0;
      return allNodesResult;
    }

    return singleResult;

  }

}
