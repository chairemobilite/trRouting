#include <sstream>
#include "spdlog/spdlog.h"

#include "calculator.hpp"
#include "constants.hpp"
#include "routing_result.hpp"
#include "line.hpp"
#include "parameters.hpp"
#include "combinations.hpp"
#include "routing_result.hpp"
#include "point.hpp"

namespace TrRouting
{


  std::string LineIdxToString(const std::vector<int> & linesList, const std::vector<std::unique_ptr<Line>> &lines) {
    std::stringstream formatter;

    for(auto lineIdx: linesList) {
      formatter << lines[lineIdx].get()->shortname << " ";
    }
    return formatter.str();
  }
  
  /**
   * @brief Visitor class to get the line ids used by a step
   * 
   * TODO See TODO of LineIdxVisitor, when concrete classes are returned, the visitor approach for line id should be re...visited!
   */
  class LineIdxStepVisitor : public StepVisitor<int> {
  private:
    int lineIdx;
    std::map<boost::uuids::uuid, int>& lineIndexesByUuid;
  public:
    LineIdxStepVisitor(std::map<boost::uuids::uuid, int>& _lineIndexesByUuid):
      lineIdx(-1),
      lineIndexesByUuid(_lineIndexesByUuid) {}
    void visitBoardingStep(const BoardingStep& step) override {
      lineIdx = lineIndexesByUuid[step.lineUuid];
    }
    void visitUnboardingStep(const UnboardingStep& step) override {
      lineIdx = -1;
    }
    void visitWalkingStep(const WalkingStep& step) override {
      lineIdx = -1;
    }
    int getResult() override {
      return lineIdx;
    }
  };

  /** 
   * @brief Visitor class to get the line ids used by a result object
   * 
   * TODO when allNodes and single calculation are split in the calculation method, we can use concrete 
   * return types instead. When that is possible, consider adding a method in SingleCalculationResult 
   * instead of this visitor.
   * */
  class LineIdxVisitor : public ResultVisitor<std::vector<int>> {
  private:
    std::vector<int> linesIdx;
    LineIdxStepVisitor stepVisitor;
  public:
    LineIdxVisitor(std::map<boost::uuids::uuid, int>& _lineIndexesByUuid): stepVisitor(LineIdxStepVisitor(_lineIndexesByUuid)) {}
    std::vector<int> getResult() override {
      return linesIdx;
    }
    void visitSingleCalculationResult(const SingleCalculationResult& result) override {
      for (auto const& step : result.steps) {
        int stepLineIdx = step.get()->accept(stepVisitor);
        if (stepLineIdx >= 0) {
          linesIdx.push_back(stepLineIdx);
        }
      }
    };
    void visitAlternativesResult(const AlternativesResult& result) override {
      // Nothing to do for this result
    }
    void visitAllNodesResult(const AllNodesResult& result) override {
      // Nothing to do for this result
    }
  };

