#include "calculator.hpp"
#include "json.hpp"

namespace TrRouting
{
    
  RoutingResult Calculator::reverseJourney(int bestDepartureTime, int bestAccessStopIndex, int bestAccessTravelTime)
  {
    RoutingResult result;
    nlohmann::json json;
    int stopsCount {1};
    std::vector<int> resultingStops;
    int i {0};
    bool foundRoute {true};
    
    if (params.returnAllStopsResult)
    {
      stopsCount = stops.size();
      resultingStops = std::vector<int>(stopsCount);
      std::iota (std::begin(resultingStops), std::end(resultingStops), 0); // generate sequencial indexes of each stops
    }
    else
    {
      std::vector<int> resultingStops;
      resultingStops = std::vector<int>(stopsCount);
    }

    result.json = json.dump();
    return result;
  }

}