#ifndef TR_CACHE_FETCHER
#define TR_CACHE_FETCHER

#include "data_fetcher.hpp"
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include "tuple_boost_serialize.hpp"

namespace TrRouting
{
  
  class CacheFetcher: public DataFetcher
  {
  
  public:
    
    CacheFetcher() {}
    CacheFetcher(std::string applicationShortname) {
      
    }
    
    template<class T>
    static const T loadFromCacheFile(T& data, std::string applicationShortname, std::string cacheFileName) {
      std::ifstream iCacheFile;
      iCacheFile.open(applicationShortname + "_" + cacheFileName + ".cache", std::ios::in | std::ios::binary);
      boost::archive::binary_iarchive iarch(iCacheFile);
      iarch >> data;
      iCacheFile.close();
      return data;
    }
    
    const std::pair<std::vector<Stop> , std::map<unsigned long long, int>> getStops( std::string applicationShortname);
    const std::pair<std::vector<Route>, std::map<unsigned long long, int>> getRoutes(std::string applicationShortname);
    const std::pair<std::vector<Trip> , std::map<unsigned long long, int>> getTrips( std::string applicationShortname);
    const std::pair<std::vector<std::tuple<int,int,int,int,int,short,short>>, std::vector<std::tuple<int,int,int,int,int,short,short>>> getConnections(std::string applicationShortname, std::map<unsigned long long, int> stopIndexesById, std::map<unsigned long long, int> tripIndexesById);
    const std::pair<std::vector<std::tuple<int,int,int>>, std::vector<std::pair<int,int>>> getFootpaths(std::string applicationShortname, std::map<unsigned long long, int> stopIndexesById);
    
  private:
    
  };
    
}

#endif // TR_CACHE_FETCHER
