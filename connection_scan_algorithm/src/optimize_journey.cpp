#include "spdlog/spdlog.h"

#include "calculator.hpp"
#include "trip.hpp"
#include "parameters.hpp"
#include "node.hpp"
#include "transit_data.hpp"

namespace TrRouting
{
  
  std::vector<int> Calculator::optimizeJourney(std::deque<JourneyStep> &journey)
  {

    CalculationTime optimizeJourneyCalculationTime = CalculationTime();
    optimizeJourneyCalculationTime.start();
    long long       optimizeJourneyStartCalculationTime;
    optimizeJourneyStartCalculationTime = optimizeJourneyCalculationTime.getDurationMicrosecondsNoStop();

    std::vector<int> usedOptimizationCases;

    //json["optimizedNodes"] = nlohmann::json::array();

    short optimizationCase    {-1}; // 1: CSL, 2: BTS, 3: GTF, 4: CSS | For optimization cases diagrams, see in the references directory
    bool  startedOptimization {false};
    std::vector<std::reference_wrapper<const Node>> ignoreOptimizationNodes;

    // Get a local reference to not have to call the function everywhere
    // TODO lot of these use case could go away if we don't refer to connect by index only
    auto & forwardConnections = transitData.getForwardConnections();
    auto & reverseConnections = transitData.getReverseConnections();
    
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
          const Trip & trip  = std::get<journeyStepIndexes::FINAL_TRIP>(journeyStep).value();
          int sequenceStartIdx = (*(reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journeyStep)].get())).getSequenceInTrip() - 1;
          int sequenceEndIdx   = (*(reverseConnections[std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION >(journeyStep)].get())).getSequenceInTrip() - 1;

          // get first and last nodes for the journey step trip segment (boarding and unboarding nodes):
          auto enterConnect = *(reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journeyStep)]);

          //TODO Double check this, unsure what type is enterConnect
          const Node & firstNodeByJourneyStep  =  enterConnect.getDepartureNode();

          auto exitConnect = *(reverseConnections[std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION >(journeyStep)]);

          //TODO Double check this, unsure what type is exitConnect
          lastNodeByJourneyStepIdx.push_back(exitConnect.getArrivalNode());
          inBetweenNodesByJourneyStepIdx.resize(journeyStepIdx+1); // Resize outer vector so we can push_back in it later

          // get in-between nodes for the journet step trip segment (boarding and unboarding excluded):
          for(int sequenceIdx = sequenceStartIdx + 1; sequenceIdx <= sequenceEndIdx; ++sequenceIdx)
          {
            const Node & nodeDep = (*(forwardConnections[ trip.forwardConnectionsIdx[sequenceIdx] ])).getDepartureNode();
            if (nodeDep.uuid != firstNodeByJourneyStep.uuid && nodeDep.uuid != lastNodeByJourneyStepIdx.at(journeyStepIdx).value().get().uuid) // ignore repeated nodes at beginning or end
            {
              inBetweenNodesByJourneyStepIdx[journeyStepIdx].push_back( (*(forwardConnections[ trip.forwardConnectionsIdx[sequenceIdx] ])).getDepartureNode() );
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
              for (size_t j = 0; j < inBetweenNodesByJourneyStepIdx[i].size(); j++)
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
        //TODO We might need to check if the optional have a value
        const Trip & trip = std::get<journeyStepIndexes::FINAL_TRIP>(journey[fromJourneyStepIdx]).value();
        int sequenceStartIdx = (*(reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[fromJourneyStepIdx])].get())).getSequenceInTrip() - 1;
        int sequenceEndIdx   = (*(reverseConnections[std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION >(journey[fromJourneyStepIdx])].get())).getSequenceInTrip() - 1;

        assert(trip.reverseConnectionsIdx.size() > 1 + sequenceEndIdx); // make sure sequenceIdx will be valid
        for(size_t sequenceIdx = trip.reverseConnectionsIdx.size() - 1 - sequenceEndIdx; sequenceIdx <= trip.reverseConnectionsIdx.size() - 1 - sequenceStartIdx; ++sequenceIdx)
        {
          int connectionIdx = trip.reverseConnectionsIdx[sequenceIdx];
          
          if (optimizationNode == (*(reverseConnections[connectionIdx])).getArrivalNode())
          {
            if (!(reverseConnections[connectionIdx])->canUnboard())
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

        const Trip & trip = std::get<journeyStepIndexes::FINAL_TRIP>(journey[toJourneyStepIdx]).value();
        int sequenceStartIdx = (*(reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[toJourneyStepIdx])].get())).getSequenceInTrip() - 1;
        int sequenceEndIdx   = (*(reverseConnections[std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION >(journey[toJourneyStepIdx])].get())).getSequenceInTrip() - 1;
        for(size_t sequenceIdx = trip.reverseConnectionsIdx.size() - 1 - sequenceEndIdx; sequenceIdx <= trip.reverseConnectionsIdx.size() - 1 - sequenceStartIdx; ++sequenceIdx)
        {
          int connectionIdx = trip.reverseConnectionsIdx[sequenceIdx];
          if (optimizationNode == (*(reverseConnections[connectionIdx])).getDepartureNode())
          {
            if (!(reverseConnections[connectionIdx])->canBoard())
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
        const Trip & trip = std::get<journeyStepIndexes::FINAL_TRIP>(journey[fromJourneyStepIdx]).value();
        int sequenceStartIdx = (*(reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[fromJourneyStepIdx])].get())).getSequenceInTrip() - 1;
        int sequenceEndIdx   = (*(reverseConnections[std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION >(journey[fromJourneyStepIdx])].get())).getSequenceInTrip() - 1;

        for(size_t sequenceIdx = trip.reverseConnectionsIdx.size() - 1 - sequenceEndIdx; sequenceIdx <= trip.reverseConnectionsIdx.size() - 1 - sequenceStartIdx; ++sequenceIdx)
        {
          int connectionIdx = trip.reverseConnectionsIdx[sequenceIdx];
          
          if (optimizationNode == (*(reverseConnections[connectionIdx])).getArrivalNode())
          {
            if (!(reverseConnections[connectionIdx])->canUnboard())
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
        const Trip & arrivalJourneyStepTrip          = std::get<journeyStepIndexes::FINAL_TRIP>(journey[fromJourneyStepIdx]).value();
        int arrivalJourneyStepSequenceStartIdx = (*(reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[fromJourneyStepIdx])].get())).getSequenceInTrip() - 1;
        int arrivalJourneyStepSequenceEndIdx   = (*(reverseConnections[std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION >(journey[fromJourneyStepIdx])].get())).getSequenceInTrip() - 1;

        const Trip & departureJourneyStepTrip          = std::get<journeyStepIndexes::FINAL_TRIP>(journey[toJourneyStepIdx]).value();
        int departureJourneyStepSequenceStartIdx = (*(reverseConnections[std::get<journeyStepIndexes::FINAL_ENTER_CONNECTION>(journey[toJourneyStepIdx])].get())).getSequenceInTrip() - 1;
        int departureJourneyStepSequenceEndIdx   = (*(reverseConnections[std::get<journeyStepIndexes::FINAL_EXIT_CONNECTION >(journey[toJourneyStepIdx])].get())).getSequenceInTrip() - 1;

        int  exitConnectionIdx {-1};

        {
          for(size_t sequenceIdx = arrivalJourneyStepTrip.reverseConnectionsIdx.size() - 1 - arrivalJourneyStepSequenceEndIdx; sequenceIdx <= arrivalJourneyStepTrip.reverseConnectionsIdx.size() - 1 - arrivalJourneyStepSequenceStartIdx; ++sequenceIdx)
          {
            int connectionIdx = arrivalJourneyStepTrip.reverseConnectionsIdx[sequenceIdx];
            if (optimizationNode == (*(reverseConnections[ arrivalJourneyStepTrip.reverseConnectionsIdx[sequenceIdx] ])).getArrivalNode())
            {
              if ((*(reverseConnections[ arrivalJourneyStepTrip.reverseConnectionsIdx[sequenceIdx] ])).canUnboard())
              {
                exitConnectionIdx = connectionIdx;
              }
              else
              {
                break;
              }
            }
          }

          for(size_t sequenceIdx = departureJourneyStepTrip.reverseConnectionsIdx.size() - 1 - departureJourneyStepSequenceEndIdx; sequenceIdx <= departureJourneyStepTrip.reverseConnectionsIdx.size() - 1 - departureJourneyStepSequenceStartIdx; ++sequenceIdx)
          {
            int connectionIdx = departureJourneyStepTrip.reverseConnectionsIdx[sequenceIdx];
            if (optimizationNode == (*(reverseConnections[ departureJourneyStepTrip.reverseConnectionsIdx[sequenceIdx] ])).getDepartureNode())
            {
              if (exitConnectionIdx >= 0 && (reverseConnections[ departureJourneyStepTrip.reverseConnectionsIdx[sequenceIdx] ])->canBoard())
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


    spdlog::debug("-- remove superfluous segments -- {} microseconds", optimizeJourneyCalculationTime.getDurationMicrosecondsNoStop() - optimizeJourneyStartCalculationTime);    

    return usedOptimizationCases;


  }
}
