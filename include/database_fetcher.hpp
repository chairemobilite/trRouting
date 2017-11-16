#ifndef TR_DATABASE_FETCHER
#define TR_DATABASE_FETCHER

#include "data_fetcher.hpp"

namespace TrRouting
{
  
  class DatabaseFetcher
  {
  
  public:
    
    DatabaseFetcher() {}
    DatabaseFetcher(std::string _customDbSetupStr) : customDbSetupStr(_customDbSetupStr) {
      pgConnectionPtr = NULL;
      getConnectionPtr();
    }
    
    pqxx::connection* getConnectionPtr();
    void disconnect();
    void openConnection();
    bool isConnectionOpen();
    
    const std::pair<std::vector<Stop> , std::map<unsigned long long, int>> getStops( std::string applicationShortname);
    const std::pair<std::vector<Route>, std::map<unsigned long long, int>> getRoutes(std::string applicationShortname);
    const std::pair<std::vector<Trip> , std::map<unsigned long long, int>> getTrips( std::string applicationShortname);
    const std::pair<std::vector<std::tuple<int,int,int,int,int,short,short>>, std::vector<std::tuple<int,int,int,int,int,short,short>>> getConnections(std::string applicationShortname, std::map<unsigned long long, int> stopIndexesById, std::map<unsigned long long, int> tripIndexesById);
    const std::pair<std::vector<std::tuple<int,int,int>>, std::vector<std::pair<int,int>>> getFootpaths(std::string applicationShortname, std::map<unsigned long long, int> stopIndexesById);
    //static const std::map<unsigned long long,int> getNearestStopsIds(std::string applicationShortname, std::string dataFetcher, Point point, std::map<unsigned long long, Stop> stopsById, Parameters& params, std::string mode, int maxTravelTimeMinutes);
    //static const std::map<std::pair<unsigned long long, unsigned long long>, double> getTravelTimeByStopsPair(std::string applicationShortname, std::string mode);
    //static const std::pair<int, int> getTripTravelTimeAndDistance(Point startingPoint, Point endingPoint, std::string mode, Parameters& params);
    
  private:
    
    pqxx::connection* pgConnectionPtr;
    std::string customDbSetupStr;
    
  };
    
}

#endif // TR_DATABASE_FETCHER
