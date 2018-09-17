#ifndef TR_OSRM_FETCHER
#define TR_OSRM_FETCHER

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

#include <exception>
#include <iostream>
#include <string>
#include <utility>
#include <cstdlib>

#include <osrm/osrm.hpp>
#include <osrm/status.hpp>
#include <osrm/json_container.hpp>
#include <osrm/engine_config.hpp>
#include <osrm/table_parameters.hpp>

#include "point.hpp"
#include "stop.hpp"
#include "parameters.hpp"

using namespace osrm;

namespace TrRouting
{
  class OsrmFetcher
  {
  
  public:
    
    OsrmFetcher() {}
    OsrmFetcher(std::string applicationShortname) {
      
    }
    
    static std::vector<std::pair<int,int>> getAccessibleStopsFootpathsFromPoint(const Point point, const std::vector<Stop> stops, std::string mode, Parameters& params);
    
  };
  
}

#endif // TR_OSRM_FETCHER
