#ifndef TR_CACHE_FETCHER
#define TR_CACHE_FETCHER

#include "data_fetcher.hpp"

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
      
    }
    
    static bool isCacheFileNotEmpty(std::string applicationShortname, std::string cacheFileName) {
      
    }
    
  private:
    
  };
    
}

#endif // TR_CACHE_FETCHER
