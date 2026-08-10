#include <sstream>
#include "spdlog/spdlog.h"

#include "calculator.hpp"
#include "constants.hpp"
#include "routing_result.hpp"
#include "line.hpp"
#include "parameters.hpp"
#include "combinations.hpp"
#include "point.hpp"
#include "transit_data.hpp"
#include "alternative_filter.hpp"
#include "line_combinations.hpp"

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
  public:
    LineStepVisitor() {}
    void visitBoardingStep(const BoardingStep& step) override {
      spdlog::debug("Step Visitor line {}", boost::uuids::to_string(step.trip.line.uuid));
      stepLine = step.trip.line;
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
    LineVisitor(): stepVisitor(LineStepVisitor()) {}
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

  AlternativesResult Calculator::alternativesRouting(const RouteParameters &parameters)
  {

    std::vector< LineVector >  allCombinations;
    std::vector< LineVector >  failedCombinations;
    CombinationMap alreadyCalculatedCombinations;
    CombinationMap alreadyFoundLines;
    int maxTravelTime;
    int alternativeSequence = 1;
    int alternativesCalculatedCount = 1;
    const int maxAlternatives = parameters.getMaxAlternatives();
    int lastFoundedAtNum = 0;

    spdlog::debug("alternatives parameters:");
    spdlog::debug("  maxTotalTravelTimeSeconds: {}", parameters.getMaxTotalTravelTimeSeconds());
    spdlog::debug("  minAlternativeMaxTravelTimeSeconds: ", parameters.getMinAlternativeMaxTravelTimeSeconds());
    spdlog::debug("  alternativesMaxAddedTravelTimeSeconds: ", parameters.getAlternativesMaxAddedTravelTimeSeconds());
    spdlog::debug("  alternativesMaxTravelTimeRatio: ", parameters.getAlternativesMaxTravelTimeRatio());
    spdlog::debug("calculating fastest alternative...");
  
    std::unique_ptr<SingleCalculationResult> result = calculateSingle(parameters);

    SingleCalculationResult& routingResult = *result.get();
    AlternativesResult alternatives = AlternativesResult();

    alternatives.alternatives.push_back(std::move(result));

    alternativeSequence++;
    alternativesCalculatedCount++;

    LineVisitor visitor = LineVisitor();

    // TODO Extract the max travel time calculation to a function
    maxTravelTime = parameters.getAlternativesMaxTravelTimeRatio() * routingResult.totalTravelTime + (parameters.isForwardCalculation() ? routingResult.departureTime - parameters.getTimeOfTrip() : 0);
    if (maxTravelTime < parameters.getMinAlternativeMaxTravelTimeSeconds())
    {
      maxTravelTime = parameters.getMinAlternativeMaxTravelTimeSeconds();
    }
    else if (maxTravelTime > routingResult.totalTravelTime + parameters.getAlternativesMaxAddedTravelTimeSeconds())
    {
      maxTravelTime = routingResult.totalTravelTime + parameters.getAlternativesMaxAddedTravelTimeSeconds();
    }
    maxTravelTime = std::min(maxTravelTime, parameters.getMaxTotalTravelTimeSeconds());
    // TODO: We should not create a whole new object just to update maxTravelTime. This parameter should be in the calculation specific parameters, which do not exist yet
    Point* origin = parameters.getOrigin();
    Point* dest = parameters.getDestination();
    CommonParameters commonAlternativeParameters = CommonParameters(parameters.getScenario(),
      parameters.getTimeOfTrip(),
      parameters.getMinWaitingTimeSeconds(),
      maxTravelTime,
      parameters.getMaxAccessWalkingTravelTimeSeconds(),
      parameters.getMaxEgressWalkingTravelTimeSeconds(),
      parameters.getMaxTransferWalkingTravelTimeSeconds(),
      parameters.getMaxInnerTimeOfTripBufferSeconds(),
      parameters.isForwardCalculation()
    );
    RouteParameters alternativeParameters = RouteParameters(std::make_unique<Point>(origin->latitude, origin->longitude),
      std::make_unique<Point>(dest->latitude, dest->longitude),
      parameters.isWithAlternatives(),
      commonAlternativeParameters
    );

    spdlog::debug("  fastestTravelTimeSeconds: {} Maximum alternative travel time: {}", routingResult.totalTravelTime, maxTravelTime);

    // Get the best result and extract the lines from it
    // We will generate all the combinations of those lines and redo the calculation
    // with each of the combination of line excluded
    LineVector foundLines = routingResult.accept(visitor);
    std::stable_sort(foundLines.begin(),foundLines.end());
    alreadyFoundLines[foundLines]           = true;
    lastFoundedAtNum = 1;

    spdlog::debug("fastest line ids: {}", LinesToString(foundLines));

    // Generate combination of group of 1 line, then 2 lines up to the total amount of lines
    generateCombinations(foundLines, {}, failedCombinations, allCombinations, alreadyCalculatedCombinations);

    // Process all combinations and calculate new route with those excluded
    const int maxValidAlternatives = parameters.getMaxValidAlternatives();
    for (size_t i = 0;
         i < allCombinations.size()
           && alternativesCalculatedCount < maxAlternatives
           && alternativeSequence - 1 < maxValidAlternatives;
         i++)
    {
      // Generate parameters to send to calculate
      const LineVector combination = allCombinations.at(i);

      AlternativeLineFilter lineFilter(combination);

      spdlog::debug("calculating alternative {} from a total of {} ...", alternativeSequence, alternativesCalculatedCount);
        
      spdlog::debug("except lines: {}", LinesToString(combination));

      try {
        result = calculateSingle(alternativeParameters, false, &lineFilter);

        SingleCalculationResult& alternativeCalcResult = *result.get();

        // Extract lines from new results. If the result is valid, add it to the alternative list
        // and then generation new lines combinations to try other alternatives
        LineVisitor alternativeVisitor = LineVisitor();
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
            // Generate new combinations from the new foundlines
            generateCombinations(foundLines, combination, failedCombinations, allCombinations, alreadyCalculatedCombinations);

          alternativeSequence++;

        }
      } catch (NoRoutingFoundException& e) {

        failedCombinations.push_back(combination);
      }

      alternativesCalculatedCount++;
    }

    // Print failed combinations
    for (auto failedCombination : failedCombinations)
    {
      spdlog::debug("failed combinations: {}", LinesToString(failedCombination));
    }
    
    spdlog::debug("last alternative found at: {} on a total of {} calculations done. Maximum possible: {}",
      lastFoundedAtNum,  alternativesCalculatedCount, maxAlternatives);

    alternatives.totalAlternativesCalculated = alternativesCalculatedCount;
    return alternatives;

  }

}
