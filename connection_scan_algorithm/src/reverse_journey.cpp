#include "calculator.hpp"
#include "constants.hpp"
#include "parameters.hpp"
#include "node.hpp"
#include "trip.hpp"
#include "line.hpp"
#include "mode.hpp"
#include "path.hpp"
#include "routing_result.hpp"
#include "agency.hpp"
#include "transit_data.hpp"
#include "spdlog/spdlog.h"

namespace TrRouting
{

  std::unique_ptr<RoutingResult> Calculator::reverseJourneyStep(RouteParameters &parameters, int bestDepartureTime, std::optional<std::reference_wrapper<const Node>> bestAccessNode, const std::unordered_map<Node::uid_t, JourneyStep> & reverseAccessJourneysSteps)
  {

    // FIXME: One code path supports both Single calculation and all nodes, we shouldn't have to deal with both of these types here
    std::unique_ptr<SingleCalculationResult> singleResult = std::make_unique<SingleCalculationResult>();
    std::unique_ptr<AllNodesResult> allNodesResult = std::make_unique<AllNodesResult>();

    int              nodesCount           {1};
    int              reachableNodesCount  {0};
    bool             foundRoute           {false};

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
    else if (bestDepartureTime > -1) // route found
    {
      foundRoute = true;
      resultingNodes.push_back(bestAccessNode.value());
    } else {
      throw NoRoutingFoundException(NoRoutingReason::NO_ROUTING_FOUND);
    }

    if (foundRoute || params.returnAllNodesResult)
    {
      std::shared_ptr<Connection> journeyStepEnterConnection;
      std::shared_ptr<Connection> journeyStepExitConnection;
      std::vector<boost::uuids::uuid>                   unboardingNodeUuids;
      std::vector<boost::uuids::uuid>                   boardingNodeUuids;
      std::vector<int>                                  inVehicleTravelTimesSeconds; // the in vehicle travel time for each segment
      std::vector<Leg> legs;

      int totalInVehicleTime       { 0}; int transferArrivalTime    {-1};
      int totalWalkingTime         { 0}; int transferReadyTime      {-1}; int numberOfTransfers    {-1};
      int totalWaitingTime         { 0}; int departureTime          {-1}; int boardingSequence     {-1};
      int totalTransferWalkingTime { 0}; int arrivalTime            {-1}; int unboardingSequence   {-1};
      int totalTransferWaitingTime { 0}; int inVehicleTime          {-1};
      int totalDistance            { 0}; int distance               {-1};
      int inVehicleDistance        {-1}; int totalInVehicleDistance { 0}; int totalWalkingDistance { 0};
      int totalTransferDistance    {-1}; int accessDistance         { 0}; int egressDistance       { 0};
      int accessWalkingTime      {-1};
      int transferTime             {-1}; int egressWalkingTime      {-1};
      int waitingTime              {-1}; int accessWaitingTime      {-1};

      for (const Node & resultingNode : resultingNodes)
      {
        legs.clear();
        std::deque<JourneyStep> journey;
        boardingNodeUuids.clear();
        unboardingNodeUuids.clear();
        inVehicleTravelTimesSeconds.clear();

        totalInVehicleTime          =  0; transferArrivalTime    = -1;
        totalWalkingTime            =  0; transferReadyTime      = -1; numberOfTransfers    = -1;
        totalWaitingTime            =  0; departureTime          = -1; boardingSequence     = -1;
        totalTransferWalkingTime    =  0; arrivalTime            = -1; unboardingSequence   = -1;
        totalTransferWaitingTime    =  0; inVehicleTime          = -1;
        totalDistance               =  0; distance               = -1;
        inVehicleDistance           =  0; totalInVehicleDistance =  0; totalWalkingDistance =  0;
        totalTransferDistance       =  0; accessDistance         =  0; egressDistance       =  0;
        accessWalkingTime      = -1;
        transferTime                = -1; egressWalkingTime      = -1;
        waitingTime                 = -1; accessWaitingTime      = -1;

        // recreate journey:
        if (reverseAccessJourneysSteps.count(resultingNode.uid) == 0) { // ignore nodes with no line
          continue;
        }
        JourneyStep resultingNodeJourneyStep = reverseAccessJourneysSteps.at(resultingNode.uid);

        std::optional<std::reference_wrapper<const Node>> bestEgressNode;
        while (resultingNodeJourneyStep.hasConnections())
        {
          // here we inverse the transfers (putting them after the exit connection, instead of before the enter connection):
          if (journey.size() > 0)
          {
            journey[journey.size()-1].copyTransferTimeDistance(resultingNodeJourneyStep);
          }
          journey.push_back(resultingNodeJourneyStep);
          bestEgressNode = resultingNodeJourneyStep.getFinalExitConnection().value()->getArrivalNode();
          resultingNodeJourneyStep = reverseJourneysSteps.at(bestEgressNode.value().get().uid);
        }

        if (!params.returnAllNodesResult)
        {
          journey.push_front(JourneyStep(std::nullopt,
                                         std::nullopt,
                                         std::nullopt,
                                         nodesAccess.at(resultingNode.uid).time,
                                         false,
                                         nodesAccess.at(resultingNode.uid).distance));
        }
        journey.push_back(JourneyStep(std::nullopt,
                                      std::nullopt,
                                      std::nullopt,
                                      nodesEgress.at(bestEgressNode.value().get().uid).time,
                                      false,
                                      nodesEgress.at(bestEgressNode.value().get().uid).distance));


        //TODO This optimizeCaseStr string does nothing, should be removed
        std::string optimizeCasesStr = "";
        std::vector<int> optimizeCases = optimizeJourney(journey);

        for (int optimizeCase : optimizeCases)
        {
          if (optimizeCase == 1)
          {
            optimizeCasesStr += "CSL|";
          }
          else if (optimizeCase == 2)
          {
            optimizeCasesStr += "BTS|";
          }
          else if (optimizeCase == 3)
          {
            optimizeCasesStr += "GTF|";
          }
          else if (optimizeCase == 4)
          {
            optimizeCasesStr += "CSS|";
          }
        }

        if (optimizeCasesStr.size() > 0)
        {
          optimizeCasesStr.pop_back();
        }
        spdlog::debug("-- {} optimization case used {} ", optimizeCases.size(), optimizeCasesStr);
        size_t journeyStepsCount = journey.size();
        size_t i = 0;
        for (auto & journeyStep : journey)
        {

          // check if it is an in-vehicle journey:
          if (journeyStep.hasConnections())
          {
            // journey tuple: final enter connection, final exit connection, final footpath
            journeyStepEnterConnection  = journeyStep.getFinalEnterConnection().value();
            journeyStepExitConnection   = journeyStep.getFinalExitConnection().value();
            const Node &journeyStepNodeDeparture    = journeyStepEnterConnection->getDepartureNode();
            const Node &journeyStepNodeArrival      = journeyStepExitConnection->getArrivalNode();
            const Trip &journeyStepTrip             = journeyStep.getFinalTrip().value().get();
            transferTime                = journeyStep.getTransferTravelTime();
            distance                    = journeyStep.getTransferDistance();
            inVehicleDistance           = 0;
            departureTime               = journeyStepEnterConnection->getDepartureTime();
            arrivalTime                 = journeyStepExitConnection->getArrivalTime();
            boardingSequence            = journeyStepEnterConnection->getSequenceInTrip();
            unboardingSequence          = journeyStepExitConnection->getSequenceInTrip();
            inVehicleTime               = arrivalTime   - departureTime;
            waitingTime                 = departureTime - transferArrivalTime;
            transferArrivalTime         = arrivalTime   + transferTime;
            transferReadyTime           = transferArrivalTime;

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
            legs.push_back(std::make_tuple(std::ref(journeyStepTrip), boardingSequence - 1, unboardingSequence - 1));

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

            if (i == 1) // first leg
            {
              accessWaitingTime   = waitingTime;
            }
            else
            {
              totalTransferWaitingTime += waitingTime;
            }

            if (!params.returnAllNodesResult)
            {
              singleResult.get()->steps.push_back(std::make_unique<BoardingStep>(
                journeyStepTrip,
                boardingSequence,
                boardingSequence,
                journeyStepNodeDeparture,
                departureTime,
                waitingTime
              ));

              singleResult.get()->steps.push_back(std::make_unique<UnboardingStep>(
                journeyStepTrip,
                unboardingSequence,
                unboardingSequence + 1,
                journeyStepNodeArrival,
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

              transferArrivalTime  = bestDepartureTime + transferTime;
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
                  bestDepartureTime,
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
          if (reverseAccessJourneysSteps.at(resultingNode.uid).getFinalEnterConnection().has_value())
          {
            //TODO Should check taht MinWaiting is not -1 or use OrDefault(0)
            //TODO Could move this operation into a function of class Connection
            departureTime = reverseAccessJourneysSteps.at(resultingNode.uid).getFinalEnterConnection().value()->getDepartureTime() - reverseAccessJourneysSteps.at(resultingNode.uid).getFinalEnterConnection().value()->getMinWaitingTime();
            if (arrivalTimeSeconds - departureTime <=  parameters.getMaxTotalTravelTimeSeconds())
            {
              reachableNodesCount++;

              AccessibleNodes node = AccessibleNodes(
                resultingNode.uuid,
                arrivalTimeSeconds,
                arrivalTimeSeconds - departureTime,
                numberOfTransfers
              );
              allNodesResult.get()->nodes.push_back(node);
            }
          }
        }
        else
        {

          singleResult.get()->departureTime = bestDepartureTime;
          singleResult.get()->arrivalTime = arrivalTime;
          singleResult.get()->totalTravelTime = arrivalTime - bestDepartureTime;
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
