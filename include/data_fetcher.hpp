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

namespace TrRouting
{
  
  class DataFetcher
  {
  
    public:
    
      DataFetcher() {}
      
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
      
  };
  
}

#endif // TR_DATA_FETCHER
