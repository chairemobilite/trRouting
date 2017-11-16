#include "data_fetcher.hpp"
#include "gtfs_fetcher.hpp"

namespace TrRouting
{
  
  const std::pair<std::vector<Stop>, std::map<unsigned long long, int>> GtfsFetcher::getStops(std::string applicationShortname)
  {
    std::vector<Stop> stops;
    std::map<unsigned long long, int> stopIndexesById;
    return std::make_pair(stops, stopIndexesById);
  }
  
  const std::pair<std::vector<Route>, std::map<unsigned long long, int>> GtfsFetcher::getRoutes(std::string applicationShortname)
  {
    std::vector<Route> routes;
    std::map<unsigned long long, int> routeIndexesById;
    return std::make_pair(routes, routeIndexesById);
  }
  
  const std::pair<std::vector<Trip>, std::map<unsigned long long, int>> GtfsFetcher::getTrips(std::string applicationShortname)
  {
    std::vector<Trip> trips;
    std::map<unsigned long long, int> tripIndexesById;
    return std::make_pair(trips, tripIndexesById);
  }
  
  const std::pair<std::vector<std::tuple<int,int,int,int,int,short,short>>, std::vector<std::tuple<int,int,int,int,int,short,short>>> GtfsFetcher::getConnections(std::string applicationShortname, std::map<unsigned long long, int> stopIndexesById, std::map<unsigned long long, int> tripIndexesById)
  {
    std::vector<std::tuple<int,int,int,int,int,short,short>> forwardConnections;
    std::vector<std::tuple<int,int,int,int,int,short,short>> reverseConnections;
    return std::make_pair(forwardConnections, reverseConnections);
    
  }
  
  const std::pair<std::vector<std::tuple<int,int,int>>, std::vector<std::pair<int,int>>> GtfsFetcher::getFootpaths(std::string applicationShortname, std::map<unsigned long long, int> stopIndexesById)
  {
    std::vector<std::tuple<int,int,int>> footpaths;
    std::vector<std::pair<int,int>>      footpathsRanges;
    return std::make_pair(footpaths, footpathsRanges);
    
  }
  
  
}
