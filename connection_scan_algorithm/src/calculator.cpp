#include "spdlog/spdlog.h"

#include "calculator.hpp"
#include "toolbox.hpp"
#include "parameters.hpp"
#include "routing_result.hpp"
#include "node.hpp"
#include "trip.hpp"
#include "point.hpp"
#include "transit_data.hpp"

namespace TrRouting
{
  AccessibilityParameters routeToAccessibilityParameters(RouteParameters &parameters) {
    return AccessibilityParameters(parameters.isForwardCalculation() ? std::make_unique<Point>(parameters.getOrigin()->latitude, parameters.getOrigin()->longitude) : std::make_unique<Point>(parameters.getDestination()->latitude, parameters.getDestination()->longitude),
      parameters.getScenario(),
      parameters.getTimeOfTrip(),
      parameters.getMinWaitingTimeSeconds(),
      parameters.getMaxTotalTravelTimeSeconds(),
      parameters.getMaxAccessWalkingTravelTimeSeconds(),
      parameters.getMaxEgressWalkingTravelTimeSeconds(),
      parameters.getMaxTransferWalkingTravelTimeSeconds(),
      parameters.getMaxFirstWaitingTimeSeconds(),
      parameters.isForwardCalculation()
    );
  }

  
  
  std::unique_ptr<SingleCalculationResult> Calculator::calculateSingle(RouteParameters &parameters, bool resetAccessPaths, bool resetFilters) {
    reset(parameters, *parameters.getOrigin(), *parameters.getDestination(), resetAccessPaths, resetFilters);

    std::unique_ptr<SingleCalculationResult> result;
    RouteParameters* calculationParametersPtr = &parameters;
    std::unique_ptr<RouteParameters> forwardParametersHolder;

    // Reverse calculation requested (with arrival time): First get the best
    // departure time for this trip, before doing a normal forward calculation
    // that is known to work and fully tested. We could not simply use the
    // calculateSingleReverse as previously as it assumes the arrival time is
    // the best one and will tend to use any paths that gets to destination,
    // even if it arrives later
    if (arrivalTimeSeconds > -1 && !parameters.isForwardCalculation())
    {
      // TODO maybe we can do something different in that case, like a query flag
      // we need to make all trips usable when not coming from forward result because reverse calculation, by default, checks for usableTrips
      for (auto && tripIte : transitData.getTrips()) {
        const Trip & trip = tripIte.second;
        tripsQueryOverlay[trip.uid].usable = true;
      }
      std::optional<std::reference_wrapper<const Node>> bestAccessNode;
      std::unordered_map<Node::uid_t, JourneyStep> reverseAccessJourneysSteps;

      auto resultCalculation = reverseCalculation(parameters, reverseAccessJourneysSteps);
      if (resultCalculation) {
        // Transform the query to a forward one with the new departure time
        departureTimeSeconds = std::get<0>(*resultCalculation);
        bestAccessNode = std::get<1>(*resultCalculation);

        spdlog::debug("-- first reverse calculation -- {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);
        calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

        forwardParametersHolder = std::make_unique<RouteParameters>(
          std::make_unique<Point>(parameters.getOrigin()->latitude, parameters.getOrigin()->longitude),
          std::make_unique<Point>(parameters.getDestination()->latitude, parameters.getDestination()->longitude),
          parameters.getScenario(),
          departureTimeSeconds,
          parameters.getMinWaitingTimeSeconds(),
          parameters.getMaxTotalTravelTimeSeconds(),
          parameters.getMaxAccessWalkingTravelTimeSeconds(),
          parameters.getMaxEgressWalkingTravelTimeSeconds(),
          parameters.getMaxTransferWalkingTravelTimeSeconds(),
          parameters.getMaxFirstWaitingTimeSeconds(),
          parameters.isWithAlternatives(),
          true);
        calculationParametersPtr = forwardParametersHolder.get();
        // Do not reset filters for this second reset, otherwise any disabled line/trip will be enabled again and we'll get the same alternative over and over.
        // FIXME: That other TODO above, where the tripsQueryOverlay is all set to usable looks fishy and forces to not reset the filter here. I, tahini, cannot really explain why and that is not good.
        reset(*calculationParametersPtr, *calculationParametersPtr->getOrigin(), *calculationParametersPtr->getDestination(), resetAccessPaths, false);

        spdlog::debug("best departure time after reverse journey: {}", departureTimeSeconds);
      }
    }

    if (departureTimeSeconds > -1 && calculationParametersPtr->isForwardCalculation())
    {
      int bestArrivalTime {MAX_INT};
      std::optional<std::reference_wrapper<const Node>> bestEgressNode;
      //TODO With the TODO later that forwardJourneyStep is not necessary, we can also drop this variable
      std::unordered_map<Node::uid_t, JourneyStep> forwardEgressJourneysSteps;

      auto resultCalculation = forwardCalculation(*calculationParametersPtr, forwardEgressJourneysSteps);
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

        result = calculateSingleReverse(*calculationParametersPtr);
          
      }
      else
      {
        //TODO This will always throw an exception since to get here bestEgressNode must be invalid
        //TODO We can probably just remove the function forwardJourneyStep completely
        result = forwardJourneyStep(*calculationParametersPtr, bestEgressNode, forwardEgressJourneysSteps);

        assert(false); // See TODO
        spdlog::debug("-- forward journey -- {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);
        calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
          
      }
      return result;
    }
    throw NoRoutingFoundException(NoRoutingReason::NO_ROUTING_FOUND);
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

  std::unique_ptr<AllNodesResult> Calculator::calculateAllNodes(AccessibilityParameters &parameters) {
    reset(parameters, 
        parameters.isForwardCalculation() ? std::make_optional(*parameters.getPlace()) : std::nullopt, 
        parameters.isForwardCalculation() ? std::nullopt : std::make_optional(*parameters.getPlace()),
        true,
        true
    );

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
