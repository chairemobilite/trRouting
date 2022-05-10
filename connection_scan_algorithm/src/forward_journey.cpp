
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
#include <iostream>

namespace TrRouting
{

  std::unique_ptr<RoutingResult> Calculator::forwardJourneyStep(RouteParameters &parameters, int bestArrivalTime, int bestEgressNodeIndex, int bestEgressTravelTime, int bestEgressDistance)
  {

    // FIXME: One code path supports both Single calculation and all nodes, we shouldn't have to deal with both of these types here
    std::unique_ptr<SingleCalculationResult> singleResult = std::make_unique<SingleCalculationResult>();
    std::unique_ptr<AllNodesResult> allNodesResult = std::make_unique<AllNodesResult>();

    int              nodesCount           {1};
    int              i                    {0};
    int              reachableNodesCount  {0};
    bool             foundLine            {false};

    int transferableModeIdx {modeIndexesByShortname.find("transferable") != modeIndexesByShortname.end() ? modeIndexesByShortname["transferable"] : -1};

    std::vector<int> resultingNodes;
    if (params.returnAllNodesResult)
    {
      nodesCount     = nodes.size();
      resultingNodes = std::vector<int>(nodesCount);
      std::iota (std::begin(resultingNodes), std::end(resultingNodes), 0); // generate sequencial indexes of each nodes
    }
    else if (bestArrivalTime < MAX_INT) // line found
    {
      foundLine = true;
      resultingNodes.push_back(bestEgressNodeIndex);
    } else {
      throw NoRoutingFoundException(NoRoutingReason::NO_ROUTING_FOUND);
    }

    if (foundLine || params.returnAllNodesResult)
    {

      std::deque<std::tuple<int,int,int,int,int,short,int>> journey;
      std::tuple<int,int,int,int,int,short,int>         resultingNodeJourneyStep;
      std::tuple<int,int,int,int,int,short,int>         emptyJourneyStep {-1,-1,-1,-1,-1,-1,-1};
      ConnectionTuple * journeyStepEnterConnection; // connection tuple: departureNodeIndex, arrivalNodeIndex, departureTimeSeconds, arrivalTimeSeconds, tripIndex, canBoard, canUnboard, sequenceinTrip
      ConnectionTuple * journeyStepExitConnection;
      std::vector<boost::uuids::uuid>                   lineUuids;
      std::vector<int>                                  linesIdx;
      std::vector<std::string>                          modeShortnames;
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
      Mode     journeyStepMode;
      Path *   journeyStepPath;
      Agency * journeyStepAgency;

      int totalInVehicleTime       { 0}; int transferArrivalTime    {-1}; int firstDepartureTime     {-1};
      int totalWalkingTime         { 0}; int transferReadyTime      {-1}; int minimizedDepartureTime {-1};
      int totalWaitingTime         { 0}; int departureTime          {-1}; int numberOfTransfers      {-1};
      int totalTransferWalkingTime { 0}; int arrivalTime            {-1}; int boardingSequence       {-1};
      int totalTransferWaitingTime { 0}; int inVehicleTime          {-1}; int unboardingSequence     {-1};
      int totalDistance            { 0}; int distance               {-1};
      int inVehicleDistance        {-1}; int totalInVehicleDistance { 0}; int totalWalkingDistance   { 0};
      int totalTransferDistance    {-1}; int accessDistance         { 0}; int egressDistance         { 0};
      int journeyStepTravelTime    {-1}; int accessWalkingTime      {-1}; int bestAccessNodeIndex    {-1};
      int transferTime             {-1}; int egressWalkingTime      {-1}; int bestAccessNodePrevIndex{-1};
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
        modeShortnames.clear();
        inVehicleTravelTimesSeconds.clear();

        totalInVehicleTime       =  0; transferArrivalTime    = -1; firstDepartureTime      = -1;
        totalWalkingTime         =  0; transferReadyTime      = -1; minimizedDepartureTime  = -1;
        totalWaitingTime         =  0; departureTime          = -1; numberOfTransfers       = -1;
        totalTransferWalkingTime =  0; arrivalTime            = -1; boardingSequence        = -1;
        totalTransferWaitingTime =  0; inVehicleTime          = -1; unboardingSequence      = -1;
        totalDistance            =  0; distance               = -1;
        inVehicleDistance        =  0; totalInVehicleDistance =  0; totalWalkingDistance    =  0;
        totalTransferDistance    =  0; accessDistance         =  0; egressDistance          =  0;
        journeyStepTravelTime    = -1; accessWalkingTime      = -1; bestAccessNodeIndex     = -1;
        transferTime             = -1; egressWalkingTime      = -1; bestAccessNodePrevIndex = -1;
        waitingTime              = -1; accessWaitingTime      = -1;

        //std::cerr << nodes[resultingNodeIndex].get()->name << " : " << std::get<0>(forwardJourneysSteps[resultingNodeIndex]) << std::endl;
        // recreate journey:
        resultingNodeJourneyStep = forwardEgressJourneysSteps[resultingNodeIndex];

        if (resultingNodeJourneyStep == emptyJourneyStep) // ignore nodes with no line
        {
          continue;
        }


        while ((std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(resultingNodeJourneyStep) != -1 && std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION>(resultingNodeJourneyStep) != -1))
        {
          journey.push_front(resultingNodeJourneyStep);
          bestAccessNodeIndex = std::get<connectionIndexes::NODE_DEP>(*forwardConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(resultingNodeJourneyStep)].get());
          if (bestAccessNodePrevIndex != -1 && bestAccessNodeIndex != -1 && bestAccessNodePrevIndex == bestAccessNodeIndex) {
            // this should not happen, it means that there is an ambiguity in the data, which makes this loop runs forever and create a memory leak filling journey indefinitely.
            // TODO: add tests
            std::cerr << "Invalid data in path uuid: " << paths[trips[std::get<journeyStepIndexes::FINAL_TRIP>(resultingNodeJourneyStep)].get()->pathIdx].get()->uuid << std::endl;
            throw InvalidDataException(NoRoutingReason::INVALID_DATA);
          }
          resultingNodeJourneyStep = forwardJourneysSteps[bestAccessNodeIndex];
          bestAccessNodePrevIndex = bestAccessNodeIndex;
        }

        if (!params.returnAllNodesResult)
        {
          journey.push_back(std::make_tuple(-1,-1,-1,-1,nodesEgressTravelTime[resultingNodeIndex],-1,nodesEgressDistance[resultingNodeIndex]));
        }
        journey.push_front(std::make_tuple(-1,-1,-1,-1,nodesAccessTravelTime[bestAccessNodeIndex],-1,nodesAccessDistance[bestAccessNodeIndex]));

        //std::string stepsJson = "  \"steps\":\n  [\n";

        i = 0;
        int journeyStepsCount = journey.size();
        for (auto & journeyStep : journey)
        {

          if (std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journeyStep) != -1 && std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION>(journeyStep) != -1)
          {
            // journey tuple: final enter connection, final exit connection, final footpath
            journeyStepEnterConnection = forwardConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journeyStep)].get();
            journeyStepExitConnection  = forwardConnections[std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION>(journeyStep)].get();
            journeyStepNodeDeparture   = nodes[std::get<connectionIndexes::NODE_DEP>(*journeyStepEnterConnection)].get();
            journeyStepNodeArrival     = nodes[std::get<connectionIndexes::NODE_ARR>(*journeyStepExitConnection)].get();
            journeyStepTrip            = trips[std::get<journeyStepIndexes::FINAL_TRIP>(journeyStep)].get();
            journeyStepAgency          = agencies[journeyStepTrip->agencyIdx].get();
            journeyStepLine            = lines[journeyStepTrip->lineIdx].get();
            journeyStepPath            = paths[journeyStepTrip->pathIdx].get();
            journeyStepMode            = modes[journeyStepLine->modeIdx];
            transferTime               = std::get<journeyStepIndexes::TRANSFER_TRAVEL_TIME>(journeyStep);
            distance                   = std::get<journeyStepIndexes::TRANSFER_DISTANCE>(journeyStep);
            inVehicleDistance          = 0;
            departureTime              = std::get<connectionIndexes::TIME_DEP>(*journeyStepEnterConnection);
            arrivalTime                = std::get<connectionIndexes::TIME_ARR>(*journeyStepExitConnection);
            boardingSequence           = std::get<connectionIndexes::SEQUENCE>(*journeyStepEnterConnection);
            unboardingSequence         = std::get<connectionIndexes::SEQUENCE>(*journeyStepExitConnection);
            inVehicleTime              = arrivalTime   - departureTime;
            waitingTime                = departureTime - transferArrivalTime;
            transferArrivalTime        = arrivalTime   + transferTime;
            transferReadyTime          = transferArrivalTime;

