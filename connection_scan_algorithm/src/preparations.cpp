#include "calculator.hpp"

namespace TrRouting
{
  
  void Calculator::prepare()
  {
    prepareStops();
    prepareRoutes();
    prepareTrips();
    prepareConnections();
    prepareFootpaths();
    
    std::cout << "preparing stops tentative times, trips enter connections and journeys..." << std::endl;
    
    stopsTentativeTimes  = std::vector<int>(stops.size());
    tripsEnterConnection = std::vector<int>(trips.size());
    journeys             = std::vector<std::tuple<int,int,int>>(stops.size());
    std::fill(stopsTentativeTimes.begin(), stopsTentativeTimes.end(), std::numeric_limits<int>::max());
    std::fill(stopsTentativeTimes.begin(), stopsTentativeTimes.end(), -1);
    std::fill(journeys.begin(), journeys.end(), std::make_tuple(-1,-1,-1));
    
  }

  void Calculator::prepareStops()
  {
    
    std::cerr << "preparing stops..." << std::endl;
    std::tie(stops, stopIndexesById) = params.dataFetcher->getStops(params.applicationShortname);
    
  }

  void Calculator::prepareRoutes()
  {
    
    std::cerr << "preparing routes..." << std::endl;
    std::tie(routes, routeIndexesById) = params.dataFetcher->getRoutes(params.applicationShortname);
    
  }

  void Calculator::prepareTrips()
  {
    
    std::cerr << "preparing trips..." << std::endl;
    std::tie(trips, tripIndexesById) = params.dataFetcher->getTrips(params.applicationShortname);
    
  }

  void Calculator::prepareConnections()
  {
    
    std::cerr << "preparing connections..." << std::endl;
    std::tie(forwardConnections, reverseConnections) = params.dataFetcher->getConnections(params.applicationShortname, stopIndexesById, tripIndexesById);
    
  }
  
  void Calculator::prepareFootpaths()
  {
    
    std::cerr << "preparing footpaths..." << std::endl;
    std::tie(footpaths, footpathsRanges) = params.dataFetcher->getFootpaths(params.applicationShortname, stopIndexesById, params.maxTransferWalkingTravelTimeSeconds);
    
  }
  
  void Calculator::prepareAccessFoothpaths()
  {
    
  }

  void Calculator::prepareEgressFootpaths()
  {
    
  }
  
}
