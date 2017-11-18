#include "data_fetcher.hpp"
#include "cache_fetcher.hpp"

namespace TrRouting
{
  
  const std::pair<std::vector<Stop>, std::map<unsigned long long, int>> CacheFetcher::getStops(std::string applicationShortname)
  {
    std::vector<Stop> stops;
    std::map<unsigned long long, int> stopIndexesById;
    
    std::cout << "Fetching stops from cache..." << std::endl;
    if (DataFetcher::cacheFileExists(applicationShortname, "stops"))
    {
      stops = loadFromCacheFile(stops, applicationShortname, "stops");
    }
    else
    {
      std::cerr << "missing stops cache file!" << std::endl;
    }
    if (DataFetcher::cacheFileExists(applicationShortname, "stop_indexes"))
    {
      stopIndexesById = loadFromCacheFile(stopIndexesById, applicationShortname, "stop_indexes");
    }
    else
    {
      std::cerr << "missing stop_indexes cache file!" << std::endl;
    }
    return std::make_pair(stops, stopIndexesById);
  }
  
  const std::pair<std::vector<Route>, std::map<unsigned long long, int>> CacheFetcher::getRoutes(std::string applicationShortname)
  {
    std::vector<Route> routes;
    std::map<unsigned long long, int> routeIndexesById;
    
    std::cout << "Fetching routes from cache..." << std::endl;
    if (DataFetcher::cacheFileExists(applicationShortname, "routes"))
    {
      routes = loadFromCacheFile(routes, applicationShortname, "routes");
    }
    else
    {
      std::cerr << "missing routes cache file!" << std::endl;
    }
    if (DataFetcher::cacheFileExists(applicationShortname, "route_indexes"))
    {
      routeIndexesById = loadFromCacheFile(routeIndexesById, applicationShortname, "route_indexes");
    }
    else
    {
      std::cerr << "missing route_indexes cache file!" << std::endl;
    }
    return std::make_pair(routes, routeIndexesById);
  }
  
  const std::pair<std::vector<Trip>, std::map<unsigned long long, int>> CacheFetcher::getTrips(std::string applicationShortname)
  {
    std::vector<Trip> trips;
    std::map<unsigned long long, int> tripIndexesById;
    
    std::cout << "Fetching trips from cache..." << std::endl;
    if (DataFetcher::cacheFileExists(applicationShortname, "trips"))
    {
      trips = loadFromCacheFile(trips, applicationShortname, "trips");
    }
    else
    {
      std::cerr << "missing trips cache file!" << std::endl;
    }
    if (DataFetcher::cacheFileExists(applicationShortname, "trip_indexes"))
    {
      tripIndexesById = loadFromCacheFile(tripIndexesById, applicationShortname, "trip_indexes");
    }
    else
    {
      std::cerr << "missing trip_indexes cache file!" << std::endl;
    }
    return std::make_pair(trips, tripIndexesById);
  }
  
  const std::pair<std::vector<std::tuple<int,int,int,int,int,short,short,int>>, std::vector<std::tuple<int,int,int,int,int,short,short,int>>> CacheFetcher::getConnections(std::string applicationShortname, std::map<unsigned long long, int> stopIndexesById, std::map<unsigned long long, int> tripIndexesById)
  {
    std::vector<std::tuple<int,int,int,int,int,short,short,int>> forwardConnections;
    std::vector<std::tuple<int,int,int,int,int,short,short,int>> reverseConnections;
    
    std::cout << "Fetching connections from cache..." << std::endl;
    if (DataFetcher::cacheFileExists(applicationShortname, "connections_forward"))
    {
      forwardConnections = loadFromCacheFile(forwardConnections, applicationShortname, "connections_forward");
    }
    else
    {
      std::cerr << "missing connections_forward cache file!" << std::endl;
    }
    if (DataFetcher::cacheFileExists(applicationShortname, "connections_forward"))
    {
      reverseConnections = loadFromCacheFile(reverseConnections, applicationShortname, "connections_reverse");
    }
    else
    {
      std::cerr << "missing connections_reverse cache file!" << std::endl;
    }
    return std::make_pair(forwardConnections, reverseConnections);
    
  }
  
  const std::pair<std::vector<std::tuple<int,int,int>>, std::vector<std::pair<int,int>>> CacheFetcher::getFootpaths(std::string applicationShortname, std::map<unsigned long long, int> stopIndexesById)
  {
    std::vector<std::tuple<int,int,int>> footpaths;
    std::vector<std::pair<int,int>>      footpathsRanges;
    
    std::cout << "Fetching footpaths from cache..." << std::endl;
    if (DataFetcher::cacheFileExists(applicationShortname, "footpaths"))
    {
      footpaths = loadFromCacheFile(footpaths, applicationShortname, "footpaths");
    }
    else
    {
      std::cerr << "missing footpaths cache file!" << std::endl;
    }
    if (DataFetcher::cacheFileExists(applicationShortname, "footpaths_ranges"))
    {
      footpathsRanges = loadFromCacheFile(footpathsRanges, applicationShortname, "footpaths_ranges");
    }
    else
    {
      std::cerr << "missing footpaths_ranges cache file!" << std::endl;
    }
    return std::make_pair(footpaths, footpathsRanges);
    
  }
  
  
}
