#ifndef TR_GTFS_FETCHER
#define TR_GTFS_FETCHER

#include "data_fetcher.hpp"

namespace TrRouting
{
  
  class GtfsFetcher: public DataFetcher
  {
  
  public:
    
    GtfsFetcher() {}
    GtfsFetcher(std::string gtfsDirectoryPath) {
      
    }
    
  private:
    
  };
    
}

#endif // TR_GTFS_FETCHER
