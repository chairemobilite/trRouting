#include "calculator.hpp"
#include "constants.hpp"
#include "routing_result.hpp"

namespace TrRouting
{
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

    if (params.debugDisplay)
    {
      std::cout << "alternatives parameters:" << std::endl;
      std::cout << "  maxTotalTravelTimeSeconds: " << parameters.getMaxTotalTravelTimeSeconds() << std::endl;
      std::cout << "  minAlternativeMaxTravelTimeSeconds: " << params.minAlternativeMaxTravelTimeSeconds << std::endl;
      std::cout << "  alternativesMaxAddedTravelTimeSeconds: " << params.alternativesMaxAddedTravelTimeSeconds << std::endl;
      std::cout << "  alternativesMaxTravelTimeRatio: " << params.alternativesMaxTravelTimeRatio << std::endl;
      std::cout << "calculating fastest alternative..." << std::endl;
    }

    std::unique_ptr<RoutingResultNew> result = calculate(parameters);

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

      if (params.debugDisplay)
        std::cout << "  fastestTravelTimeSeconds: " << routingResult.totalTravelTime << std::endl;

      foundLinesIdx = routingResult.accept(visitor);
      std::stable_sort(foundLinesIdx.begin(),foundLinesIdx.end());
      alreadyFoundLinesIdx[foundLinesIdx]           = true;
      foundLinesIdxTravelTimeSeconds[foundLinesIdx] = routingResult.totalTravelTime;
      lastFoundedAtNum = 1;

      if (params.debugDisplay)
      {
        std::cout << "fastest line ids: ";
        for (auto lineIdx : foundLinesIdx)
        {
          std::cout << lines[lineIdx].get()->shortname << " ";
        }
        std::cout << std::endl;
      }


      combinationsKs.clear();
      for (int i = 1; i <= foundLinesIdx.size(); i++) { combinationsKs.push_back(i); }
      for (auto k : combinationsKs)
      {
        Combinations<int> combinations(foundLinesIdx, k);
        //std::cout << "\nk = " << k << std::endl;
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

          //if (params.debugDisplay)
          //{
            std::cerr << "calculating alternative " << alternativeSequence << " from a total of " << alternativesCalculatedCount << "..." << std::endl;
          //}


          if (params.debugDisplay)
          {
            std::cout << "except lines: ";
            for (auto lineIdx : combination)
            {
              std::cout << lines[lineIdx].get()->shortname << " ";
            }
            std::cout << std::endl;
          }



          //std::cout << std::endl;



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

                if (params.debugDisplay)
                {
                  std::cout << "travelTimeSeconds: " << alternativeCalcResult.totalTravelTime << " line Uuids: ";
                  for (auto lineIdx : foundLinesIdx)
                  {
                    std::cout << lines[lineIdx].get()->shortname << " ";
                  }
                  std::cout << std::endl;
                }

                combinationsKs.clear();

                lastFoundedAtNum = alternativesCalculatedCount;
                alreadyFoundLinesIdx[foundLinesIdx] = true;
                foundLinesIdxTravelTimeSeconds[foundLinesIdx] = alternativeCalcResult.totalTravelTime;
                for (int i = 1; i <= foundLinesIdx.size(); i++) { combinationsKs.push_back(i); }
                for (auto k : combinationsKs)
                {
                  Combinations<int> combinations(foundLinesIdx, k);
                  //std::cout << "\nk = " << k << std::endl;
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
            //std::cout << "failed" << std::endl;
            failedCombinations.push_back(combination);
          }

          alternativesCalculatedCount++;

          if (params.debugDisplay)
          {
            if (failedCombinations.size() > 0)
            {
              std::cout << "failed combinations: ";
              for (auto failedCombination : failedCombinations)
              {
                for (auto lineIdx : failedCombination)
                {
                  std::cout << "  " << lines[lineIdx].get()->shortname << " ";
                }
                std::cout << std::endl;
              }
            }
          }

        }
      }

      if (params.debugDisplay)
      {
        std::cout << std::endl;
      }

      if (params.debugDisplay)
      {
        int i {0};
        for (auto foundLinesIdx : alreadyFoundLinesIdx)
        {
          std::cout << i << ". ";
          for(auto lineIdx : foundLinesIdx.first)
          {
            std::cout << lines[lineIdx].get()->shortname << " ";
          }
          std::cout << " tt: " << (foundLinesIdxTravelTimeSeconds[foundLinesIdx.first] / 60);
          i++;
          std::cout << std::endl;
        }

        std::cout << "last alternative found at: " << lastFoundedAtNum << " on a total of " << maxAlternatives << " calculations" << std::endl;

      }
      alternatives.totalAlternativesCalculated = alternativesCalculatedCount;
      return alternatives;

    }
    throw NoRoutingFoundException(NoRoutingFoundException::NO_ROUTING_FOUND);

  }



}
