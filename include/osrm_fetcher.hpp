#ifndef TR_OSRM_FETCHER
#define TR_OSRM_FETCHER

#include <boost/algorithm/string.hpp>
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
#include "node.hpp"
#include "parameters.hpp"

using namespace osrm;

namespace TrRouting
{
  class OsrmFetcher
  {
  
  public:
    
    OsrmFetcher() {}
    OsrmFetcher(std::string projectShortname) {
      
    }
    
    static std::vector<std::pair<int,int>> getAccessibleNodesFootpathsFromPoint(const Point point, const std::vector<Node> nodes, std::string mode, Parameters& params, bool reversed = false);
    
  };
  
}

#endif // TR_OSRM_FETCHER
