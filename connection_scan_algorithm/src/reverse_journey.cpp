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

namespace TrRouting
{

  std::unique_ptr<RoutingResult> Calculator::reverseJourneyStep(RouteParameters &parameters, int bestDepartureTime, int bestAccessNodeIndex, int bestAccessTravelTime, int bestAccessDistance)
  {

    // FIXME: One code path supports both Single calculation and all nodes, we shouldn't have to deal with both of these types here
    std::unique_ptr<SingleCalculationResult> singleResult = std::make_unique<SingleCalculationResult>();
    std::unique_ptr<AllNodesResult> allNodesResult = std::make_unique<AllNodesResult>();

    int              nodesCount           {1};
    int              i                    {0};
    int              reachableNodesCount  {0};
    bool             foundRoute           {false};

    std::vector<int> resultingNodes;
    if (params.returnAllNodesResult)
    {
      nodesCount     = nodes.size();
      resultingNodes = std::vector<int>(nodesCount);
      std::iota (std::begin(resultingNodes), std::end(resultingNodes), 0); // generate sequencial indexes of each nodes
    }
    else if (bestDepartureTime > -1) // route found
    {
      foundRoute = true;
      resultingNodes.push_back(bestAccessNodeIndex);
    } else {
      throw NoRoutingFoundException(NoRoutingReason::NO_ROUTING_FOUND);
    }

    if (foundRoute || params.returnAllNodesResult)
    {

      std::deque<std::tuple<int,int,int,int,int,short,int>> journey;
      std::tuple<int,int,int,int,int,short,int>         resultingNodeJourneyStep;
      std::tuple<int,int,int,int,int,short,int>         emptyJourneyStep {-1,-1,-1,-1,-1,-1,-1};
      ConnectionTuple * journeyStepEnterConnection; // connection tuple: departureNodeIndex, arrivalNodeIndex, departureTimeSeconds, arrivalTimeSeconds, tripIndex, canBoard, canUnboard, sequenceinTrip
      ConnectionTuple * journeyStepExitConnection;
      std::vector<boost::uuids::uuid>                   lineUuids;
      std::vector<int>                                  linesIdx;
      std::vector<boost::uuids::uuid>                   agencyUuids;
      std::vector<boost::uuids::uuid>                   unboardingNodeUuids;
      std::vector<boost::uuids::uuid>                   boardingNodeUuids;
      std::vector<boost::uuids::uuid>                   tripUuids;
      std::vector<int>                                  tripsIdx;
      std::vector<int>                                  inVehicleTravelTimesSeconds; // the in vehicle travel time for each segment
      std::vector<std::tuple<int, int, int, int, int>>  legs; // tuple: tripIdx, lineIdx, pathIdx, start connection index, end connection index

      Node *   journeyStepNodeDeparture;
      Node *   journeyStepNodeArrival;
      Trip *   journeyStepTrip;
      Line *   journeyStepLine;
      Path *   journeyStepPath;
      Agency * journeyStepAgency;

      int totalInVehicleTime       { 0}; int transferArrivalTime    {-1}; int firstDepartureTime   {-1};
      int totalWalkingTime         { 0}; int transferReadyTime      {-1}; int numberOfTransfers    {-1};
      int totalWaitingTime         { 0}; int departureTime          {-1}; int boardingSequence     {-1};
      int totalTransferWalkingTime { 0}; int arrivalTime            {-1}; int unboardingSequence   {-1};
      int totalTransferWaitingTime { 0}; int inVehicleTime          {-1}; int bestEgressNodeIndex  {-1};
      int totalDistance            { 0}; int distance               {-1};
      int inVehicleDistance        {-1}; int totalInVehicleDistance { 0}; int totalWalkingDistance { 0};
      int totalTransferDistance    {-1}; int accessDistance         { 0}; int egressDistance       { 0};
      int journeyStepTravelTime    {-1}; int accessWalkingTime      {-1};
      int transferTime             {-1}; int egressWalkingTime      {-1};
      int waitingTime              {-1}; int accessWaitingTime      {-1};

      for (auto & resultingNodeIndex : resultingNodes)
      {

        legs.clear();
        journey.clear();
        lineUuids.clear();
        linesIdx.clear();
        boardingNodeUuids.clear();
        unboardingNodeUuids.clear();
        tripUuids.clear();
        tripsIdx.clear();
        inVehicleTravelTimesSeconds.clear();

        totalInVehicleTime          =  0; transferArrivalTime    = -1; firstDepartureTime   = -1;
        totalWalkingTime            =  0; transferReadyTime      = -1; numberOfTransfers    = -1;
        totalWaitingTime            =  0; departureTime          = -1; boardingSequence     = -1;
        totalTransferWalkingTime    =  0; arrivalTime            = -1; unboardingSequence   = -1;
        totalTransferWaitingTime    =  0; inVehicleTime          = -1; bestEgressNodeIndex  = -1;
        totalDistance               =  0; distance               = -1;
        inVehicleDistance           =  0; totalInVehicleDistance =  0; totalWalkingDistance =  0;
        totalTransferDistance       =  0; accessDistance         =  0; egressDistance       =  0;
        journeyStepTravelTime       = -1; accessWalkingTime      = -1;
        transferTime                = -1; egressWalkingTime      = -1;
        waitingTime                 = -1; accessWaitingTime      = -1;

        // recreate journey:
        resultingNodeJourneyStep = reverseAccessJourneysSteps[resultingNodeIndex];

        if (resultingNodeJourneyStep == emptyJourneyStep) // ignore nodes with no route
        {
          continue;
        }

        while ((std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(resultingNodeJourneyStep) != -1 && std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION>(resultingNodeJourneyStep) != -1))
        {
          // here we inverse the transfers (putting them after the exit connection, instead of before the enter connection):
          if (journey.size() > 0)
          {
            std::get<journeyStepIndexes::TRANSFER_TRAVEL_TIME>(journey[journey.size()-1]) = std::get<journeyStepIndexes::TRANSFER_TRAVEL_TIME>(resultingNodeJourneyStep);
            std::get<journeyStepIndexes::TRANSFER_DISTANCE>(journey[journey.size()-1])    = std::get<journeyStepIndexes::TRANSFER_DISTANCE>(resultingNodeJourneyStep);
          }
          journey.push_back(resultingNodeJourneyStep);
          bestEgressNodeIndex      = std::get<connectionIndexes::NODE_ARR>(*reverseConnections[std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION>(resultingNodeJourneyStep)].get());
          resultingNodeJourneyStep = reverseJourneysSteps[bestEgressNodeIndex];
        }

        if (!params.returnAllNodesResult)
        {
          journey.push_front(std::make_tuple(-1,-1,-1,-1,nodesAccessTravelTime[resultingNodeIndex],-1,nodesAccessDistance[resultingNodeIndex]));
        }
        journey.push_back(std::make_tuple(-1,-1,-1,-1,nodesEgressTravelTime[bestEgressNodeIndex],-1,nodesEgressDistance[bestEgressNodeIndex]));



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

        int journeyStepsCount = journey.size();
        i = 0;
        for (auto & journeyStep : journey)
        {

          // check if it is an in-vehicle journey:
          if (std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journeyStep) != -1 && std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION>(journeyStep) != -1)
          {
            // journey tuple: final enter connection, final exit connection, final footpath
            journeyStepEnterConnection  = reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journeyStep)].get();
            journeyStepExitConnection   = reverseConnections[std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION>(journeyStep)].get();
            journeyStepNodeDeparture    = nodes[std::get<connectionIndexes::NODE_DEP>(*journeyStepEnterConnection)].get();
            journeyStepNodeArrival      = nodes[std::get<connectionIndexes::NODE_ARR>(*journeyStepExitConnection)].get();
            journeyStepTrip             = trips[std::get<journeyStepIndexes::FINAL_TRIP>(journeyStep)].get();
            journeyStepAgency           = agencies[journeyStepTrip->agencyIdx].get();
            journeyStepLine             = lines[journeyStepTrip->lineIdx].get();
            journeyStepPath             = paths[journeyStepTrip->pathIdx].get();
            transferTime                = std::get<journeyStepIndexes::TRANSFER_TRAVEL_TIME>(journeyStep);
            distance                    = std::get<journeyStepIndexes::TRANSFER_DISTANCE>(journeyStep);
            inVehicleDistance           = 0;
            departureTime               = std::get<connectionIndexes::TIME_DEP>(*journeyStepEnterConnection);
            arrivalTime                 = std::get<connectionIndexes::TIME_ARR>(*journeyStepExitConnection);
            boardingSequence            = std::get<connectionIndexes::SEQUENCE>(*journeyStepEnterConnection);
            unboardingSequence          = std::get<connectionIndexes::SEQUENCE>(*journeyStepExitConnection);
            inVehicleTime               = arrivalTime   - departureTime;
            waitingTime                 = departureTime - transferArrivalTime;
            transferArrivalTime         = arrivalTime   + transferTime;
            transferReadyTime           = transferArrivalTime;

            if (journey.size() > i + 1 && std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[i+1]) != -1)
            {
              transferReadyTime += (std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(*reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[i+1])].get()) >= 0 ? std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(*reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[i+1])].get()) : parameters.getMinWaitingTimeSeconds());
            }

            totalInVehicleTime         += inVehicleTime;
            totalWaitingTime           += waitingTime;
            if (Mode::TRANSFERABLE != journeyStepLine->mode.shortname)
            {
              numberOfTransfers += 1;
            }
            lineUuids.push_back(journeyStepLine->uuid);
            linesIdx.push_back(journeyStepTrip->lineIdx);
            inVehicleTravelTimesSeconds.push_back(inVehicleTime);
            agencyUuids.push_back(journeyStepAgency->uuid);
            boardingNodeUuids.push_back(journeyStepNodeDeparture->uuid);
            unboardingNodeUuids.push_back(journeyStepNodeArrival->uuid);
            tripUuids.push_back(journeyStepTrip->uuid);
            tripsIdx.push_back(std::get<journeyStepIndexes::FINAL_TRIP>(journeyStep));
            legs.push_back(std::make_tuple(std::get<journeyStepIndexes::FINAL_TRIP>(journeyStep), journeyStepTrip->lineIdx, journeyStepTrip->pathIdx, boardingSequence - 1, unboardingSequence - 1));

            if (unboardingSequence - 1 < journeyStepPath->segmentsDistanceMeters.size()) // check if distances are available for this path
            {
              for (int seqI = boardingSequence - 1; seqI < unboardingSequence; seqI++)
              {
                inVehicleDistance += journeyStepPath->segmentsDistanceMeters[seqI];
              }
              totalDistance += inVehicleDistance;
              if (Mode::TRANSFERABLE == journeyStepLine->mode.shortname)
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
              firstDepartureTime  = departureTime;
              accessWaitingTime   = waitingTime;
            }
            else
            {
              totalTransferWaitingTime += waitingTime;
            }

            if (!params.returnAllNodesResult)
            {
              singleResult.get()->steps.push_back(std::make_unique<BoardingStep>(
                journeyStepAgency->uuid,
                journeyStepAgency->acronym,
                journeyStepAgency->name,
                journeyStepLine->uuid,
                journeyStepLine->shortname,
                journeyStepLine->longname,
                journeyStepPath->uuid,
                journeyStepLine->mode.name,
                journeyStepLine->mode.shortname,
                journeyStepTrip->uuid,
                boardingSequence,
                boardingSequence,
                journeyStepNodeDeparture->uuid,
                journeyStepNodeDeparture->code,
                journeyStepNodeDeparture->name,
                *journeyStepNodeDeparture->point.get(),
                departureTime,
                waitingTime
              ));

              singleResult.get()->steps.push_back(std::make_unique<UnboardingStep>(
                journeyStepAgency->uuid,
                journeyStepAgency->acronym,
                journeyStepAgency->name,
                journeyStepLine->uuid,
                journeyStepLine->shortname,
                journeyStepLine->longname,
                journeyStepPath->uuid,
                journeyStepLine->mode.name,
                journeyStepLine->mode.shortname,
                journeyStepTrip->uuid,
                unboardingSequence,
                unboardingSequence + 1,
                journeyStepNodeArrival->uuid,
                journeyStepNodeArrival->code,
                journeyStepNodeArrival->name,
                *journeyStepNodeArrival->point.get(),
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

            transferTime          = std::get<journeyStepIndexes::TRANSFER_TRAVEL_TIME>(journeyStep);
            distance              = std::get<journeyStepIndexes::TRANSFER_DISTANCE>(journeyStep);
            if (totalDistance != -1)
            {
              totalDistance += distance;
            }
            totalWalkingDistance += distance;
            if (i == 0) // access
            {

              transferArrivalTime  = bestDepartureTime + transferTime;
              transferReadyTime    = transferArrivalTime;

              if (journey.size() > i + 1 && std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[i+1]) != -1)
              {
                transferReadyTime += (std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(*reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[i+1])].get()) >= 0 ? std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(*reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[i+1])].get()) : parameters.getMinWaitingTimeSeconds());
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
          if (std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(reverseAccessJourneysSteps[resultingNodeIndex]) != -1)
          {
            departureTime = std::get<connectionIndexes::TIME_DEP>(*reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(reverseAccessJourneysSteps[resultingNodeIndex])].get()) - std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(*reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(reverseAccessJourneysSteps[resultingNodeIndex])].get());
            if (arrivalTimeSeconds - departureTime <=  parameters.getMaxTotalTravelTimeSeconds())
            {
              reachableNodesCount++;

              AccessibleNodes node = AccessibleNodes(
                nodes[resultingNodeIndex].get()->uuid,
                arrivalTimeSeconds,
                arrivalTimeSeconds - departureTime,
                numberOfTransfers
              );
              allNodesResult.get()->nodes.push_back(node);
            }
          }
        }
        else if (!params.returnAllNodesResult)
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
      return std::move(allNodesResult);
    }

    return std::move(singleResult);
  }

}
