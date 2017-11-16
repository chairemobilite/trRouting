#include "calculator.hpp"

namespace TrRouting
{
    
  void Calculator::prepare()
  {
    
    std::cerr << "preparing stops, routes, trips, connections and footpaths..." << std::endl;
    if (params.dataFetcherShortname == "database")
    {
      std::tie(stops, stopIndexesById)                 = params.databaseFetcher->getStops(params.applicationShortname);
      std::tie(routes, routeIndexesById)               = params.databaseFetcher->getRoutes(params.applicationShortname);
      std::tie(trips, tripIndexesById)                 = params.databaseFetcher->getTrips(params.applicationShortname);
      std::tie(forwardConnections, reverseConnections) = params.databaseFetcher->getConnections(params.applicationShortname, stopIndexesById, tripIndexesById);
      std::tie(footpaths, footpathsRanges)             = params.databaseFetcher->getFootpaths(params.applicationShortname, stopIndexesById);
    }
    else if (params.dataFetcherShortname == "cache")
    {
      std::tie(stops, stopIndexesById)                 = params.cacheFetcher->getStops(params.applicationShortname);
      std::tie(routes, routeIndexesById)               = params.cacheFetcher->getRoutes(params.applicationShortname);
      std::tie(trips, tripIndexesById)                 = params.cacheFetcher->getTrips(params.applicationShortname);
      std::tie(forwardConnections, reverseConnections) = params.cacheFetcher->getConnections(params.applicationShortname, stopIndexesById, tripIndexesById);
      std::tie(footpaths, footpathsRanges)             = params.cacheFetcher->getFootpaths(params.applicationShortname, stopIndexesById);
    }
    else if (params.dataFetcherShortname == "gtfs")
    {
      std::tie(stops, stopIndexesById)                 = params.gtfsFetcher->getStops(params.applicationShortname);
      std::tie(routes, routeIndexesById)               = params.gtfsFetcher->getRoutes(params.applicationShortname);
      std::tie(trips, tripIndexesById)                 = params.gtfsFetcher->getTrips(params.applicationShortname);
      std::tie(forwardConnections, reverseConnections) = params.gtfsFetcher->getConnections(params.applicationShortname, stopIndexesById, tripIndexesById);
      std::tie(footpaths, footpathsRanges)             = params.gtfsFetcher->getFootpaths(params.applicationShortname, stopIndexesById);
    }
    else if (params.dataFetcherShortname == "csv")
    {
      std::tie(stops, stopIndexesById)                 = params.csvFetcher->getStops(params.applicationShortname);
      std::tie(routes, routeIndexesById)               = params.csvFetcher->getRoutes(params.applicationShortname);
      std::tie(trips, tripIndexesById)                 = params.csvFetcher->getTrips(params.applicationShortname);
      std::tie(forwardConnections, reverseConnections) = params.csvFetcher->getConnections(params.applicationShortname, stopIndexesById, tripIndexesById);
      std::tie(footpaths, footpathsRanges)             = params.csvFetcher->getFootpaths(params.applicationShortname, stopIndexesById);
    }
    
    std::cout << "preparing stops tentative times, trips enter connections and journeys..." << std::endl;
    
    stopsTentativeTime    = std::vector<int>(stops.size());
    stopsAccessTravelTime = std::vector<int>(stops.size());
    stopsEgressTravelTime = std::vector<int>(stops.size());
    tripsEnterConnection  = std::vector<int>(trips.size());
    tripsEnterConnectionTransferTravelTime = std::vector<int>(trips.size());
    tripsEnabled          = std::vector<int>(trips.size());
    journeys              = std::vector<std::tuple<int,int,int,int,int>>(stops.size());
    
  }
  
}
