#include "calculator.hpp"

namespace TrRouting
{
  
  int Calculator::countStations() {
    return stations.size();
  }

  int Calculator::countAgencies() {
    return agencies.size();
  }

  int Calculator::countServices() {
    return services.size();
  }

  int Calculator::countNodes() {
    return nodes.size();
  }

  int Calculator::countStops() {
    return stops.size();
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

  int Calculator::countNetworks() {
    return networks.size();
  }

  RoutingResult Calculator::calculate(bool resetAccessPaths, bool resetFilters) {
    
    reset(resetAccessPaths, resetFilters);

    RoutingResult result;
    
    result.json = {};
    
    std::tuple<int,int,int> forwardResult;
    std::tuple<int,int,int> reverseResult;
    
    int i {0};
    int bestEgressNodeIndex {-1};
    int bestEgressTravelTime {-1};
    int bestEgressDistance {-1};
    int bestArrivalTime {MAX_INT};
    int bestAccessNodeIndex {-1};
    int bestAccessTravelTime {-1};
    int bestAccessDistance {-1};
    int bestDepartureTime {-1};
    
    if (departureTimeSeconds > -1 && params.forwardCalculation == true)
    {
      
      initialDepartureTimeSeconds = departureTimeSeconds; // set initial departure time so we can find the latest possible departure time with reverse calculation later and still know the initial waiting time
      
      std::tie(bestArrivalTime, bestEgressNodeIndex, bestEgressTravelTime, bestEgressDistance) = forwardCalculation();

      if (params.debugDisplay)
        std::cerr << "-- forward calculation -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
      calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
      
      if (params.returnAllNodesResult)
      {
        result = forwardJourneyStep(bestArrivalTime, bestEgressNodeIndex, bestEgressTravelTime, bestEgressDistance);
        if (params.debugDisplay)
          std::cerr << "-- forward journey -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
        calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
      }
      else
      {
        
        if (bestArrivalTime < MAX_INT)
        {
          if (params.debugDisplay)
            std::cout << "bestArrivalTime after forward journey: " << bestArrivalTime << std::endl;
          
          arrivalTimeSeconds = bestArrivalTime;
          
          for (auto & egressFootpath : egressFootpaths) // reset nodes reverse tentative times with new arrival time:
          {
            nodesReverseTentativeTime[std::get<0>(egressFootpath)] = arrivalTimeSeconds - std::get<1>(egressFootpath);
          }
          
          std::tie(bestDepartureTime, bestAccessNodeIndex, bestAccessTravelTime, bestAccessDistance) = reverseCalculation();
          if (params.debugDisplay)
            std::cerr << "-- reverse calculation -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
          calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
          
          result = reverseJourneyStep(bestDepartureTime, bestAccessNodeIndex, bestAccessTravelTime, bestAccessDistance);
          if (params.debugDisplay)
            std::cerr << "-- reverse journey -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
          calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
          
        }
        else
        {
          
          result = forwardJourneyStep(bestArrivalTime, bestEgressNodeIndex, bestEgressTravelTime, bestEgressDistance);
          if (params.debugDisplay)
            std::cerr << "-- forward journey -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
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
      
      std::tie(bestDepartureTime, bestAccessNodeIndex, bestAccessTravelTime, bestAccessDistance) = reverseCalculation();
      if (params.debugDisplay)
        std::cerr << "-- reverse calculation -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
      calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

      result = reverseJourneyStep(bestDepartureTime, bestAccessNodeIndex, bestAccessTravelTime, bestAccessDistance);
      if (params.debugDisplay)
        std::cerr << "-- reverse journey -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
      calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

    }

    return result;
    
  }
  
}
