#ifndef TR_OSRM_FETCHER
#define TR_OSRM_FETCHER

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

#include "forward_declarations.hpp"
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
