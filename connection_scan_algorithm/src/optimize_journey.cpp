#include "spdlog/spdlog.h"

#include "calculator.hpp"
#include "trip.hpp"
#include "parameters.hpp"
#include "node.hpp"

namespace TrRouting
{
  
  std::vector<int> Calculator::optimizeJourney(std::deque<JourneyStep> &journey)
  {

    CalculationTime algorithmCalculationTime = CalculationTime();
    algorithmCalculationTime.start();
    long long       calculationTime;
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

    std::vector<int> usedOptimizationCases;

    //json["optimizedNodes"] = nlohmann::json::array();

    short optimizationCase    {-1}; // 1: CSL, 2: BTS, 3: GTF, 4: CSS | For optimization cases diagrams, see in the references directory
    bool  startedOptimization {false};
    std::vector<std::reference_wrapper<const Node>> ignoreOptimizationNodes;

    //TODO startedOptimization can be replaced with a do...while
    while(!startedOptimization || optimizationCase >= 0)
    {
      
      startedOptimization     = true;
      optimizationCase        = -1;
      int journeyStepIdx      { 0};
      int fromJourneyStepIdx  {-1};
      int toJourneyStepIdx    {-1};
      std::optional<std::reference_wrapper<const Node>> optimizationNode;

      std::vector<std::optional<std::reference_wrapper<const Node>>> lastNodeByJourneyStepIdx; // last node of each journey segment (unboarding node)
      std::vector<std::vector<std::reference_wrapper<const Node>>> inBetweenNodesByJourneyStepIdx; // all nodes between boarding and unboarding excluded for each journey segment
      //TODO lot of indexes to track in here, we should revisit this code and simplify
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
          auto enterConnect = *(reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journeyStep)]);

          const Node & firstNodeByJourneyStep  =  std::get<connectionIndexes::NODE_DEP>(enterConnect).get();

          auto exitConnect = *(reverseConnections[std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION >(journeyStep)]);

          lastNodeByJourneyStepIdx.push_back(std::get<connectionIndexes::NODE_ARR>(exitConnect).get());
          inBetweenNodesByJourneyStepIdx.resize(journeyStepIdx+1); // Resize outer vector so we can push_back in it later

          // get in-between nodes for the journet step trip segment (boarding and unboarding excluded):
          for(int sequenceIdx = sequenceStartIdx + 1; sequenceIdx <= sequenceEndIdx; ++sequenceIdx)
          {
            const Node & nodeDep = std::get<connectionIndexes::NODE_DEP>(*(forwardConnections[ trips[tripIdx]->forwardConnectionsIdx[sequenceIdx] ]));
            if (nodeDep.uuid != firstNodeByJourneyStep.uuid && nodeDep.uuid != lastNodeByJourneyStepIdx.at(journeyStepIdx).value().get().uuid) // ignore repeated nodes at beginning or end
            {
              inBetweenNodesByJourneyStepIdx[journeyStepIdx].push_back( std::get<connectionIndexes::NODE_DEP>(*(forwardConnections[ trips[tripIdx]->forwardConnectionsIdx[sequenceIdx] ])) );
            }
          }
          
