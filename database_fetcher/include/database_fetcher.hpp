#ifndef TR_DATABASE_FETCHER
#define TR_DATABASE_FETCHER

#include <pqxx/pqxx> 
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <math.h>
#include <boost/algorithm/string.hpp>
#include <cereal/archives/binary.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <stdlib.h>

#include "calculation_time.hpp"
#include "parameters.hpp"
#include "stop.hpp"
#include "route.hpp"
#include "point.hpp"
#include "trip.hpp"

namespace TrRouting
{
  
  class DatabaseFetcher
  {
  
  public:
    
    static const std::map<std::string,int> getPickUpTypes (std::string applicationShortname);
    static const std::map<std::string,int> getDropOffTypes(std::string applicationShortname);
    //static const std::map<unsigned long long,PathStopSequence> getPathStopSequencesById(std::string applicationShortname, std::string dataFetcher);
    static const std::map<unsigned long long,Stop>  getStops(std::string applicationShortname, std::string dataFetcher, int maxTimeValue = 9999);
    static const std::map<unsigned long long,Route> getRoutes(std::string applicationShortname, std::string dataFetcher);
    static const std::map<unsigned long long,std::map<unsigned long long, int> > getFootpaths(std::string applicationShortname, std::string dataFetcher, int maxTransferWalkingTravelTimeMinutes, std::string transfersSqlWhereClause);
    static const std::map<unsigned long long,Connection> getConnectionsById(std::string applicationShortname, std::string dataFetcher, std::string connectionsSqlWhereClause, Parameters& params);
    static const std::map<unsigned long long,int> getNearestStopsIds(std::string applicationShortname, std::string dataFetcher, Point point, std::map<unsigned long long, Stop> stopsById, Parameters& params, std::string mode, int maxTravelTimeMinutes);
    static const std::map<std::pair<unsigned long long, unsigned long long>, double> getTravelTimeByStopsPair(std::string applicationShortname, std::string mode);
    static const std::pair<int, int> getTripTravelTimeAndDistance(Point startingPoint, Point endingPoint, std::string mode, Parameters& params);
    
    static pqxx::connection* getConnectionPtr();
    template<class T>
    static void saveToCacheFile(std::string applicationShortname, T& data, std::string cacheFileName);
    template<class T>
    static const T loadFromCacheFile(T& data, std::string applicationShortname, std::string cacheFileName);
    static bool isCacheFileNotEmpty(std::string applicationShortname, std::string cacheFileName);
    static void disconnect();
    static bool isConnectionOpen();
    static void setDbSetupStr(std::string customDbSetupStr);
    
  private:
    
    static std::string dbSetupStr;
    static pqxx::connection* pgConnectionPtr;
    
  };
  
}

#endif // TR_DATABASE_FETCHER