  AlternativesResult Calculator::alternativesRouting(RouteParameters &parameters)
  {

    std::vector<int>                 foundLinesIdx;
    std::vector<int>                 exceptLinesIdxFromParameters = *parameters.getExceptLinesIdx(); // make a copy of lines that are already disabled in parameters
    std::vector< std::vector<int> >  allCombinations;
    std::vector< std::vector<int> >  failedCombinations;
    bool                             combinationMatchesWithFailed {false};
    bool                             combinationMatchesWithAtLeastOneFailed {false};
    std::map<std::vector<int>, bool> alreadyCalculatedCombinations;
    std::map<std::vector<int>, bool> alreadyFoundLinesIdx;
    std::map<std::vector<int>, int>  foundLinesIdxTravelTimeSeconds;
    std::vector<int>                 combinationsKs;
    int maxTravelTime;
    int alternativeSequence = 1;
    int alternativesCalculatedCount = 1;
    int maxAlternatives = params.maxAlternatives;
    int lastFoundedAtNum = 0;
    //int departureTimeSeconds = -1;
    std::vector<std::string> lineShortnames;

    spdlog::debug("alternatives parameters:");
    spdlog::debug("  maxTotalTravelTimeSeconds: {}", parameters.getMaxTotalTravelTimeSeconds());
    spdlog::debug("  minAlternativeMaxTravelTimeSeconds: ", params.minAlternativeMaxTravelTimeSeconds);
    spdlog::debug("  alternativesMaxAddedTravelTimeSeconds: ", params.alternativesMaxAddedTravelTimeSeconds);
    spdlog::debug("  alternativesMaxTravelTimeRatio: ", params.alternativesMaxTravelTimeRatio);
    spdlog::debug("calculating fastest alternative...");
  
    std::unique_ptr<RoutingResult> result = calculate(parameters);

    // Can technically be allNodes or SingleCalculation, so we check the type
    if (result.get()->resType == result_type::SINGLE_CALCULATION)
    {
      SingleCalculationResult& routingResult = dynamic_cast<SingleCalculationResult&>(*result.get());
      AlternativesResult alternatives = AlternativesResult();

      alternatives.alternatives.push_back(std::move(result));

      alternativeSequence++;
      alternativesCalculatedCount++;

      LineIdxVisitor visitor = LineIdxVisitor(lineIndexesByUuid);

      //departureTimeSeconds = routingResult.departureTimeSeconds + routingResult.firstWaitingTimeSeconds - params.minWaitingTimeSeconds;

      maxTravelTime = params.alternativesMaxTravelTimeRatio * routingResult.totalTravelTime + (parameters.isForwardCalculation() ? routingResult.departureTime - parameters.getTimeOfTrip() : 0);
      if (maxTravelTime < params.minAlternativeMaxTravelTimeSeconds)
      {
        maxTravelTime = params.minAlternativeMaxTravelTimeSeconds;
      }
      else if (maxTravelTime > routingResult.totalTravelTime + params.alternativesMaxAddedTravelTimeSeconds)
      {
        maxTravelTime = routingResult.totalTravelTime + params.alternativesMaxAddedTravelTimeSeconds;
      }
      // TODO: We should not create a whole new object just to update maxTravelTime. This parameter should be in the calculation specific parameters, which do not exist yet
      Point* origin = parameters.getOrigin();
      Point* dest = parameters.getDestination();
      RouteParameters alternativeParameters = RouteParameters(std::make_unique<Point>(origin->latitude, origin->longitude),
        std::make_unique<Point>(dest->latitude, dest->longitude),
        parameters.getScenario(),
        parameters.getTimeOfTrip(),
        parameters.getMinWaitingTimeSeconds(),
        maxTravelTime,
        parameters.getMaxAccessWalkingTravelTimeSeconds(),
        parameters.getMaxEgressWalkingTravelTimeSeconds(),
        parameters.getMaxTransferWalkingTravelTimeSeconds(),
        parameters.getMaxFirstWaitingTimeSeconds(),
        parameters.isWithAlternatives(),
        parameters.isForwardCalculation());

      //params.departureTimeSeconds = departureTimeSeconds;

      spdlog::debug("  fastestTravelTimeSeconds: ", routingResult.totalTravelTime);

      foundLinesIdx = routingResult.accept(visitor);
      std::stable_sort(foundLinesIdx.begin(),foundLinesIdx.end());
      alreadyFoundLinesIdx[foundLinesIdx]           = true;
      foundLinesIdxTravelTimeSeconds[foundLinesIdx] = routingResult.totalTravelTime;
      lastFoundedAtNum = 1;

      spdlog::debug("fastest line ids: {}", LineIdxToString(foundLinesIdx, lines));


      combinationsKs.clear();
      for (int i = 1; i <= foundLinesIdx.size(); i++) { combinationsKs.push_back(i); }
      for (auto k : combinationsKs)
      {
        Combinations<int> combinations(foundLinesIdx, k);

        for (auto newCombination : combinations)
        {
          std::stable_sort(newCombination.begin(), newCombination.end());
          allCombinations.push_back(newCombination);
          alreadyCalculatedCombinations[newCombination] = true;
        }
      }

      std::vector<int> combination;
      for (int i = 0; i < allCombinations.size(); i++)
      {
        if (alternativesCalculatedCount < maxAlternatives && alternativeSequence - 1 < params.maxValidAlternatives)
        {

          combination = allCombinations.at(i);
          // TODO: The exception should be part of the calculation specific parameters, which do not exist yet
          alternativeParameters.exceptLinesIdx = exceptLinesIdxFromParameters; // reset except lines using parameters
          for (auto lineIdx : combination)
          {
            alternativeParameters.exceptLinesIdx.push_back(lineIdx);
          }

          spdlog::info("calculating alternative {} from a total of {} ...", alternativeSequence, alternativesCalculatedCount);
          
          spdlog::debug("except lines: {}", LineIdxToString(combination, lines));

          try {
            result = calculate(alternativeParameters, false, true);

            if (result.get()->resType == result_type::SINGLE_CALCULATION)
            {
              SingleCalculationResult& alternativeCalcResult = dynamic_cast<SingleCalculationResult&>(*result.get());
              LineIdxVisitor alternativeVisitor = LineIdxVisitor(lineIndexesByUuid);
              foundLinesIdx = alternativeCalcResult.accept(alternativeVisitor);
              std::stable_sort(foundLinesIdx.begin(), foundLinesIdx.end());

              if (foundLinesIdx.size() > 0 && alreadyFoundLinesIdx.count(foundLinesIdx) == 0)
              {
                lineShortnames.clear();
                for(auto lineIdx : foundLinesIdx)
                {
                  lineShortnames.push_back(lines[lineIdx].get()->shortname);
                }

                alternatives.alternatives.push_back(std::move(result));

                spdlog::debug("travelTimeSeconds: {}  line Uuids: {}",
                              alternativeCalcResult.totalTravelTime,
                              LineIdxToString(foundLinesIdx, lines));
                
                combinationsKs.clear();

                lastFoundedAtNum = alternativesCalculatedCount;
                alreadyFoundLinesIdx[foundLinesIdx] = true;
                foundLinesIdxTravelTimeSeconds[foundLinesIdx] = alternativeCalcResult.totalTravelTime;
                for (int i = 1; i <= foundLinesIdx.size(); i++) { combinationsKs.push_back(i); }
                for (auto k : combinationsKs)
                {
                  Combinations<int> combinations(foundLinesIdx, k);

                  for (auto newCombination : combinations)
                  {

                    newCombination.insert( newCombination.end(), combination.begin(), combination.end() );
                    std::stable_sort(newCombination.begin(), newCombination.end());
                    if (alreadyCalculatedCombinations.count(newCombination) == 0)
                    {
                      combinationMatchesWithAtLeastOneFailed = false;
                      for (auto failedCombination : failedCombinations)
                      {
                        combinationMatchesWithFailed = true;
                        for (int failedLineIdx : failedCombination)
                        {
                          if (std::find(newCombination.begin(), newCombination.end(), failedLineIdx) == newCombination.end())
                          {
                            combinationMatchesWithFailed = false;
                            break;
                          }
                        }
                        if (combinationMatchesWithFailed)
                        {
                          combinationMatchesWithAtLeastOneFailed = true;
                          break;
                        }
                      }
                      if (!combinationMatchesWithAtLeastOneFailed)
                      {
                        allCombinations.push_back(newCombination);
                      }
                      alreadyCalculatedCombinations[newCombination] = true;
                    }
                  }
                }

                combination.clear();

                alternativeSequence++;

              }
            }
          } catch (NoRoutingFoundException& e) {

            failedCombinations.push_back(combination);
          }

          alternativesCalculatedCount++;

          if (failedCombinations.size() > 0)
          {
            for (auto failedCombination : failedCombinations)
            {
              spdlog::debug("failed combinations: {}", LineIdxToString(failedCombination, lines));
            }
          }
        }
      }

      int i {0};
      for (auto foundLinesIdx : alreadyFoundLinesIdx)
      {          
        spdlog::debug("{}. {} tt: ", i, LineIdxToString(foundLinesIdx.first, lines),
                      (foundLinesIdxTravelTimeSeconds[foundLinesIdx.first] / 60));
        i++;          
      }
      
      spdlog::debug("last alternative found at: {} on a total of {} calculations", lastFoundedAtNum,  maxAlternatives);

      alternatives.totalAlternativesCalculated = alternativesCalculatedCount;
      return alternatives;

    }
    throw NoRoutingFoundException(NoRoutingReason::NO_ROUTING_FOUND);

  }



}
