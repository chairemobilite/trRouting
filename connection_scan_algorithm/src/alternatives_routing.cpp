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

namespace {
  // Placed in anynymous namespace so it's local to this file
  std::string LinesToString(const std::vector<std::reference_wrapper<const TrRouting::Line>> & linesList) {
    std::stringstream formatter;

    for(auto line: linesList) {
      formatter << line.get().shortname << " ";
    }
    return formatter.str();
  }
}

namespace TrRouting
{
  /**
   * @brief Visitor class to get the line ids used by a step
   * 
   * TODO See TODO of LineVisitor, when concrete classes are returned, the visitor approach for line id should be re...visited!
   */
  class LineStepVisitor : public StepVisitor<std::optional<std::reference_wrapper<const Line>>> {
  private:
    std::optional<std::reference_wrapper<const Line>> stepLine;
    const std::map<boost::uuids::uuid, Line>& lines;
  public:
    LineStepVisitor(const std::map<boost::uuids::uuid, Line>& _lines):
      lines(_lines) {}
    void visitBoardingStep(const BoardingStep& step) override {
      spdlog::debug("Step Visitor line {}", boost::uuids::to_string(step.lineUuid));
      stepLine = lines.at(step.lineUuid);
    }
    void visitUnboardingStep(const UnboardingStep& ) override {
      //No line for this step type, don't set result
      stepLine.reset();
    }
    void visitWalkingStep(const WalkingStep& ) override {
      //No line for this step type, don't set result
      stepLine.reset();
    }
    std::optional<std::reference_wrapper<const Line>> getResult() override {
      return stepLine;
    }
  };

  /** 
   * @brief Visitor class to get the line ids used by a result object
   * 
   * TODO when allNodes and single calculation are split in the calculation method, we can use concrete 
   * return types instead. When that is possible, consider adding a method in SingleCalculationResult 
   * instead of this visitor.
   * */
  class LineVisitor : public ResultVisitor<std::vector<std::reference_wrapper<const Line>>> {
  private:
    std::vector<std::reference_wrapper<const Line>> linesList;
    LineStepVisitor stepVisitor;
  public:
    LineVisitor(const std::map<boost::uuids::uuid, Line>& _lines): stepVisitor(LineStepVisitor(_lines)) {}
    std::vector<std::reference_wrapper<const Line>> getResult() override {
      return linesList;
    }
    void visitSingleCalculationResult(const SingleCalculationResult& result) override {
      for (auto const& step : result.steps) {
        std::optional<std::reference_wrapper<const Line>> stepLine = step.get()->accept(stepVisitor);
        if (stepLine.has_value()) {
          linesList.push_back(stepLine.value());
        }
      }
    };
    void visitAlternativesResult(const AlternativesResult& ) override {
      // Nothing to do for this result
    }
    void visitAllNodesResult(const AllNodesResult& ) override {
      // Nothing to do for this result
    }
  };

