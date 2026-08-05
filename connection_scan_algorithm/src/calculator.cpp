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
      parameters.getMaxInnerTimeOfTripBufferSeconds(),
      parameters.isForwardCalculation()
    );
  }
  
  std::unique_ptr<SingleCalculationResult> Calculator::calculateSingle(RouteParameters &parameters, bool resetAccessPaths, AlternativeFilter *alternativeFilter) {
    reset(parameters, *parameters.getOrigin(), *parameters.getDestination(), resetAccessPaths, alternativeFilter);

    std::unique_ptr<SingleCalculationResult> result;

    if (departureTimeSeconds > -1 && parameters.isForwardCalculation())
    {
      std::unordered_map<Node::uid_t, JourneyStep> forwardEgressJourneysSteps;

      auto resultCalculation = forwardCalculation(parameters, forwardEgressJourneysSteps);
      spdlog::debug("-- forward calculation -- {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);
      calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

      if (resultCalculation.has_value()) {
        int bestArrivalTime = std::get<0>(*resultCalculation);
        
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
        // There's service at access/egress but no routing found
        spdlog::debug("no routing found in forward trip calculation");
        throw NoRoutingFoundException(NoRoutingReason::NO_ROUTING_FOUND);
      }
    }
    else if (arrivalTimeSeconds > -1)
    {
      std::unordered_map<Node::uid_t, JourneyStep> reverseAccessJourneysSteps;

      // TODO maybe we can do something different in that case, like a query flag
      // we need to make all trips usable when not coming from forward result because reverse calculation, by default, checks for usableTrips
      for (auto && tripIte : transitData.getTrips()) {
        const Trip & trip = tripIte.second;
        tripsQueryOverlay[trip.uid].usable = true;
      }

      auto resultCalculation = reverseCalculation(parameters, reverseAccessJourneysSteps);
      spdlog::debug("-- reverse calculation -- {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);
      calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
      if (resultCalculation.has_value()) {
        int bestDepartureTime = std::get<0>(*resultCalculation);
        
        spdlog::debug("bestDepartureTime after reverse journey: {}", bestDepartureTime);
          
        departureTimeSeconds = bestDepartureTime;
          
        for (auto & accessFootpath : accessFootpaths) // reset nodes reverse tentative times with new arrival time:
        {
          nodesTentativeTime[accessFootpath.node.uid] = departureTimeSeconds + accessFootpath.time;
        }

        result = calculateSingleForward(parameters);
      }
      else
      {
        // There's service at access/egress but no routing found
        spdlog::debug("no routing found in reverse trip calculation");
        throw NoRoutingFoundException(NoRoutingReason::NO_ROUTING_FOUND);
      }

    }

    return result;
  }

  // To be called only by calculateSingle, depends on preparations steps done there
  std::unique_ptr<SingleCalculationResult> Calculator::calculateSingleForward(RouteParameters &parameters) {

    std::unique_ptr<SingleCalculationResult> result;

    std::optional<std::reference_wrapper<const Node>> bestEgressNode;
    std::unordered_map<Node::uid_t, JourneyStep> forwardEgressJourneysSteps;

    auto resultCalculation = forwardCalculation(parameters, forwardEgressJourneysSteps);
    if (resultCalculation) {
      bestEgressNode = std::get<1>(*resultCalculation);
    }

    spdlog::debug("-- forward calculation --  {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
    result = forwardJourneyStep(parameters, bestEgressNode, forwardEgressJourneysSteps);

    spdlog::debug("-- forward journey -- {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

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

  std::unique_ptr<AllNodesResult> Calculator::calculateAllNodes(AccessibilityParameters &parameters) {
    reset(parameters, 
        parameters.isForwardCalculation() ? std::make_optional(*parameters.getPlace()) : std::nullopt, 
        parameters.isForwardCalculation() ? std::nullopt : std::make_optional(*parameters.getPlace()),
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
