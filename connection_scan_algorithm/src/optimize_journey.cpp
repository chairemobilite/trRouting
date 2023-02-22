#include "spdlog/spdlog.h"

#include "calculator.hpp"
#include "trip.hpp"
#include "parameters.hpp"
#include "node.hpp"
#include "transit_data.hpp"

namespace TrRouting
{
  
  std::string Calculator::optimizeCasesToString(const std::vector<int> optimizeCases) {

    std::string optimizeCasesStr = "";

    for (int optimizeCase : optimizeCases) {
      if (optimizeCase == 1) {
        optimizeCasesStr += "CSL|";
      } else if (optimizeCase == 2) {
        optimizeCasesStr += "BTS|";
      } else if (optimizeCase == 3) {
        optimizeCasesStr += "GTF|";
      } else if (optimizeCase == 4) {
        optimizeCasesStr += "CSS|";
      }
    }

    // Remove last |
    if (optimizeCasesStr.size() > 0) {
      optimizeCasesStr.pop_back();
    }
    return optimizeCasesStr;
  }

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
            journeyStep.getFinalTrip().has_value()
            && journeyStep.hasConnections()
        )
        {
          const Trip & trip  = journeyStep.getFinalTrip().value().get();
          int sequenceStartIdx = journeyStep.getFinalEnterConnection().value().get().getSequenceInTrip() - 1;
          int sequenceEndIdx   = journeyStep.getFinalExitConnection().value().get().getSequenceInTrip() - 1;

          // get first and last nodes for the journey step trip segment (boarding and unboarding nodes):
          auto enterConnect = journeyStep.getFinalEnterConnection().value().get();

          //TODO Double check this, unsure what type is enterConnect
          const Node & firstNodeByJourneyStep  =  enterConnect.getDepartureNode();

          auto exitConnect = journeyStep.getFinalExitConnection().value().get();

          //TODO Double check this, unsure what type is exitConnect
          lastNodeByJourneyStepIdx.push_back(exitConnect.getArrivalNode());
          inBetweenNodesByJourneyStepIdx.resize(journeyStepIdx+1); // Resize outer vector so we can push_back in it later

          // get in-between nodes for the journet step trip segment (boarding and unboarding excluded):
          for(int sequenceIdx = sequenceStartIdx + 1; sequenceIdx <= sequenceEndIdx; ++sequenceIdx)
          {
            const Node & nodeDep = trip.forwardConnections[sequenceIdx]->getDepartureNode();
            if (nodeDep.uuid != firstNodeByJourneyStep.uuid && nodeDep.uuid != lastNodeByJourneyStepIdx.at(journeyStepIdx).value().get().uuid) // ignore repeated nodes at beginning or end
            {
              inBetweenNodesByJourneyStepIdx[journeyStepIdx].push_back( trip.forwardConnections[sequenceIdx]->getDepartureNode() );
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
        const Trip & trip = journey[fromJourneyStepIdx].getFinalTrip().value().get();
        int sequenceStartIdx = journey[fromJourneyStepIdx].getFinalEnterConnection().value().get().getSequenceInTrip() - 1;
        int sequenceEndIdx   = journey[fromJourneyStepIdx].getFinalExitConnection().value().get().getSequenceInTrip() - 1;

        // Editorial comment: There's lot of +1/-1 in this code. This suggest that we have an array index that start at 1 instead of zero. This need confirmation
        assert(trip.reverseConnections.size() >= 1 + sequenceEndIdx); // make sure sequenceIdx will be valid
        for(size_t sequenceIdx = trip.reverseConnections.size() - 1 - sequenceEndIdx; sequenceIdx <= trip.reverseConnections.size() - 1 - sequenceStartIdx; ++sequenceIdx)
        {
          auto connection = trip.reverseConnections[sequenceIdx];
          
          if (optimizationNode.value() == connection->getArrivalNode())
          {
            if (!connection->canUnboard())
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
              journey[fromJourneyStepIdx].setFinalExitConnection(*connection);

              break;
            }
          }
        }

      }

      else if (optimizationCase == 2) // BTS // untested
      {

        const Trip & trip = journey[toJourneyStepIdx].getFinalTrip().value().get();
        int sequenceStartIdx = journey[toJourneyStepIdx].getFinalEnterConnection().value().get().getSequenceInTrip() - 1;
        int sequenceEndIdx   = journey[toJourneyStepIdx].getFinalExitConnection().value().get().getSequenceInTrip() - 1;
        for(size_t sequenceIdx = trip.reverseConnections.size() - 1 - sequenceEndIdx; sequenceIdx <= trip.reverseConnections.size() - 1 - sequenceStartIdx; ++sequenceIdx)
        {
          auto connection = trip.reverseConnections[sequenceIdx];
          if (optimizationNode.value() == connection->getDepartureNode())
          {
            if (!connection->canBoard())
            {
              ignoreOptimizationNodes.push_back(optimizationNode.value());
              break;
            }
            else
            {
              usedOptimizationCases.push_back(2);
              journey[toJourneyStepIdx].setFinalEnterConnection(*connection);
              journey[toJourneyStepIdx].setTransferTimeDistance(0,0);
              break;
            }
          }
        }
        optimizationCase = -1;
      }

      else if (optimizationCase == 3) // GTF // untested
      {
        const Trip & trip = journey[fromJourneyStepIdx].getFinalTrip().value().get();
        int sequenceStartIdx = journey[fromJourneyStepIdx].getFinalEnterConnection().value().get().getSequenceInTrip() - 1;
        int sequenceEndIdx   = journey[fromJourneyStepIdx].getFinalExitConnection().value().get().getSequenceInTrip() - 1;

        for(size_t sequenceIdx = trip.reverseConnections.size() - 1 - sequenceEndIdx; sequenceIdx <= trip.reverseConnections.size() - 1 - sequenceStartIdx; ++sequenceIdx)
        {
          auto connection = trip.reverseConnections[sequenceIdx];
          
          if (optimizationNode.value() == connection->getArrivalNode())
          {
            if (!connection->canUnboard())
            {
              ignoreOptimizationNodes.push_back(optimizationNode.value());
              break;
            }
            else
            {
              usedOptimizationCases.push_back(3);
              journey[fromJourneyStepIdx].setFinalExitConnection(*connection);
              journey[toJourneyStepIdx].setTransferTimeDistance(0,0);
              break;
            }
          }
        }
      }

      else if (optimizationCase == 4) // CSS
      {
        const Trip & arrivalJourneyStepTrip          = journey[fromJourneyStepIdx].getFinalTrip().value().get();
        int arrivalJourneyStepSequenceStartIdx = journey[fromJourneyStepIdx].getFinalEnterConnection().value().get().getSequenceInTrip() - 1;
        int arrivalJourneyStepSequenceEndIdx   = journey[fromJourneyStepIdx].getFinalExitConnection().value().get().getSequenceInTrip() - 1;

        const Trip & departureJourneyStepTrip          = journey[toJourneyStepIdx].getFinalTrip().value().get();
        int departureJourneyStepSequenceStartIdx = journey[toJourneyStepIdx].getFinalEnterConnection().value().get().getSequenceInTrip() - 1;
        int departureJourneyStepSequenceEndIdx   = journey[toJourneyStepIdx].getFinalExitConnection().value().get().getSequenceInTrip() - 1;

        std::optional<std::reference_wrapper<Connection>> exitConnection;

        {
          for(size_t sequenceIdx = arrivalJourneyStepTrip.reverseConnections.size() - 1 - arrivalJourneyStepSequenceEndIdx; sequenceIdx <= arrivalJourneyStepTrip.reverseConnections.size() - 1 - arrivalJourneyStepSequenceStartIdx; ++sequenceIdx)
          {
            auto connection = arrivalJourneyStepTrip.reverseConnections[sequenceIdx];
            if (optimizationNode.value() == connection->getArrivalNode())
            {
              if (connection->canUnboard())
              {
                exitConnection = *connection;
              }
              else
              {
                break;
              }
            }
          }

          for(size_t sequenceIdx = departureJourneyStepTrip.reverseConnections.size() - 1 - departureJourneyStepSequenceEndIdx; sequenceIdx <= departureJourneyStepTrip.reverseConnections.size() - 1 - departureJourneyStepSequenceStartIdx; ++sequenceIdx)
          {
            auto connection = departureJourneyStepTrip.reverseConnections[sequenceIdx];
            if (optimizationNode.value() == connection->getDepartureNode())
            {
              if (exitConnection.has_value() && connection->canBoard())
              {
                usedOptimizationCases.push_back(4);
                journey[fromJourneyStepIdx].setFinalExitConnection(exitConnection.value());
                journey[toJourneyStepIdx].setFinalEnterConnection(*connection);
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
