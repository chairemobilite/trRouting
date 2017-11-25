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
      std::tie(bestArrivalTime, bestEgressStopIndex, bestEgressTravelTime)   = forwardCalculation();

      std::cerr << "-- forward calculation -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
      calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

      result = forwardJourney(bestArrivalTime, bestEgressStopIndex, bestEgressTravelTime);

      std::cerr << "-- forward journey -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
      calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

    }
    else if (arrivalTimeSeconds > -1)
    {
      std::tie(bestDepartureTime, bestAccessStopIndex, bestAccessTravelTime) = reverseCalculation();

      std::cerr << "-- reverse calculation -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
      calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

      result = reverseJourney(bestDepartureTime, bestAccessStopIndex, bestAccessTravelTime);

      std::cerr << "-- reverse journey -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
      calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

    }

    //if (!params.returnAllStopsResult && bestArrivalTime < MAX_INT && bestEgressStopIndex != -1)
    //{
    //  arrivalTimeSeconds = bestArrivalTime;
    //  
    //  
    //  Calculator::reverseCalculation();
    //      
    //}

    return result;
    
  }
  
}
