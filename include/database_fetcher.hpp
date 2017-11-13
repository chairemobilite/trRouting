#ifndef TR_DATABASE_FETCHER
#define TR_DATABASE_FETCHER

#include "data_fetcher.hpp"

namespace TrRouting
{
  
  class DatabaseFetcher: public DataFetcher
  {
  
  public:
    
    DatabaseFetcher() {}
    DatabaseFetcher(std::string customDbSetupStr) {
      if (pgConnectionPtr == NULL)
      {
        pgConnectionPtr = new pqxx::connection(customDbSetupStr);
      }
    }
    
    //static const std::map<unsigned long long,PathStopSequence> getPathStopSequencesById(std::string applicationShortname, std::string dataFetcher);
    //static const std::map<unsigned long long,Stop>  getStops(std::string applicationShortname, std::string dataFetcher, int maxTimeValue = 9999);
    //static const std::map<unsigned long long,Route> getRoutes(std::string applicationShortname, std::string dataFetcher);
    //static const std::map<unsigned long long,std::map<unsigned long long, int> > getFootpaths(std::string applicationShortname, std::string dataFetcher, int maxTransferWalkingTravelTimeMinutes, std::string transfersSqlWhereClause);
    //static const std::map<unsigned long long,Connection> getConnectionsById(std::string applicationShortname, std::string dataFetcher, std::string connectionsSqlWhereClause, Parameters& params);
    //static const std::map<unsigned long long,int> getNearestStopsIds(std::string applicationShortname, std::string dataFetcher, Point point, std::map<unsigned long long, Stop> stopsById, Parameters& params, std::string mode, int maxTravelTimeMinutes);
    //static const std::map<std::pair<unsigned long long, unsigned long long>, double> getTravelTimeByStopsPair(std::string applicationShortname, std::string mode);
    //static const std::pair<int, int> getTripTravelTimeAndDistance(Point startingPoint, Point endingPoint, std::string mode, Parameters& params);
    
    void disconnect();
    bool isConnectionOpen();
    
  private:
    
    pqxx::connection* pgConnectionPtr;
    
  };
    
}

#endif // TR_DATABASE_FETCHER
