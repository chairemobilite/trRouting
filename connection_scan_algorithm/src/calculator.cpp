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

    reset(parameters, resetAccessPaths, resetFilters);

    std::unique_ptr<RoutingResult> result;
    
    int bestArrivalTime {MAX_INT};
    int bestDepartureTime {-1};
    std::optional<std::reference_wrapper<const Node>> bestEgressNode;
    std::optional<std::reference_wrapper<const Node>> bestAccessNode;

    // TODO Could they be local to each section?
    std::unordered_map<Node::uid_t, JourneyStep> forwardEgressJourneysSteps;
    std::unordered_map<Node::uid_t, JourneyStep> reverseAccessJourneysSteps;


    if (departureTimeSeconds > -1 && parameters.isForwardCalculation())
    {
      
      initialDepartureTimeSeconds = departureTimeSeconds; // set initial departure time so we can find the latest possible departure time with reverse calculation later and still know the initial waiting time

      auto resultCalculation = forwardCalculation(parameters, forwardEgressJourneysSteps);
      if (resultCalculation.has_value()) {
        bestArrivalTime = std::get<0>(*resultCalculation);
        bestEgressNode = std::get<1>(*resultCalculation);
      }

      spdlog::debug("-- forward calculation -- {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);
      calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
      
      if (params.returnAllNodesResult)
      {
        result = forwardJourneyStep(parameters, bestArrivalTime, bestEgressNode, forwardEgressJourneysSteps);
        
        spdlog::debug("-- forward journey -- {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);
        calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
      }
      else
      {
        
        if (bestArrivalTime < MAX_INT)
        {
          spdlog::debug("bestArrivalTime after forward journey: {}", bestArrivalTime);
          
          arrivalTimeSeconds = bestArrivalTime;
          
          for (auto & egressFootpath : egressFootpaths) // reset nodes reverse tentative times with new arrival time:
          {
            nodesReverseTentativeTime[egressFootpath.node.uid] = arrivalTimeSeconds - egressFootpath.time;
          }

          auto resultCalculationRev = reverseCalculation(parameters, reverseAccessJourneysSteps);
          if (resultCalculationRev) {
            bestDepartureTime = std::get<0>(*resultCalculationRev);
            bestAccessNode = std::get<1>(*resultCalculationRev);
          }

          spdlog::debug("-- reverse calculation -- {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);
          calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

          result = reverseJourneyStep(parameters, bestDepartureTime, bestAccessNode, reverseAccessJourneysSteps);

          spdlog::debug("-- reverse journey -- {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);
          calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
          
        }
        else
        {

          result = forwardJourneyStep(parameters, bestArrivalTime, bestEgressNode, forwardEgressJourneysSteps);

          spdlog::debug("-- forward journey -- {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);
          calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
          
        }
        
      }
    }
    else if (arrivalTimeSeconds > -1)
    {
      departureTimeSeconds = -1;
      initialDepartureTimeSeconds = -1;
      //TODO maybe we can do something different in that case, like a query flag
      // we need to make all trips usable when not coming from forward result because reverse calculation, by default, checks for usableTrips
      for (auto && tripIte : transitData.getTrips()) {
        const Trip & trip = tripIte.second;
        tripsQueryOverlay[trip.uid].usable = true;
      }

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

    }

    return result;
    
  }
  
}
