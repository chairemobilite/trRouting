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
