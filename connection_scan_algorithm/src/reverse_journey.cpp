#include "calculator.hpp"
#include "constants.hpp"
#include "json.hpp"

namespace TrRouting
{

  RoutingResult Calculator::reverseJourneyStep(int bestDepartureTime, int bestAccessNodeIndex, int bestAccessTravelTime, int bestAccessDistance)
  {
  
    RoutingResult    result;
    nlohmann::json   json;
    int              nodesCount           {1};
    int              i                    {0};
    int              reachableNodesCount  {0};
    bool             foundRoute           {false};

    int transferableModeIdx {modeIndexesByShortname["transferable"]};

    std::vector<int> resultingNodes;
    if (params.returnAllNodesResult)
    {
      nodesCount     = nodes.size();
      // Initialize status to no routing
      json["status"] = STATUS_NO_ROUTING_FOUND;
      json["nodes"]  = nlohmann::json::array();
      resultingNodes = std::vector<int>(nodesCount);
      std::iota (std::begin(resultingNodes), std::end(resultingNodes), 0); // generate sequencial indexes of each nodes
    }
    else if (bestDepartureTime > -1) // route found
    {
      foundRoute = true;
      resultingNodes.push_back(bestAccessNodeIndex);
    }

    if (foundRoute || params.returnAllNodesResult)
    {

      std::deque<std::tuple<int,int,int,int,int,short,int>> journey;
      std::tuple<int,int,int,int,int,short,int>         resultingNodeJourneyStep;
      std::tuple<int,int,int,int,int,short,int>         emptyJourneyStep {-1,-1,-1,-1,-1,-1,-1};
      std::tuple<int,int,int,int,int,short,short,int,int,int,short,short> * journeyStepEnterConnection; // connection tuple: departureNodeIndex, arrivalNodeIndex, departureTimeSeconds, arrivalTimeSeconds, tripIndex, canBoard, canUnboard, sequenceinTrip
      std::tuple<int,int,int,int,int,short,short,int,int,int,short,short> * journeyStepExitConnection;
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
      nlohmann::json stepJson = {};
      nlohmann::json nodeJson = {};

      Node *   journeyStepNodeDeparture;
      Node *   journeyStepNodeArrival;
      Trip *   journeyStepTrip;
      Line *   journeyStepLine;
      Mode     journeyStepMode;
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
        modeShortnames.clear();
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
        //std::cerr << nodes[bestEgressNodeIndex].get()->name << std::endl;
        i = 0;
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
          
          i++;
        }
        
        if (!params.returnAllNodesResult)
        {
          json["steps"] = nlohmann::json::array();
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

        json["optimizeCases"] = optimizeCasesStr;



        std::string stepsStr = "";


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
            journeyStepMode             = modes[journeyStepLine->modeIdx];
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
              transferReadyTime += (std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(*reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[i+1])].get()) >= 0 ? std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(*reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[i+1])].get()) : params.minWaitingTimeSeconds);
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
              firstDepartureTime  = departureTime;
              accessWaitingTime   = waitingTime;
            }
            else
            {
              totalTransferWaitingTime += waitingTime;
            }
            
            if (!params.returnAllNodesResult)
            {
              stepJson                         = {};
              stepJson["action"]               = "board";
              stepJson["agencyAcronym"]        = journeyStepAgency->acronym;
              stepJson["agencyName"]           = journeyStepAgency->name;
              stepJson["agencyUuid"]           = boost::uuids::to_string(journeyStepAgency->uuid);
              stepJson["lineShortname"]        = journeyStepLine->shortname;
              stepJson["lineLongname"]         = journeyStepLine->longname;
              stepJson["lineUuid"]             = boost::uuids::to_string(journeyStepLine->uuid);
              stepJson["pathUuid"]             = boost::uuids::to_string(journeyStepPath->uuid);
              stepJson["modeName"]             = journeyStepMode.name;
              stepJson["mode"]                 = journeyStepMode.shortname;
              stepJson["tripUuid"]             = boost::uuids::to_string(journeyStepTrip->uuid);
              stepJson["legSequenceInTrip"]    = boardingSequence;
              stepJson["stopSequenceInTrip"]   = boardingSequence;
              stepJson["nodeName"]             = journeyStepNodeDeparture->name;
              stepJson["nodeCode"]             = journeyStepNodeDeparture->code;
              stepJson["nodeUuid"]             = boost::uuids::to_string(journeyStepNodeDeparture->uuid);
              stepJson["nodeCoordinates"]      = {journeyStepNodeDeparture->point.get()->longitude, journeyStepNodeDeparture->point.get()->latitude};
              stepJson["departureTime"]        = Toolbox::convertSecondsToFormattedTime(departureTime);
              stepJson["departureTimeSeconds"] = departureTime;
              stepJson["waitingTimeSeconds"]   = waitingTime;
              stepJson["waitingTimeMinutes"]   = Toolbox::convertSecondsToMinutes(waitingTime);
              json["steps"].push_back(stepJson);

              stepJson                                = {};
              stepJson["action"]                      = "unboard";
              stepJson["agencyAcronym"]               = journeyStepAgency->acronym;
              stepJson["agencyName"]                  = journeyStepAgency->name;
              stepJson["agencyUuid"]                  = boost::uuids::to_string(journeyStepAgency->uuid);
              stepJson["lineShortname"]               = journeyStepLine->shortname;
              stepJson["lineLongname"]                = journeyStepLine->longname;
              stepJson["lineUuid"]                    = boost::uuids::to_string(journeyStepLine->uuid);
              stepJson["pathUuid"]                    = boost::uuids::to_string(journeyStepPath->uuid);
              stepJson["modeName"]                    = journeyStepMode.name;
              stepJson["mode"]                        = journeyStepMode.shortname;
              stepJson["tripUuid"]                    = boost::uuids::to_string(journeyStepTrip->uuid);
              stepJson["legSequenceInTrip"]           = unboardingSequence;
              stepJson["stopSequenceInTrip"]          = unboardingSequence + 1;
              stepJson["nodeName"]                    = journeyStepNodeArrival->name;
              stepJson["nodeCode"]                    = journeyStepNodeArrival->code;
              stepJson["nodeUuid"]                    = boost::uuids::to_string(journeyStepNodeArrival->uuid);
              stepJson["nodeCoordinates"]             = {journeyStepNodeArrival->point.get()->longitude, journeyStepNodeArrival->point.get()->latitude};
              stepJson["arrivalTime"]                 = Toolbox::convertSecondsToFormattedTime(arrivalTime);
              stepJson["arrivalTimeSeconds"]          = arrivalTime;
              stepJson["inVehicleTimeSeconds"]        = inVehicleTime;
              stepJson["inVehicleTimeMinutes"]        = Toolbox::convertSecondsToMinutes(inVehicleTime);
              stepJson["inVehicleDistanceMeters"]     = inVehicleDistance;
              json["steps"].push_back(stepJson);
              stepsStr += "wait" + std::to_string(waitingTime) + "s|";
              if (transferableModeIdx == journeyStepLine->modeIdx)
              {
                stepsStr += "transferable" + std::to_string(inVehicleTime) + "s" + std::to_string(inVehicleDistance) + "m|";
              }
              else
              {
                stepsStr += "ride" + std::to_string(inVehicleTime) + "s" + std::to_string(inVehicleDistance) + "m|";
              }
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
                stepJson                         = {};
                stepJson["action"]               = "walking";
                stepJson["type"]                 = "transfer";
                stepJson["travelTimeSeconds"]    = transferTime;
                stepJson["travelTimeMinutes"]    = Toolbox::convertSecondsToMinutes(transferTime);
                stepJson["distanceMeters"]       = distance;
                stepJson["departureTime"]        = Toolbox::convertSecondsToFormattedTime(arrivalTime);
                stepJson["arrivalTime"]          = Toolbox::convertSecondsToFormattedTime(transferArrivalTime);
                stepJson["departureTimeSeconds"] = arrivalTime;
                stepJson["arrivalTimeSeconds"]   = transferArrivalTime;
                stepJson["readyToBoardAt"]       = Toolbox::convertSecondsToFormattedTime(transferReadyTime);
                json["steps"].push_back(stepJson);
                stepsStr += "transfer" + std::to_string(transferTime) + "s" + std::to_string(distance) + "m|";
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
                transferReadyTime += (std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(*reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[i+1])].get()) >= 0 ? std::get<connectionIndexes::MIN_WAITING_TIME_SECONDS>(*reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[i+1])].get()) : params.minWaitingTimeSeconds);
              }

              totalWalkingTime    += transferTime;
              accessWalkingTime    = transferTime;
              accessDistance       = distance;
              if (!params.returnAllNodesResult)
              {
                stepJson                          = {};
                stepJson["action"]                = "walking";
                stepJson["type"]                  = "access";
                stepJson["travelTimeSeconds"]     = transferTime;
                stepJson["travelTimeMinutes"]     = Toolbox::convertSecondsToMinutes(transferTime);
                stepJson["distanceMeters"]        = distance;
                stepJson["departureTime"]         = Toolbox::convertSecondsToFormattedTime(bestDepartureTime);
                stepJson["departureTime"]         = Toolbox::convertSecondsToFormattedTime(bestDepartureTime);
                stepJson["arrivalTime"]           = Toolbox::convertSecondsToFormattedTime(transferArrivalTime);
                stepJson["departureTimeSeconds"]  = bestDepartureTime;
                stepJson["arrivalTimeSeconds"]    = transferArrivalTime;
                stepJson["readyToBoardAtSeconds"] = transferReadyTime;
                stepJson["readyToBoardAt"]        = Toolbox::convertSecondsToFormattedTime(transferReadyTime);
                json["steps"].push_back(stepJson);
                stepsStr += "access" + std::to_string(transferTime) + "s" + std::to_string(distance) + "m|";
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
                stepJson                         = {};
                stepJson["action"]               = "walking";
                stepJson["type"]                 = "egress";
                stepJson["travelTimeSeconds"]    = transferTime;
                stepJson["travelTimeMinutes"]    = Toolbox::convertSecondsToMinutes(transferTime);
                stepJson["distanceMeters"]       = distance;
                stepJson["departureTime"]        = Toolbox::convertSecondsToFormattedTime(arrivalTime);
                stepJson["arrivalTime"]          = Toolbox::convertSecondsToFormattedTime(arrivalTime + transferTime);
                stepJson["departureTimeSeconds"] = arrivalTime;
                stepJson["arrivalTimeSeconds"]   = arrivalTime + transferTime;
                json["steps"].push_back(stepJson);
                stepsStr += "egress" + std::to_string(transferTime) + "s" + std::to_string(distance) + "m|";
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
            if (arrivalTimeSeconds - departureTime <= params.maxTotalTravelTimeSeconds)
            {
              reachableNodesCount++;
              json["status"]                     = STATUS_SUCCESS;
              nodeJson                           = {};
              nodeJson["id"]                     = boost::uuids::to_string(nodes[resultingNodeIndex].get()->uuid);
              nodeJson["departureTime"]          = Toolbox::convertSecondsToFormattedTime(departureTime);
              nodeJson["departureTimeSeconds"]   = departureTime;
              nodeJson["totalTravelTimeSeconds"] = arrivalTimeSeconds - departureTime;
              nodeJson["numberOfTransfers"]      = numberOfTransfers;
              json["nodes"].push_back(nodeJson);
            }
          }
        }
        else if (!params.returnAllNodesResult)
        {



          if (json["steps"].size() > 0)
          {


            // get modes and lines as strings

            std::string modesStr = "";
            for (std::string modeShortname : modeShortnames)
            {
              modesStr += modeShortname + "|";
            }
            if (modesStr.size() > 0)
            {
              modesStr.pop_back();
            }

            std::string lineUuidsStr = "";
            for (boost::uuids::uuid lineUuid : lineUuids)
            {
              lineUuidsStr += boost::uuids::to_string(lineUuid) + "|";
            }
            if (lineUuidsStr.size() > 0)
            {
              lineUuidsStr.pop_back();
            }

            if (stepsStr.size() > 0)
            {
              stepsStr.pop_back();
            }

            json["status"]                                         = STATUS_SUCCESS;
            json["origin"]                                         = { origin->longitude,      origin->latitude      };
            json["destination"]                                    = { destination->longitude, destination->latitude };
            json["departureTime"]                                  = Toolbox::convertSecondsToFormattedTime(bestDepartureTime);
            json["departureTimeSeconds"]                           = bestDepartureTime;
            if (initialDepartureTimeSeconds != -1)
            {
              json["initialDepartureTime"]                         = Toolbox::convertSecondsToFormattedTime(initialDepartureTimeSeconds);
              json["initialDepartureTimeSeconds"]                  = initialDepartureTimeSeconds;
              json["initialLostTimeAtDepartureSeconds"]            = bestDepartureTime - initialDepartureTimeSeconds;
              json["initialLostTimeAtDepartureMinutes"]            = Toolbox::convertSecondsToMinutes(bestDepartureTime - initialDepartureTimeSeconds);
            }
            json["arrivalTime"]                                    = Toolbox::convertSecondsToFormattedTime(arrivalTime);
            json["arrivalTimeSeconds"]                             = arrivalTime;
            json["totalTravelTimeMinutes"]                         = Toolbox::convertSecondsToMinutes(arrivalTime - bestDepartureTime);
            json["totalTravelTimeSeconds"]                         = arrivalTime - bestDepartureTime;
            json["totalDistanceMeters"]                            = totalDistance;
            json["totalInVehicleTimeMinutes"]                      = Toolbox::convertSecondsToMinutes(totalInVehicleTime);
            json["totalInVehicleTimeSeconds"]                      = totalInVehicleTime;
            json["totalInVehicleDistanceMeters"]                   = totalInVehicleDistance;
            json["totalNonTransitTravelTimeMinutes"]               = Toolbox::convertSecondsToMinutes(totalWalkingTime);
            json["totalNonTransitTravelTimeSeconds"]               = totalWalkingTime;
            json["totalNonTransitDistanceMeters"]                  = totalWalkingDistance;
            json["numberOfBoardings"]                              = numberOfTransfers + 1;
            json["numberOfTransfers"]                              = numberOfTransfers == -1 ? 0 : numberOfTransfers;
            json["transferWalkingTimeMinutes"]                     = Toolbox::convertSecondsToMinutes(totalTransferWalkingTime);
            json["transferWalkingTimeSeconds"]                     = totalTransferWalkingTime;
            json["transferWalkingDistanceMeters"]                  = totalTransferDistance;
            json["accessTravelTimeMinutes"]                        = Toolbox::convertSecondsToMinutes(accessWalkingTime);
            json["accessTravelTimeSeconds"]                        = accessWalkingTime;
            json["accessDistanceMeters"]                           = accessDistance;
            json["egressTravelTimeMinutes"]                        = Toolbox::convertSecondsToMinutes(egressWalkingTime);
            json["egressTravelTimeSeconds"]                        = egressWalkingTime;
            json["egressDistanceMeters"]                           = egressDistance;
            json["transferWaitingTimeMinutes"]                     = Toolbox::convertSecondsToMinutes(totalTransferWaitingTime);
            json["transferWaitingTimeSeconds"]                     = totalTransferWaitingTime;
            json["firstWaitingTimeMinutes"]                        = Toolbox::convertSecondsToMinutes(accessWaitingTime);
            json["firstWaitingTimeSeconds"]                        = accessWaitingTime;
            json["totalWaitingTimeMinutes"]                        = Toolbox::convertSecondsToMinutes(totalWaitingTime);
            json["totalWaitingTimeSeconds"]                        = totalWaitingTime;
            json["modes"]                                          = modesStr;
            json["lineUuids"]                                      = lineUuidsStr;
            json["stepsSummary"]                                   = stepsStr;
            /*json["exceptLineShortnames"]                           = nlohmann::json::array();
            
            for (auto & lineIdx : params.exceptLinesIdx)
            {
              json["exceptLineShortnames"].push_back(lines[lineIdx]->shortname);
            }*/

            result.travelTimeSeconds             = arrivalTime - bestDepartureTime;
            result.arrivalTimeSeconds            = arrivalTime;
            result.departureTimeSeconds          = bestDepartureTime;
            result.initialDepartureTimeSeconds   = initialDepartureTimeSeconds;
            result.initialLostTimeAtDepartureSeconds = initialDepartureTimeSeconds != -1 ? bestDepartureTime - initialDepartureTimeSeconds: -1;
            result.numberOfTransfers             = numberOfTransfers;
            result.inVehicleTravelTimeSeconds    = totalInVehicleTime;
            result.transferTravelTimeSeconds     = totalTransferWalkingTime;
            result.waitingTimeSeconds            = totalWaitingTime;
            result.accessTravelTimeSeconds       = accessWalkingTime;
            result.egressTravelTimeSeconds       = egressWalkingTime;
            result.transferWaitingTimeSeconds    = totalTransferWaitingTime;
            result.firstWaitingTimeSeconds       = accessWaitingTime;
            result.nonTransitTravelTimeSeconds   = totalWalkingTime;
            result.legs                          = legs;
            result.lineUuids                     = lineUuids;
            result.linesIdx                      = linesIdx;
            result.modeShortnames                = modeShortnames;
            result.agencyUuids                   = agencyUuids;
            result.boardingNodeUuids             = boardingNodeUuids;
            result.unboardingNodeUuids           = unboardingNodeUuids;
            result.tripUuids                     = tripUuids;
            result.tripsIdx                      = tripsIdx;
            result.inVehicleTravelTimesSeconds   = inVehicleTravelTimesSeconds;
            result.status                        = STATUS_SUCCESS;
            
          }
        }
      }
    }
    else // no route found
    {
      json["status"]                     = STATUS_NO_ROUTING_FOUND;
      json["origin"]                     = { origin->longitude,      origin->latitude      };
      json["destination"]                = { destination->longitude, destination->latitude };
      json["arrivalTime"]                = Toolbox::convertSecondsToFormattedTime(arrivalTimeSeconds);
      json["arrivalTimeSeconds"]         = arrivalTimeSeconds;
      result.status                      = STATUS_NO_ROUTING_FOUND;
      result.travelTimeSeconds           = -1;
      result.arrivalTimeSeconds          = arrivalTimeSeconds;
      result.departureTimeSeconds        = -1;
      result.initialDepartureTimeSeconds = -1;
      result.initialLostTimeAtDepartureSeconds = -1;
      result.numberOfTransfers           = -1;
      result.inVehicleTravelTimeSeconds  = -1;
      result.transferTravelTimeSeconds   = -1;
      result.waitingTimeSeconds          = -1;
      result.accessTravelTimeSeconds     = -1;
      result.egressTravelTimeSeconds     = -1;
      result.transferWaitingTimeSeconds  = -1;
      result.firstWaitingTimeSeconds     = -1;
      result.nonTransitTravelTimeSeconds = -1;
    }

    if (params.returnAllNodesResult)
    {
      json["numberOfReachableNodes"]  = reachableNodesCount;
      json["percentOfReachableNodes"] = round(10000 * (float)reachableNodesCount / (float)(nodesCount))/100.0;
    }

    result.json = json;
    
    return result;
  }

}
