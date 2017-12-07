#include "calculator.hpp"

namespace TrRouting
{
  
  RoutingResult Calculator::calculate() {
    
    reset();

    RoutingResult result;
    
    result.json = "";
    
    std::tuple<int,int,int> forwardResult;
    std::tuple<int,int,int> reverseResult;

    int i {0};
    int bestEgressStopIndex {-1};
    int bestEgressTravelTime {-1};
    int bestArrivalTime {MAX_INT};
    int bestAccessStopIndex {-1};
    int bestAccessTravelTime {-1};
    int bestDepartureTime {-1};

    if (departureTimeSeconds > -1)
    {
      
      initialDepartureTimeSeconds = departureTimeSeconds; // set initial departure time so we can find the latest possible departure time with reverse calculation later and still know the initial waiting time
      
      std::tie(bestArrivalTime, bestEgressStopIndex, bestEgressTravelTime) = forwardCalculation();

      std::cerr << "-- forward calculation -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
      calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
      
      if (params.returnAllStopsResult)
      {
        result = forwardJourney(bestArrivalTime, bestEgressStopIndex, bestEgressTravelTime);
        
        std::cerr << "-- forward journey -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
        calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
      }
      else
      {
        
        if (bestArrivalTime < MAX_INT)
        {
          departureTimeSeconds = -1;
          arrivalTimeSeconds   = bestArrivalTime;
          
          for (auto & egressFootpath : egressFootpaths) // reset stops reverse tentative times with new arrival time:
          {
            stopsReverseTentativeTime[egressFootpath.first] = arrivalTimeSeconds - egressFootpath.second;
          }
          
          std::tie(bestDepartureTime, bestAccessStopIndex, bestAccessTravelTime) = reverseCalculation();
      
          std::cerr << "-- reverse calculation -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
          calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
          
          result = reverseJourney(bestDepartureTime, bestAccessStopIndex, bestAccessTravelTime);
          
          std::cerr << "-- reverse journey -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
          calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
          
        }
        else
        {
          
          result = forwardJourney(bestArrivalTime, bestEgressStopIndex, bestEgressTravelTime);
          
          std::cerr << "-- forward journey -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
          calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
          
        }
        
      }
    }
    else if (arrivalTimeSeconds > -1)
    {
      
      std::fill(tripsUsable.begin(), tripsUsable.end(), 1); // we need to make all trips usable when not coming from forward result because reverse calculation, by default, checks for usableTrips == 1
      
      std::tie(bestDepartureTime, bestAccessStopIndex, bestAccessTravelTime) = reverseCalculation();

      std::cerr << "-- reverse calculation -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
      calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

      result = reverseJourney(bestDepartureTime, bestAccessStopIndex, bestAccessTravelTime);

      std::cerr << "-- reverse journey -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
      calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

    }

    return result;
    
  }
  
}