            if (journey.size() > i + 1 && std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[i+1]) != -1)
            {
              transferReadyTime += (std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(*forwardConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[i+1])].get()) >= 0 ? std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(*forwardConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[i+1])].get()) : parameters.getMinWaitingTimeSeconds());
            }

            totalInVehicleTime         += inVehicleTime;
            totalWaitingTime           += waitingTime;
            if (transferableModeIdx != journeyStepLine->modeIdx)
            {
              numberOfTransfers += 1;
            }
            lineUuids.push_back(journeyStepLine->uuid);
            linesIdx.push_back(journeyStepTrip->lineIdx);
            modeShortnames.push_back(journeyStepMode.shortname);
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
              if (transferableModeIdx == journeyStepLine->modeIdx)
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
              accessWaitingTime      = waitingTime;
              firstDepartureTime     = departureTime;
              minimizedDepartureTime = firstDepartureTime - accessWalkingTime - (std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(*forwardConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[1])].get()) >= 0 ? std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(*forwardConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[1])].get()) : parameters.getMinWaitingTimeSeconds());
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
                journeyStepMode.name,
                journeyStepMode.shortname,
                journeyStepTrip->uuid,
                boardingSequence,
                boardingSequence,
                journeyStepNodeDeparture->uuid,
                journeyStepNodeDeparture->code,
                journeyStepNodeDeparture->name,
                *journeyStepNodeDeparture->point.get(),
                minimizedDepartureTime,
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
                journeyStepMode.name,
                journeyStepMode.shortname,
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
              transferArrivalTime  = departureTimeSeconds + transferTime;
              transferReadyTime    = transferArrivalTime;

              if (journey.size() > i + 1 && std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[i+1]) != -1)
              {
                transferReadyTime += (std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(*forwardConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[i+1])].get()) >= 0 ? std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(*forwardConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[i+1])].get()) : parameters.getMinWaitingTimeSeconds());
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
          if (std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(forwardEgressJourneysSteps[resultingNodeIndex]) != -1)
          {
            arrivalTime = std::get<connectionIndexes::TIME_ARR>(*forwardConnections[std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION>(forwardEgressJourneysSteps[resultingNodeIndex])].get());
            if (arrivalTime - departureTimeSeconds <= parameters.getMaxTotalTravelTimeSeconds())
            {
              reachableNodesCount++;

              AccessibleNodes node = AccessibleNodes(
                nodes[resultingNodeIndex].get()->uuid,
                arrivalTime,
                arrivalTime - departureTimeSeconds,
                numberOfTransfers
              );
              allNodesResult.get()->nodes.push_back(node);
            }
          }
        }
        else if (!params.returnAllNodesResult)
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

    if (params.debugDisplay)
      std::cerr << "-- forward result: " << (params.returnAllNodesResult ? " allNodes " : "single calculation") << std::endl;

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
