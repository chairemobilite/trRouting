#ifndef TR_DATA_FETCHER
#define TR_DATA_FETCHER

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
#include "stop.hpp"
#include "route.hpp"
#include "point.hpp"
#include "trip.hpp"
#include "tuple_boost_serialize.hpp"
#include "toolbox.hpp"

namespace TrRouting
{
  
  class DataFetcher
  {
  
    public:
    
      DataFetcher() {};
      
      template<class T>
      static void saveToCacheFile(std::string applicationShortname, T& data, std::string cacheFileName) {
        std::ofstream oCacheFile;
        oCacheFile.open(applicationShortname + "_" + cacheFileName + ".cache", std::ios::out | std::ios::trunc | std::ios::binary);
        boost::archive::binary_oarchive oarch(oCacheFile);
        oarch << data;
        oCacheFile.close();
      }
      
      static bool cacheFileExists(std::string applicationShortname, std::string cacheFileName) {
        std::ifstream iCacheFile;
        bool notEmpty = false;
        iCacheFile.open(applicationShortname + "_" + cacheFileName + ".cache", std::ios::in | std::ios::binary | std::ios::ate);
        notEmpty = iCacheFile.tellg() > 0;
        iCacheFile.close();
        return notEmpty;
      }
      
      virtual const std::pair<std::vector<Stop>, std::map<unsigned long long, int>> getStops(std::string applicationShortname)
      {
        std::vector<Stop> stops;
        std::map<unsigned long long, int> stopIndexesById;
        return std::make_pair(stops, stopIndexesById);
      }
      
      virtual const std::pair<std::vector<Route>, std::map<unsigned long long, int>> getRoutes(std::string applicationShortname)
      {
        std::vector<Route> routes;
        std::map<unsigned long long, int> routeIndexesById;
        return std::make_pair(routes, routeIndexesById);
      }
      
      virtual const std::pair<std::vector<Trip>, std::map<unsigned long long, int>> getTrips(std::string applicationShortname)
      {
        std::vector<Trip> trips;
        std::map<unsigned long long, int> tripIndexesById;
        return std::make_pair(trips, tripIndexesById);
      }
      
      virtual const std::pair<std::vector<std::tuple<int,int,int,int,int,short,short>>, std::vector<std::tuple<int,int,int,int,int,short,short>>> getConnections(std::string applicationShortname, std::map<unsigned long long, int> stopIndexesById, std::map<unsigned long long, int> tripIndexesById)
      {
        std::vector<std::tuple<int,int,int,int,int,short,short>> forwardConnections;
        std::vector<std::tuple<int,int,int,int,int,short,short>> reverseConnections;
        return std::make_pair(forwardConnections, reverseConnections);
      }
      
      virtual const std::pair<std::vector<std::tuple<int,int,int>>, std::vector<std::pair<int,int>>> getFootpaths(std::string applicationShortname, std::map<unsigned long long, int> stopIndexesById)
      {
        std::vector<std::tuple<int,int,int>> footpaths;
        std::vector<std::pair<int,int>>      footpathsRanges;
        return std::make_pair(footpaths, footpathsRanges);
      }

      
  };
  
}

#endif // TR_DATA_FETCHER