          for (int i = 0; i < journeyStepIdx; i++)
          {
            
            // Search for CSL (1): Cut superfluous line:
            if (inBetweenNodesByJourneyStepIdx[i].size() > 0)
            {
              auto commonNodeI = std::find(inBetweenNodesByJourneyStepIdx[i].begin(), inBetweenNodesByJourneyStepIdx[i].end(), lastNodeByJourneyStepIdx.at(journeyStepIdx).value());
              if (commonNodeI != inBetweenNodesByJourneyStepIdx[i].end() && std::find(ignoreOptimizationNodes.begin(), ignoreOptimizationNodes.end(), lastNodeByJourneyStepIdx.at(journeyStepIdx).value()) == ignoreOptimizationNodes.end())
              {
                optimizationCase    = 1;
                optimizationNode = lastNodeByJourneyStepIdx.at(journeyStepIdx);
                fromJourneyStepIdx  = i;
                toJourneyStepIdx    = journeyStepIdx;
                break;
              }
            }
            

            // Search for BTS (2): Boarded too soon:
            if (optimizationCase == -1 && inBetweenNodesByJourneyStepIdx[journeyStepIdx].size() > 0)
            {
              if (lastNodeByJourneyStepIdx.at(i).has_value()) {
                auto commonNodeI = std::find(inBetweenNodesByJourneyStepIdx[journeyStepIdx].begin(), inBetweenNodesByJourneyStepIdx[journeyStepIdx].end(), lastNodeByJourneyStepIdx.at(i).value());
                if (commonNodeI != inBetweenNodesByJourneyStepIdx[journeyStepIdx].end() && std::find(ignoreOptimizationNodes.begin(), ignoreOptimizationNodes.end(),  lastNodeByJourneyStepIdx.at(i).value()) == ignoreOptimizationNodes.end())
                {
                  optimizationCase    = 2;
                  optimizationNode = lastNodeByJourneyStepIdx.at(i);
                  fromJourneyStepIdx  = i;
                  toJourneyStepIdx    = journeyStepIdx;
                  break;
                }
              }
            }

            // Search for GTF (3): Gone too far:
            if (optimizationCase == -1 && inBetweenNodesByJourneyStepIdx[i].size() > 0)
            {
              auto commonNodeI = std::find(inBetweenNodesByJourneyStepIdx[i].begin(), inBetweenNodesByJourneyStepIdx[i].end(), firstNodeByJourneyStep);
              if (commonNodeI != inBetweenNodesByJourneyStepIdx[i].end() && std::find(ignoreOptimizationNodes.begin(), ignoreOptimizationNodes.end(), firstNodeByJourneyStep) == ignoreOptimizationNodes.end())
              {
                optimizationCase    = 3;
                optimizationNode = firstNodeByJourneyStep;
                fromJourneyStepIdx  = i;
                toJourneyStepIdx    = journeyStepIdx;
                break;
              }
            }

            // Search for CSS (4): Cut superfluous segment:
            if (inBetweenNodesByJourneyStepIdx[i].size() > 0 && inBetweenNodesByJourneyStepIdx[journeyStepIdx].size() > 0)
            {
              std::vector<int> commonNodes;
              for (int j = 0; j < inBetweenNodesByJourneyStepIdx[i].size(); j++)
              {
                const Node & node = inBetweenNodesByJourneyStepIdx[i][j];
                auto commonNodeI = std::find(inBetweenNodesByJourneyStepIdx[journeyStepIdx].begin(), inBetweenNodesByJourneyStepIdx[journeyStepIdx].end(), node);
                if (commonNodeI != inBetweenNodesByJourneyStepIdx[journeyStepIdx].end() && std::find(ignoreOptimizationNodes.begin(), ignoreOptimizationNodes.end(), node) == ignoreOptimizationNodes.end())
                {
                  optimizationCase    = 4;
                  optimizationNode = node;
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

          if (optimizationCase >= 0) // no need to check further journey steps if we found an optimization node already
          {
            spdlog::debug(" Found optimization case: {} @node: {} [{}]", optimizationCase, optimizationNode.value().get().name, optimizationNode.value().get().code);
            
            break;
          }

        } else {
          lastNodeByJourneyStepIdx.push_back(std::nullopt);
          inBetweenNodesByJourneyStepIdx.resize(journeyStepIdx+1);
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
          
          if (optimizationNode == std::get<connectionIndexes::NODE_ARR>(*(reverseConnections[connectionIdx])))
          {
            if (std::get<connectionIndexes::CAN_UNBOARD>(*(reverseConnections[connectionIdx])) != 1)
            {
              ignoreOptimizationNodes.push_back(optimizationNode.value());
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
          if (optimizationNode == std::get<connectionIndexes::NODE_DEP>(*(reverseConnections[connectionIdx])))
          {
            if (std::get<connectionIndexes::CAN_BOARD>(*(reverseConnections[connectionIdx])) != 1)
            {
              ignoreOptimizationNodes.push_back(optimizationNode.value());
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
          
          if (optimizationNode == std::get<connectionIndexes::NODE_ARR>(*(reverseConnections[connectionIdx])))
          {
            if (std::get<connectionIndexes::CAN_UNBOARD>(*(reverseConnections[connectionIdx])) != 1)
            {
              ignoreOptimizationNodes.push_back(optimizationNode.value());
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
            if (optimizationNode == std::get<connectionIndexes::NODE_ARR>(*(reverseConnections[ trips[arrivalJourneyStepTripIdx]->reverseConnectionsIdx[sequenceIdx] ])))
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
            if (optimizationNode == std::get<connectionIndexes::NODE_DEP>(*(reverseConnections[ trips[departureJourneyStepTripIdx]->reverseConnectionsIdx[sequenceIdx] ])))
            {
              if (exitConnectionIdx >= 0 && std::get<connectionIndexes::CAN_BOARD>(*(reverseConnections[ trips[departureJourneyStepTripIdx]->reverseConnectionsIdx[sequenceIdx] ])) == 1)
              {
                usedOptimizationCases.push_back(4);
                std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION>(journey[fromJourneyStepIdx]) = exitConnectionIdx;
                std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[toJourneyStepIdx])  = connectionIdx;
              }
              else
              {
                ignoreOptimizationNodes.push_back(optimizationNode.value());
                break;
              }
            }
          }
        }

      }

    }


    spdlog::debug("-- remove superfluous segments -- {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);    

    return usedOptimizationCases;


  }
}