  AlternativesResult Calculator::alternativesRouting(RouteParameters &parameters)
  {
    using LineVector = std::vector<std::reference_wrapper<const Line>>;
    LineVector exceptLinesFromParameters = parameters.getExceptLines(); // make a copy of lines that are already disabled in parameters
    std::vector< LineVector >  allCombinations;
    std::vector< LineVector >  failedCombinations;
    bool                             combinationMatchesWithFailed {false};
    bool                             combinationMatchesWithAtLeastOneFailed {false};
    std::map<LineVector, bool> alreadyCalculatedCombinations;
    std::map<LineVector, bool> alreadyFoundLines;
    std::map<LineVector, int>  foundLinesTravelTimeSeconds;
    int maxTravelTime;
    int alternativeSequence = 1;
    int alternativesCalculatedCount = 1;
    int maxAlternatives = params.maxAlternatives;
    int lastFoundedAtNum = 0;
    //int departureTimeSeconds = -1;

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

      LineVisitor visitor = LineVisitor(lines);

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

      spdlog::debug("  fastestTravelTimeSeconds: {}", routingResult.totalTravelTime);

      // Get the best result and extract the lines from it
      // We will generate all the combinations of those lines and redo the calculation
      // with each of the combination of line excluded 
      LineVector foundLines = routingResult.accept(visitor);
      std::stable_sort(foundLines.begin(),foundLines.end());
      alreadyFoundLines[foundLines]           = true;
      foundLinesTravelTimeSeconds[foundLines] = routingResult.totalTravelTime;
      lastFoundedAtNum = 1;

      spdlog::debug("fastest line ids: {}", LinesToString(foundLines));

      // Generate combination of group of 1 line, then 2 lines up to the total amount of lines
      for (size_t k = 1; k <= foundLines.size(); k++)
      {
        Combinations<std::reference_wrapper<const Line>> combinations(foundLines, k);

        for (auto newCombination : combinations)
        {
          std::stable_sort(newCombination.begin(), newCombination.end());
          allCombinations.push_back(newCombination);
          alreadyCalculatedCombinations[newCombination] = true;
        }
      }

      // Process all combinations and calculate new route with those excluded
      for (size_t i = 0; i < allCombinations.size(); i++)
      {
        if (alternativesCalculatedCount < maxAlternatives && alternativeSequence - 1 < params.maxValidAlternatives)
        {
          // Generate parameters to send to calculate
          const LineVector combination = allCombinations.at(i);
          // TODO: The exception should be part of the calculation specific parameters, which do not exist yet
          alternativeParameters.exceptLines = exceptLinesFromParameters; // reset except lines using parameters
          for (auto line : combination)
          {
            alternativeParameters.exceptLines.push_back(line);
          }

          spdlog::info("calculating alternative {} from a total of {} ...", alternativeSequence, alternativesCalculatedCount);
          
          spdlog::debug("except lines: {}", LinesToString(combination));

          try {
            result = calculate(alternativeParameters, false, true);

            if (result.get()->resType == result_type::SINGLE_CALCULATION)
            {
              SingleCalculationResult& alternativeCalcResult = dynamic_cast<SingleCalculationResult&>(*result.get());

              // Extract lines from new results. If the result is valid, add it to the alternative list
              // and then generation new lines combinations to try other alternatives
              LineVisitor alternativeVisitor = LineVisitor(lines);
              foundLines = alternativeCalcResult.accept(alternativeVisitor);
              std::stable_sort(foundLines.begin(), foundLines.end());

              if (foundLines.size() > 0 && alreadyFoundLines.count(foundLines) == 0)
              {
                alternatives.alternatives.push_back(std::move(result));

                spdlog::debug("travelTimeSeconds: {}  line Uuids: {}",
                              alternativeCalcResult.totalTravelTime,
                              LinesToString(foundLines));
                

                lastFoundedAtNum = alternativesCalculatedCount;
                alreadyFoundLines[foundLines] = true;
                foundLinesTravelTimeSeconds[foundLines] = alternativeCalcResult.totalTravelTime;
                for (size_t k = 1; k <= foundLines.size(); k++)
                {
                  Combinations<std::reference_wrapper<const Line>> combinations(foundLines, k);
                  for (auto newCombination : combinations)
                  {
                    // Add the lines currently exclused (combination) to the newly generated combinations
                    // from the line in the latest alternative
                    std::copy(combination.begin(), combination.end(),
                              std::back_inserter(newCombination));
                    std::stable_sort(newCombination.begin(), newCombination.end());
                    if (alreadyCalculatedCombinations.count(newCombination) == 0)
                    {
                      combinationMatchesWithAtLeastOneFailed = false;
                      for (auto failedCombination : failedCombinations)
                      {
                        combinationMatchesWithFailed = true;
                        for (auto failedLine : failedCombination)
                        {
                          if (std::find(newCombination.begin(), newCombination.end(), failedLine) == newCombination.end())
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
              spdlog::debug("failed combinations: {}", LinesToString(failedCombination));
            }
          }
        }
      }

      int i {0};
      for (auto flines : alreadyFoundLines)
      {          
        spdlog::debug("{}. {} tt: ", i, LinesToString(flines.first),
                      (foundLinesTravelTimeSeconds[flines.first] / 60));
        i++;          
      }
      
      spdlog::debug("last alternative found at: {} on a total of {} calculations", lastFoundedAtNum,  maxAlternatives);

      alternatives.totalAlternativesCalculated = alternativesCalculatedCount;
      return alternatives;

    }
    throw NoRoutingFoundException(NoRoutingReason::NO_ROUTING_FOUND);

  }



}
