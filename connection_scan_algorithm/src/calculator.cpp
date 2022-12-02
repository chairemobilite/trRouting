#include "spdlog/spdlog.h"

#include "calculator.hpp"
#include "toolbox.hpp"
#include "parameters.hpp"
#include "routing_result.hpp"
#include "node.hpp"
#include "trip.hpp"
#include "transit_data.hpp"

namespace TrRouting
{
  
  std::unique_ptr<RoutingResult> Calculator::calculate(RouteParameters &parameters, bool resetAccessPaths, bool resetFilters) {

    if (params.returnAllNodesResult) {
      return calculateAllNodes(parameters, resetAccessPaths, resetFilters);
    } else {
      return calculateSingle(parameters, resetAccessPaths, resetFilters);
    }
  }

  std::unique_ptr<SingleCalculationResult> Calculator::calculateSingle(RouteParameters &parameters, bool resetAccessPaths, bool resetFilters) {
    reset(parameters, resetAccessPaths, resetFilters);

    std::unique_ptr<SingleCalculationResult> result;

    if (departureTimeSeconds > -1 && parameters.isForwardCalculation())
    {
      
      int bestArrivalTime {MAX_INT};
      std::optional<std::reference_wrapper<const Node>> bestEgressNode;
      //TODO With the TODO later that forwardJourneyStep is not necessary, we can also drop this variable
      std::unordered_map<Node::uid_t, JourneyStep> forwardEgressJourneysSteps;

      auto resultCalculation = forwardCalculation(parameters, forwardEgressJourneysSteps);
      if (resultCalculation.has_value()) {
        bestArrivalTime = std::get<0>(*resultCalculation);
        bestEgressNode = std::get<1>(*resultCalculation);
      }

      spdlog::debug("-- forward calculation -- {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);
      calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
        
      if (bestArrivalTime < MAX_INT)
      {
        spdlog::debug("bestArrivalTime after forward journey: {}", bestArrivalTime);
          
        arrivalTimeSeconds = bestArrivalTime;
          
        for (auto & egressFootpath : egressFootpaths) // reset nodes reverse tentative times with new arrival time:
        {
          nodesReverseTentativeTime[egressFootpath.node.uid] = arrivalTimeSeconds - egressFootpath.time;
        }

        result = calculateSingleReverse(parameters);
          
      }
      else
      {
        //TODO This will always throw an exception since to get here bestEgressNode must be invalid
        //TODO We can probably just remove the function forwardJourneyStep completely
        result = forwardJourneyStep(parameters, bestEgressNode, forwardEgressJourneysSteps);

        assert(false); // See TODO
        spdlog::debug("-- forward journey -- {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);
        calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
          
      }
    }
    else if (arrivalTimeSeconds > -1)
    {
      departureTimeSeconds = -1;
      //TODO maybe we can do something different in that case, like a query flag
      // we need to make all trips usable when not coming from forward result because reverse calculation, by default, checks for usableTrips
      for (auto && tripIte : transitData.getTrips()) {
        const Trip & trip = tripIte.second;
        tripsQueryOverlay[trip.uid].usable = true;
      }
      result = calculateSingleReverse(parameters);

    }

    return result;
  }

  // To be called only by calculateSingle, depends on preparations steps done there
  std::unique_ptr<SingleCalculationResult> Calculator::calculateSingleReverse(RouteParameters &parameters) {

    std::unique_ptr<SingleCalculationResult> result;

    int bestDepartureTime {-1};
    std::optional<std::reference_wrapper<const Node>> bestAccessNode;
    std::unordered_map<Node::uid_t, JourneyStep> reverseAccessJourneysSteps;

    auto resultCalculation = reverseCalculation(parameters, reverseAccessJourneysSteps);
    if (resultCalculation) {
      bestDepartureTime = std::get<0>(*resultCalculation);
      bestAccessNode = std::get<1>(*resultCalculation);
    }

    spdlog::debug("-- reverse calculation --  {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
    result = reverseJourneyStep(parameters, bestDepartureTime, bestAccessNode, reverseAccessJourneysSteps);

    spdlog::debug("-- reverse journey -- {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

    return result;
  }

  std::unique_ptr<AllNodesResult> Calculator::calculateAllNodes(RouteParameters &parameters, bool resetAccessPaths, bool resetFilters) {
    reset(parameters, resetAccessPaths, resetFilters);

    std::unique_ptr<AllNodesResult> result;

    if (departureTimeSeconds > -1 && parameters.isForwardCalculation())
    {
      std::unordered_map<Node::uid_t, JourneyStep> forwardEgressJourneysSteps;

      forwardCalculationAllNodes(parameters, forwardEgressJourneysSteps);

      spdlog::debug("-- forward calculation all nodes -- {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);
      calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

      result = forwardJourneyStepAllNodes(parameters, forwardEgressJourneysSteps);

      spdlog::debug("-- forward journey all nodes -- {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);
      calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

    }
    else if (arrivalTimeSeconds > -1)
    {
      std::unordered_map<Node::uid_t, JourneyStep> reverseAccessJourneysSteps;

      departureTimeSeconds = -1;
      //TODO maybe we can do something different in that case, like a query flag
      // we need to make all trips usable when not coming from forward result because reverse calculation, by default, checks for usableTrips
      for (auto && tripIte : transitData.getTrips()) {
        const Trip & trip = tripIte.second;
        tripsQueryOverlay[trip.uid].usable = true;
      }

      reverseCalculationAllNodes(parameters, reverseAccessJourneysSteps);

      spdlog::debug("-- reverse calculation --  {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);
      calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

      result = reverseJourneyStepAllNodes(parameters, reverseAccessJourneysSteps);

      spdlog::debug("-- reverse journey -- {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);
      calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
    }

    return result;
  }
  
}
