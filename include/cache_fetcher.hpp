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
#include "data_source.hpp"
#include "household.hpp"
#include "person.hpp"
#include "od_trip.hpp"
#include "place.hpp"
#include "service.hpp"
#include "scenario.hpp"
#include "path.hpp"
#include "block.hpp"
#include "stop.hpp"
#include "mode.hpp"
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

    static void saveToCapnpCacheFile(T& data, std::string cacheFilePath, Parameters& params, std::string customPath = "");
    static bool capnpCacheFileExists(         std::string cacheFilePath, Parameters& params, std::string customPath = "");
    static int  getCacheFilesCount  (         std::string cacheFilePath, Parameters& params, std::string customPath = "");
    static std::string getFilePath  (         std::string cacheFilePath, Parameters& params, std::string customPath = "");
    
    const std::pair<std::vector<Mode>, std::map<std::string, int>> getModes();
    
    void getDataSources(
      std::vector<std::unique_ptr<DataSource>>& ts, 
      std::map<boost::uuids::uuid, int>& tIndexesById, 
      Parameters& params,
      std::string customPath
    );

    void getHouseholds (
      std::vector<std::unique_ptr<Household>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById, 
      std::map<boost::uuids::uuid, int>& dataSourceIndexesByUuid, 
      Parameters& params,
      std::string customPath
    );

    void getPersons    (
      std::vector<std::unique_ptr<Person>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById, 
      std::map<boost::uuids::uuid, int>& dataSourceIndexesByUuid,
      std::map<boost::uuids::uuid, int>& householdIndexesByUuid, 
      Parameters& params,
      std::string customPath
    );

    void getAgencies   (
      std::vector<std::unique_ptr<Agency>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById,
      Parameters& params,
      std::string customPath
    );
    
    const std::pair<std::vector<Service>   , std::map<boost::uuids::uuid, int>> getServices(Parameters& params, std::string customPath = "");
    const std::pair<std::vector<Person>    , std::map<boost::uuids::uuid, int>> getPersons(std::map<boost::uuids::uuid, int> dataSourceIndexesByUuid, std::map<boost::uuids::uuid, int> householdIndexesByUuid, Parameters& params, std::string customPath = "");
    const std::pair<std::vector<OdTrip>    , std::map<boost::uuids::uuid, int>> getOdTrips(std::map<boost::uuids::uuid, int> dataSourceIndexesByUuid, std::map<boost::uuids::uuid, int> householdIndexesByUuid, std::map<boost::uuids::uuid, int> personIndexesByUuid, Parameters& params, std::string customPath = "");
    const std::pair<std::vector<Place>     , std::map<boost::uuids::uuid, int>> getPlaces(std::map<boost::uuids::uuid, int> dataSourceIndexesByUuid, Parameters& params, std::string customPath = "");
    const std::pair<std::vector<Scenario>  , std::map<boost::uuids::uuid, int>> getScenarios(std::map<boost::uuids::uuid, int> serviceIndexesByUuid, std::map<boost::uuids::uuid, int> lineIndexesByUuid, std::map<boost::uuids::uuid, int> agencyIndexesByUuid, std::map<boost::uuids::uuid, int> nodeIndexesByUuid, std::map<std::string, int> modeIndexesByShortname, Parameters& params, std::string customPath = "");
    const std::pair<std::vector<Station>   , std::map<boost::uuids::uuid, int>> getStations(Parameters& params, std::string customPath = "");
    const std::pair<std::vector<Node>      , std::map<boost::uuids::uuid, int>> getNodes(std::map<boost::uuids::uuid, int> stationIndexesByUuid, Parameters& params, std::string customPath = "");
    const std::vector<Node>                                                     getNodeFootpaths(std::vector<Node> nodes, std::map<boost::uuids::uuid, int> nodeIndexesByUuid, Parameters& params, std::string customPath = "");
    const std::pair<std::vector<Line>      , std::map<boost::uuids::uuid, int>> getLines(std::map<boost::uuids::uuid, int> agencyIndexesByUuid, std::map<std::string, int> modeIndexesByShortname, Parameters& params, std::string customPath = "");
    const std::pair<std::vector<Path>      , std::map<boost::uuids::uuid, int>> getPaths(std::map<boost::uuids::uuid, int> lineIndexesByUuid, std::map<boost::uuids::uuid, int> nodeIndexesByUuid, Parameters& params, std::string customPath = "");
    const std::tuple<std::vector<Trip>     , std::map<boost::uuids::uuid, int>, std::vector<std::vector<int>>, std::vector<std::vector<float>>, std::vector<Block>, std::map<boost::uuids::uuid, int>, std::vector<std::tuple<int,int,int,int,int,short,short,int,int,int,short>>, std::vector<std::tuple<int,int,int,int,int,short,short,int,int,int,short>>> getTripsAndConnections(std::map<boost::uuids::uuid, int> agencyIndexesByUuid, std::vector<Line> lines, std::map<boost::uuids::uuid, int> lineIndexesByUuid, std::vector<Path> paths, std::map<boost::uuids::uuid, int> pathIndexesByUuid, std::map<boost::uuids::uuid, int> nodeIndexesByUuid, std::map<boost::uuids::uuid, int> serviceIndexesByUuid, Parameters& params);

    //const std::vector<std::pair<int,int>> getOdTripFootpaths(std::string projectShortname, Parameters& params);
    //const std::pair<std::vector<OdTrip>, std::map<unsigned long long, int>> getOdTrips(std::string projectShortname, std::vector<Stop> stops, Parameters& params);
    
  private:
    
  };
    
}

#endif // TR_CACHE_FETCHER
