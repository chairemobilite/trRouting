#ifndef TR_CACHE_FETCHER
#define TR_CACHE_FETCHER

#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <math.h>
#include <fcntl.h>
#include <boost/uuid/uuid.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
//#include <boost/algorithm/string.hpp>
//#include <boost/property_tree/ptree.hpp>
//#include <boost/property_tree/json_parser.hpp>
//#include <boost/tokenizer.hpp>
//#include <boost/filesystem.hpp>
//#include <boost/lexical_cast.hpp>
//#include <boost/asio/ip/tcp.hpp>
//#include <stdlib.h>
#include <capnp/message.h>
#include <capnp/serialize-packed.h>

#include "calculation_time.hpp"

//#include "tuple_boost_serialize.hpp"
#include "toolbox.hpp"
#include "parameters.hpp"
#include "agency.hpp"
#include "line.hpp"
#include "node.hpp"
#include "service.hpp"
#include "scenario.hpp"
#include "path.hpp"
#include "block.hpp"
#include "stop.hpp"
#include "mode.hpp"
#include "od_trip.hpp"
#include "station.hpp"
#include "trip.hpp"

namespace TrRouting
{
  
  class CacheFetcher
  {
  
  public:
    
    CacheFetcher() {}
    CacheFetcher(std::string projectShortname) {
      
    }
    
    template<class T>
    static void saveToCapnpCacheFile(T& data, std::string cacheFileName, Parameters& params) {
      std::ofstream oCacheFile;
      oCacheFile.open(params.cacheDirectoryPath + params.projectShortname + "/" + cacheFileName + ".capnpbin", std::ios::out | std::ios::trunc | std::ios::binary);
      oCacheFile.close();
      int fd = open((params.projectShortname + "/" + cacheFileName + ".capnpbin").c_str(), O_WRONLY);
      ::capnp::writePackedMessageToFd(fd, data);
      close(fd);
    }
  
    static bool capnpCacheFileExists(std::string cacheFileName, Parameters& params) {
      std::ifstream iCacheFile;
      bool notEmpty = false;
      iCacheFile.open(params.cacheDirectoryPath + params.projectShortname + "/" + cacheFileName + ".capnpbin", std::ios::in | std::ios::binary | std::ios::ate);
      notEmpty = iCacheFile.tellg() > 0;
      iCacheFile.close();
      return notEmpty;
    }
    
    const std::pair<std::vector<Mode>    , std::map<std::string       , int>> getModes();
    const std::pair<std::vector<Service> , std::map<boost::uuids::uuid, int>> getServices(Parameters& params);
    const std::pair<std::vector<Scenario>, std::map<boost::uuids::uuid, int>> getScenarios(std::map<boost::uuids::uuid, int> serviceIndexesByUuid, Parameters& params);
    const std::pair<std::vector<Station> , std::map<boost::uuids::uuid, int>> getStations(Parameters& params);
    const std::pair<std::vector<Node>    , std::map<boost::uuids::uuid, int>> getNodes(std::map<boost::uuids::uuid, int> stationIndexesByUuid, Parameters& params);
    const std::vector<Node>                                                   getNodeFootpaths(std::vector<Node> nodes, std::map<boost::uuids::uuid, int> nodeIndexesByUuid, Parameters& params);
    const std::pair<std::vector<Agency>  , std::map<boost::uuids::uuid, int>> getAgencies(Parameters& params);
    const std::pair<std::vector<Line>    , std::map<boost::uuids::uuid, int>> getLines(std::map<boost::uuids::uuid, int> agencyIndexesByUuid, std::map<std::string, int> modeIndexesByShortname, Parameters& params);
    const std::pair<std::vector<Path>    , std::map<boost::uuids::uuid, int>> getPaths(std::map<boost::uuids::uuid, int> lineIndexesByUuid, std::map<boost::uuids::uuid, int> nodeIndexesByUuid, Parameters& params);
    const std::tuple<std::vector<Trip>   , std::map<boost::uuids::uuid, int>, std::vector<Block>, std::map<boost::uuids::uuid, int>, std::vector<std::tuple<int,int,int,int,int,short,short,int>>, std::vector<std::tuple<int,int,int,int,int,short,short,int>>> getTripsAndConnections(std::map<boost::uuids::uuid, int> agencyIndexesByUuid, std::vector<Line> lines, std::map<boost::uuids::uuid, int> lineIndexesByUuid, std::vector<Path> paths, std::map<boost::uuids::uuid, int> pathIndexesByUuid, std::map<boost::uuids::uuid, int> nodeIndexesByUuid, std::map<boost::uuids::uuid, int> serviceIndexesByUuid, Parameters& params);

    //const std::vector<std::pair<int,int>> getOdTripFootpaths(std::string projectShortname, Parameters& params);
    //const std::pair<std::vector<OdTrip>, std::map<unsigned long long, int>> getOdTrips(std::string projectShortname, std::vector<Stop> stops, Parameters& params);
    
  private:
    
  };
    
}

#endif // TR_CACHE_FETCHER
