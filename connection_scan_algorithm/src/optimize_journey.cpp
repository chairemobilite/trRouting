#include "calculator.hpp"

namespace TrRouting
{
  
  std::vector<int> Calculator::optimizeJourney(std::deque<std::tuple<int,int,int,int,int,short,int,int>> &journey)
  {

    CalculationTime algorithmCalculationTime = CalculationTime();
    algorithmCalculationTime.start();
    long long       calculationTime;
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

    std::vector<int> usedOptimizationCases;

    //json["optimizedNodes"] = nlohmann::json::array();

    short optimizationCase    {-1}; // 1: CSL, 2: BTS, 3: GTF, 4: CSS | For optimization cases diagrams, see in the references directory
    bool  startedOptimization {false};
    std::vector<int> ignoreOptimizationNodesIdx;

    while(!startedOptimization || optimizationCase >= 0)
    {
      
      startedOptimization     = true;
      optimizationCase        = -1;
      int journeyStepsCount   = journey.size();
      int journeyStepIdx      { 0};
      int fromJourneyStepIdx  {-1};
      int toJourneyStepIdx    {-1};
      int optimizationNodeIdx {-1};

      std::vector<int>              firstNodeIdxByJourneyStepIdx(journeyStepsCount, -1); // first node of each journey segment (boarding node)
      std::vector<int>              lastNodeIdxByJourneyStepIdx(journeyStepsCount, -1); // last node of each journey segment (unboarding node)
      std::vector<std::vector<int>> inBetweenNodesIdxByJourneyStepIdx(journeyStepsCount, std::vector<int>()); // all nodes between boarding and unboarding excluded for each journey segment
      
      for (auto & journeyStep : journey)
      {
        // parse only in-vehicle journey steps:
        if (
             std::get<journeyStepIndexes::FINAL_TRIP>(journeyStep)
          && std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journeyStep) != -1
          && std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION>(journeyStep)  != -1
        )
        {
          int tripIdx          = std::get<journeyStepIndexes::FINAL_TRIP>(journeyStep);
          int sequenceStartIdx = std::get<connectionIndexes::SEQUENCE>(*(reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journeyStep)].get())) - 1;
          int sequenceEndIdx   = std::get<connectionIndexes::SEQUENCE>(*(reverseConnections[std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION >(journeyStep)].get())) - 1;

          // get first and last nodes for the journey step trip segment (boarding and unboarding nodes):
          firstNodeIdxByJourneyStepIdx[journeyStepIdx] = std::get<connectionIndexes::NODE_DEP>(*(reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journeyStep)].get()));
          lastNodeIdxByJourneyStepIdx[journeyStepIdx]  = std::get<connectionIndexes::NODE_ARR>(*(reverseConnections[std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION >(journeyStep)].get()));

          // get in-between nodes for the journet step trip segment (boarding and unboarding excluded):
          for(int sequenceIdx = sequenceStartIdx + 1; sequenceIdx <= sequenceEndIdx; ++sequenceIdx)
          {
            int nodeDepIdx = std::get<connectionIndexes::NODE_DEP>(*(forwardConnections[ trips[tripIdx]->forwardConnectionsIdx[sequenceIdx] ]));
            if (nodeDepIdx != firstNodeIdxByJourneyStepIdx[journeyStepIdx] && nodeDepIdx != lastNodeIdxByJourneyStepIdx[journeyStepIdx]) // ignore repeated nodes at beginning or end
            {
              inBetweenNodesIdxByJourneyStepIdx[journeyStepIdx].push_back( std::get<connectionIndexes::NODE_DEP>(*(forwardConnections[ trips[tripIdx]->forwardConnectionsIdx[sequenceIdx] ])) );
            }
          }
          
          for (int i = 0; i < journeyStepIdx; i++)
          {
            
            std::vector<int>::iterator commonNodeI;

            // Search for CSL (1): Cut superfluous line:
            if (inBetweenNodesIdxByJourneyStepIdx[i].size() > 0)
            {
              commonNodeI = std::find(inBetweenNodesIdxByJourneyStepIdx[i].begin(), inBetweenNodesIdxByJourneyStepIdx[i].end(), lastNodeIdxByJourneyStepIdx[journeyStepIdx]);
              if (commonNodeI != inBetweenNodesIdxByJourneyStepIdx[i].end() && std::find(ignoreOptimizationNodesIdx.begin(), ignoreOptimizationNodesIdx.end(), lastNodeIdxByJourneyStepIdx[journeyStepIdx]) == ignoreOptimizationNodesIdx.end())
              {
                optimizationCase    = 1;
                optimizationNodeIdx = lastNodeIdxByJourneyStepIdx[journeyStepIdx];
                fromJourneyStepIdx  = i;
                toJourneyStepIdx    = journeyStepIdx;
                break;
              }
            }
            

            // Search for BTS (2): Boarded too soon:
            if (optimizationCase == -1 && inBetweenNodesIdxByJourneyStepIdx[journeyStepIdx].size() > 0)
            {
              commonNodeI = std::find(inBetweenNodesIdxByJourneyStepIdx[journeyStepIdx].begin(), inBetweenNodesIdxByJourneyStepIdx[journeyStepIdx].end(), lastNodeIdxByJourneyStepIdx[i]);
              if (commonNodeI != inBetweenNodesIdxByJourneyStepIdx[journeyStepIdx].end() && std::find(ignoreOptimizationNodesIdx.begin(), ignoreOptimizationNodesIdx.end(),  lastNodeIdxByJourneyStepIdx[i]) == ignoreOptimizationNodesIdx.end())
              {
                optimizationCase    = 2;
                optimizationNodeIdx = lastNodeIdxByJourneyStepIdx[i];
                fromJourneyStepIdx  = i;
                toJourneyStepIdx    = journeyStepIdx;
                break;
              }
            }

            // Search for GTF (3): Gone too far:
            if (optimizationCase == -1 && inBetweenNodesIdxByJourneyStepIdx[i].size() > 0)
            {
              commonNodeI = std::find(inBetweenNodesIdxByJourneyStepIdx[i].begin(), inBetweenNodesIdxByJourneyStepIdx[i].end(), firstNodeIdxByJourneyStepIdx[journeyStepIdx]);
              if (commonNodeI != inBetweenNodesIdxByJourneyStepIdx[i].end() && std::find(ignoreOptimizationNodesIdx.begin(), ignoreOptimizationNodesIdx.end(), firstNodeIdxByJourneyStepIdx[journeyStepIdx]) == ignoreOptimizationNodesIdx.end())
              {
                optimizationCase    = 3;
                optimizationNodeIdx = firstNodeIdxByJourneyStepIdx[journeyStepIdx];
                fromJourneyStepIdx  = i;
                toJourneyStepIdx    = journeyStepIdx;
                break;
              }
            }

            // Search for CSS (4): Cut superfluous segment:
            if (inBetweenNodesIdxByJourneyStepIdx[i].size() > 0 && inBetweenNodesIdxByJourneyStepIdx[journeyStepIdx].size() > 0)
            {
              std::vector<int> commonNodesIdx;
              for (int j = 0; j < inBetweenNodesIdxByJourneyStepIdx[i].size(); j++)
              {
                int nodeIdx = inBetweenNodesIdxByJourneyStepIdx[i][j];
                commonNodeI = std::find(inBetweenNodesIdxByJourneyStepIdx[journeyStepIdx].begin(), inBetweenNodesIdxByJourneyStepIdx[journeyStepIdx].end(), nodeIdx);
                if (commonNodeI != inBetweenNodesIdxByJourneyStepIdx[journeyStepIdx].end() && std::find(ignoreOptimizationNodesIdx.begin(), ignoreOptimizationNodesIdx.end(), nodeIdx) == ignoreOptimizationNodesIdx.end())
                {
                  optimizationCase    = 4;
                  optimizationNodeIdx = nodeIdx;
                  fromJourneyStepIdx  = i;
                  toJourneyStepIdx    = journeyStepIdx;
                  break;
                }
              }
              if (optimizationCase >= 0)
              {
                break;
              }
            }

          }
          //std::cerr << std::endl;

          if (optimizationCase >= 0) // no need to check further journey steps if we found an optimization node already
          {
            if (params.debugDisplay)
            {
              std::cerr << " Found optimization case: " << optimizationCase << " @node: " << nodes[optimizationNodeIdx]->name << "[" << nodes[optimizationNodeIdx]->code << "]" << std::endl;
            }
            break;
          }

        }

        ++journeyStepIdx;
      }


      // deal with optimization cases:
      if (optimizationCase == 1) // CSL
      {
        int tripIdx          = std::get<journeyStepIndexes::FINAL_TRIP>(journey[fromJourneyStepIdx]);
        int sequenceStartIdx = std::get<connectionIndexes::SEQUENCE>(*(reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[fromJourneyStepIdx])].get())) - 1;
        int sequenceEndIdx   = std::get<connectionIndexes::SEQUENCE>(*(reverseConnections[std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION >(journey[fromJourneyStepIdx])].get())) - 1;

        for(int sequenceIdx = trips[tripIdx]->reverseConnectionsIdx.size() - 1 - sequenceEndIdx; sequenceIdx <= trips[tripIdx]->reverseConnectionsIdx.size() - 1 - sequenceStartIdx; ++sequenceIdx)
        {
          int connectionIdx = trips[tripIdx]->reverseConnectionsIdx[sequenceIdx];
          
          if (optimizationNodeIdx == std::get<connectionIndexes::NODE_ARR>(*(reverseConnections[connectionIdx])))
          {
            if (std::get<connectionIndexes::CAN_UNBOARD>(*(reverseConnections[connectionIdx])) != 1)
            {
              ignoreOptimizationNodesIdx.push_back(optimizationNodeIdx);
              break;
            }
            else
            {
              usedOptimizationCases.push_back(1);
              if (toJourneyStepIdx - fromJourneyStepIdx == 1)
              {
                journey.erase(journey.begin() + toJourneyStepIdx);
              }
              else if (toJourneyStepIdx - fromJourneyStepIdx > 1) // could not split correctly...
              {
                journey.erase(journey.begin() + fromJourneyStepIdx + 1, journey.begin() + toJourneyStepIdx);
              }
              std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION>(journey[fromJourneyStepIdx]) = connectionIdx;
              break;
            }
          }
        }

      }

      else if (optimizationCase == 2) // BTS // untested
      {

        int tripIdx          = std::get<journeyStepIndexes::FINAL_TRIP>(journey[toJourneyStepIdx]);
        int sequenceStartIdx = std::get<connectionIndexes::SEQUENCE>(*(reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[toJourneyStepIdx])].get())) - 1;
        int sequenceEndIdx   = std::get<connectionIndexes::SEQUENCE>(*(reverseConnections[std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION >(journey[toJourneyStepIdx])].get())) - 1;
        for(int sequenceIdx = trips[tripIdx]->reverseConnectionsIdx.size() - 1 - sequenceEndIdx; sequenceIdx <= trips[tripIdx]->reverseConnectionsIdx.size() - 1 - sequenceStartIdx; ++sequenceIdx)
        {
          int connectionIdx = trips[tripIdx]->reverseConnectionsIdx[sequenceIdx];
          if (optimizationNodeIdx == std::get<connectionIndexes::NODE_DEP>(*(reverseConnections[connectionIdx])))
          {
            if (std::get<connectionIndexes::CAN_BOARD>(*(reverseConnections[connectionIdx])) != 1)
            {
              ignoreOptimizationNodesIdx.push_back(optimizationNodeIdx);
              break;
            }
            else
            {
              usedOptimizationCases.push_back(2);
              std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[toJourneyStepIdx]) = connectionIdx;
              std::get<journeyStepIndexes::TRANSFER_TRAVEL_TIME>(journey[toJourneyStepIdx])   = 0;
              std::get<journeyStepIndexes::TRANSFER_DISTANCE>(journey[toJourneyStepIdx])      = 0;
              break;
            }
          }
        }
        optimizationCase = -1;
      }

      else if (optimizationCase == 3) // GTF // untested
      {
        int tripIdx          = std::get<journeyStepIndexes::FINAL_TRIP>(journey[fromJourneyStepIdx]);
        int sequenceStartIdx = std::get<connectionIndexes::SEQUENCE>(*(reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[fromJourneyStepIdx])].get())) - 1;
        int sequenceEndIdx   = std::get<connectionIndexes::SEQUENCE>(*(reverseConnections[std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION >(journey[fromJourneyStepIdx])].get())) - 1;

        for(int sequenceIdx = trips[tripIdx]->reverseConnectionsIdx.size() - 1 - sequenceEndIdx; sequenceIdx <= trips[tripIdx]->reverseConnectionsIdx.size() - 1 - sequenceStartIdx; ++sequenceIdx)
        {
          int connectionIdx = trips[tripIdx]->reverseConnectionsIdx[sequenceIdx];
          
          if (optimizationNodeIdx == std::get<connectionIndexes::NODE_ARR>(*(reverseConnections[connectionIdx])))
          {
            if (std::get<connectionIndexes::CAN_UNBOARD>(*(reverseConnections[connectionIdx])) != 1)
            {
              ignoreOptimizationNodesIdx.push_back(optimizationNodeIdx);
              break;
            }
            else
            {
              usedOptimizationCases.push_back(3);
              std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION>(journey[fromJourneyStepIdx]) = connectionIdx;
              std::get<journeyStepIndexes::TRANSFER_TRAVEL_TIME>(journey[toJourneyStepIdx])    = 0;
              std::get<journeyStepIndexes::TRANSFER_DISTANCE>(journey[toJourneyStepIdx])       = 0;
              break;
            }
          }
        }
      }

      else if (optimizationCase == 4) // CSS
      {
        int arrivalJourneyStepTripIdx          = std::get<journeyStepIndexes::FINAL_TRIP>(journey[fromJourneyStepIdx]);
        int arrivalJourneyStepSequenceStartIdx = std::get<connectionIndexes::SEQUENCE>(*(reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[fromJourneyStepIdx])].get())) - 1;
        int arrivalJourneyStepSequenceEndIdx   = std::get<connectionIndexes::SEQUENCE>(*(reverseConnections[std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION >(journey[fromJourneyStepIdx])].get())) - 1;

        int departureJourneyStepTripIdx          = std::get<journeyStepIndexes::FINAL_TRIP>(journey[toJourneyStepIdx]);
        int departureJourneyStepSequenceStartIdx = std::get<connectionIndexes::SEQUENCE>(*(reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[toJourneyStepIdx])].get())) - 1;
        int departureJourneyStepSequenceEndIdx   = std::get<connectionIndexes::SEQUENCE>(*(reverseConnections[std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION >(journey[toJourneyStepIdx])].get())) - 1;

        int  exitConnectionIdx {-1};

        {
          for(int sequenceIdx = trips[arrivalJourneyStepTripIdx]->reverseConnectionsIdx.size() - 1 - arrivalJourneyStepSequenceEndIdx; sequenceIdx <= trips[arrivalJourneyStepTripIdx]->reverseConnectionsIdx.size() - 1 - arrivalJourneyStepSequenceStartIdx; ++sequenceIdx)
          {
            int connectionIdx = trips[arrivalJourneyStepTripIdx]->reverseConnectionsIdx[sequenceIdx];
            if (optimizationNodeIdx == std::get<connectionIndexes::NODE_ARR>(*(reverseConnections[ trips[arrivalJourneyStepTripIdx]->reverseConnectionsIdx[sequenceIdx] ])))
            {
              if (std::get<connectionIndexes::CAN_UNBOARD>(*(reverseConnections[ trips[arrivalJourneyStepTripIdx]->reverseConnectionsIdx[sequenceIdx] ])) == 1)
              {
                exitConnectionIdx = connectionIdx;
              }
              else
              {
                break;
              }
            }
          }

          for(int sequenceIdx = trips[departureJourneyStepTripIdx]->reverseConnectionsIdx.size() - 1 - departureJourneyStepSequenceEndIdx; sequenceIdx <= trips[departureJourneyStepTripIdx]->reverseConnectionsIdx.size() - 1 - departureJourneyStepSequenceStartIdx; ++sequenceIdx)
          {
            int connectionIdx = trips[departureJourneyStepTripIdx]->reverseConnectionsIdx[sequenceIdx];
            if (optimizationNodeIdx == std::get<connectionIndexes::NODE_DEP>(*(reverseConnections[ trips[departureJourneyStepTripIdx]->reverseConnectionsIdx[sequenceIdx] ])))
            {
              if (exitConnectionIdx >= 0 && std::get<connectionIndexes::CAN_BOARD>(*(reverseConnections[ trips[departureJourneyStepTripIdx]->reverseConnectionsIdx[sequenceIdx] ])) == 1)
              {
                usedOptimizationCases.push_back(4);
                std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION>(journey[fromJourneyStepIdx]) = exitConnectionIdx;
                std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[toJourneyStepIdx])  = connectionIdx;
              }
              else
              {
                ignoreOptimizationNodesIdx.push_back(optimizationNodeIdx);
                break;
              }
            }
          }
        }

      }

    }


    if (params.debugDisplay)
    {
      std::cerr << "-- remove superfluous segments -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
    }

    return usedOptimizationCases;


  }
}