#include "spdlog/spdlog.h"

#include "calculator.hpp"
#include "toolbox.hpp"
#include "parameters.hpp"
#include "routing_result.hpp"
#include "node.hpp"


namespace TrRouting
{
  
  int Calculator::countAgencies() {
    return agencies.size();
  }

  int Calculator::countServices() {
    return services.size();
  }

  int Calculator::countNodes() {
    return nodes.size();
  }

  int Calculator::countLines() {
    return lines.size();
  }

  int Calculator::countPaths() {
    return paths.size();
  }

  int Calculator::countScenarios() {
    return scenarios.size();
  }

  int Calculator::countTrips() {
    return trips.size();
  }

  long long Calculator::countConnections() {
    return forwardConnections.size();
  }

  std::unique_ptr<RoutingResult> Calculator::calculate(RouteParameters &parameters, bool resetAccessPaths, bool resetFilters) {

    reset(parameters, resetAccessPaths, resetFilters);

    std::unique_ptr<RoutingResult> result;
    
    std::tuple<int,int,int> forwardResult;
    std::tuple<int,int,int> reverseResult;
    
    int bestArrivalTime {MAX_INT};
    int bestDepartureTime {-1};
    std::optional<std::reference_wrapper<const Node>> bestEgressNode;
    std::optional<std::reference_wrapper<const Node>> bestAccessNode;

    if (departureTimeSeconds > -1 && parameters.isForwardCalculation())
    {
      
      initialDepartureTimeSeconds = departureTimeSeconds; // set initial departure time so we can find the latest possible departure time with reverse calculation later and still know the initial waiting time

      auto resultCalculation = forwardCalculation(parameters);
      if (resultCalculation.has_value()) {
        bestArrivalTime = std::get<0>(*resultCalculation);
        bestEgressNode = std::get<1>(*resultCalculation);
      }

      spdlog::debug("-- forward calculation -- {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);
      calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
      
      if (params.returnAllNodesResult)
      {
        result = forwardJourneyStep(parameters, bestArrivalTime, bestEgressNode);
        
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

          auto resultCalculationRev = reverseCalculation(parameters);
          if (resultCalculationRev) {
            bestDepartureTime = std::get<0>(*resultCalculationRev);
            bestAccessNode = std::get<1>(*resultCalculationRev);
          }

          spdlog::debug("-- reverse calculation -- {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);
          calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

          result = reverseJourneyStep(parameters, bestDepartureTime, bestAccessNode);

          spdlog::debug("-- reverse journey -- {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);
          calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
          
        }
        else
        {

          result = forwardJourneyStep(parameters, bestArrivalTime, bestEgressNode);

          spdlog::debug("-- forward journey -- {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);
          calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
          
        }
        
      }
    }
    else if (arrivalTimeSeconds > -1)
    {
      departureTimeSeconds = -1;
      initialDepartureTimeSeconds = -1;
      std::fill(tripsUsable.begin(), tripsUsable.end(), 1);
      //tripsUsable = std::vector<std::unique_ptr<int>>(trips.size(), std::make_unique<int>(1));
      //std::fill(tripsUsable.begin(), tripsUsable.end(), std::make_unique<int>(1)); // we need to make all trips usable when not coming from forward result because reverse calculation, by default, checks for usableTrips == 1

      auto resultCalculation = reverseCalculation(parameters);
      if (resultCalculation) {
        bestDepartureTime = std::get<0>(*resultCalculation);
        bestAccessNode = std::get<1>(*resultCalculation);
      }

      spdlog::debug("-- reverse calculation --  {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);
      calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

      result = reverseJourneyStep(parameters, bestDepartureTime, bestAccessNode);

      spdlog::debug("-- reverse journey -- {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);
      calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

    }

    return std::move(result);
    
  }
  
}
