#ifndef TR_GTFS_FETCHER
#define TR_GTFS_FETCHER

#include "data_fetcher.hpp"

namespace TrRouting
{
  
  class GtfsFetcher
  {
  
  public:
    
    GtfsFetcher() {}
    GtfsFetcher(std::string gtfsDirectoryPath) {
      
    }
    
    const std::pair<std::vector<Stop> , std::map<unsigned long long, int>> getStops( std::string applicationShortname);
    const std::pair<std::vector<Route>, std::map<unsigned long long, int>> getRoutes(std::string applicationShortname);
    const std::pair<std::vector<Trip> , std::map<unsigned long long, int>> getTrips( std::string applicationShortname);
    const std::pair<std::vector<std::tuple<int,int,int,int,int,short,short>>, std::vector<std::tuple<int,int,int,int,int,short,short>>> getConnections(std::string applicationShortname, std::map<unsigned long long, int> stopIndexesById, std::map<unsigned long long, int> tripIndexesById);
    const std::pair<std::vector<std::tuple<int,int,int>>, std::vector<std::pair<int,int>>> getFootpaths(std::string applicationShortname, std::map<unsigned long long, int> stopIndexesById);
    
  };
  
}

#endif // TR_GTFS_FETCHER
