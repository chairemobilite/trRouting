#ifndef TR_OSRM_FETCHER
#define TR_OSRM_FETCHER

#include "parameters.hpp"
#include "point.hpp"
#include "stop.hpp"

namespace TrRouting
{
  
  class OsrmFetcher
  {
  
  public:
    
    OsrmFetcher() {}
    OsrmFetcher(std::string applicationShortname) {
      
    }
    
    static std::vector<std::pair<int,int>> getAccessibleStopsFootpathsFromPoint(const Point point, const std::vector<Stop> stops, Parameters& params, std::string mode, int maxTravelTimeSeconds);
    
  };
    
}

#endif // TR_OSRM_FETCHER
