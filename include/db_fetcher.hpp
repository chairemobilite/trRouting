#ifndef TR_DB_FETCHER
#define TR_DB_FETCHER

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
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <stdlib.h>

#include "calculation_time.hpp"
#include "connection.hpp"
#include "path_stop_sequence.hpp"
#include "parameters.hpp"
#include "stop.hpp"
#include "route.hpp"
#include "point.hpp"

namespace TrRouting
{
  
  class DbFetcher
  {
  
  public:
    
    static const std::map<std::string,int> getPickUpTypes (std::string applicationShortname);
    static const std::map<std::string,int> getDropOffTypes(std::string applicationShortname);
    static const std::map<unsigned long long,PathStopSequence> getPathStopSequencesById(std::string applicationShortname, std::string dataFetcher);
    static const std::map<unsigned long long,Stop>  getStopsById (std::string applicationShortname, std::string dataFetcher, int maxTimeValue = 9999);
    static const std::map<unsigned long long,Route> getRoutesById(std::string applicationShortname, std::string dataFetcher);
    static const std::map<unsigned long long,std::map<unsigned long long, int> > getTransferDurationsByStopId(std::string applicationShortname, std::string dataFetcher, int maxTransferWalkingTravelTimeMinutes, std::string transfersSqlWhereClause);
    static const std::map<unsigned long long,Connection> getConnectionsById(std::string applicationShortname, std::string dataFetcher, std::string connectionsSqlWhereClause, Parameters& params);
    static const std::map<unsigned long long,int> getNearestStopsIds(std::string applicationShortname, std::string dataFetcher, Point point, std::map<unsigned long long, Stop> stopsById, Parameters& params, std::string mode, int maxTravelTimeMinutes);
    static const std::map<std::pair<unsigned long long, unsigned long long>, double> getTravelTimeByStopsPair(std::string applicationShortname, std::string mode);
    
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

#endif // TR_DB_FETCHER
