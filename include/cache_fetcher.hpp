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
#include <boost/uuid/nil_generator.hpp>
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
#include "network.hpp"
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

    /**
     * Save data to a cache file. The cache file path can be obtained using the
     * getFilePath function
     */
    static void saveToCapnpCacheFile(T& data, std::string cacheFilePath);
    /**
     * Returns whether a cache file exists. The cache file path can be obtained
     * using the getFilePath function
     */
    static bool capnpCacheFileExists(         std::string cacheFilePath);
    /**
     * Get the number of cache files in a directory. The cache file path can be
     * obtained using the getFilePath function
     */
    static int  getCacheFilesCount  (         std::string cacheFilePath);
    /**
     * Get the complete file path from the parameters and configuration
     * 
     * Returns the complete file path in the cache
     */
    static std::string getFilePath  (         std::string cacheFilePath, Parameters& params, std::string customPath = "");
    
    const std::pair<std::vector<Mode>, std::map<std::string, int>> getModes();
    
    void getDataSources(
      std::vector<std::unique_ptr<DataSource>>& ts, 
      std::map<boost::uuids::uuid, int>& tIndexesById, 
      Parameters& params,
      std::string customPath = ""
    );

    void getHouseholds(
      std::vector<std::unique_ptr<Household>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById, 
      std::map<boost::uuids::uuid, int>& dataSourceIndexesByUuid, 
      std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
      Parameters& params,
      std::string customPath = ""
    );

    void getPersons(
      std::vector<std::unique_ptr<Person>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById, 
      std::map<boost::uuids::uuid, int>& dataSourceIndexesByUuid,
      std::map<boost::uuids::uuid, int>& householdIndexesByUuid, 
      std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
      Parameters& params,
      std::string customPath = ""
    );

    void getOdTrips(
      std::vector<std::unique_ptr<OdTrip>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById, 
      std::map<boost::uuids::uuid, int>& dataSourceIndexesByUuid,
      std::map<boost::uuids::uuid, int>& householdIndexesByUuid, 
      std::map<boost::uuids::uuid, int>& personIndexesByUuid,
      std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
      Parameters& params,
      std::string customPath = ""
    );

    void getPlaces(
      std::vector<std::unique_ptr<Place>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById, 
      std::map<boost::uuids::uuid, int>& dataSourceIndexesByUuid,
      std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
      Parameters& params,
      std::string customPath = ""
    );

    /**
     * Read the agencies cache file and fill the agencies vector.
     * 
     * @return 0 in case of success, values below 0 when errors occurred:
     * -EBADMSG if deserialization did not work
     * -ENOENT if the file does not exist
     * -EINVAL For any other data related error
     * -(error codes from the open system call)
     */
    int getAgencies(
      std::vector<std::unique_ptr<Agency>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById,
      Parameters& params,
      std::string customPath = ""
    );

    void getServices(
      std::vector<std::unique_ptr<Service>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById,
      Parameters& params,
      std::string customPath = ""
    );

    void getStations(
      std::vector<std::unique_ptr<Station>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById,
      Parameters& params,
      std::string customPath = ""
    );

    void getNodes(
      std::vector<std::unique_ptr<Node>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById,
      std::map<boost::uuids::uuid, int>& stationIndexesById,
      Parameters& params,
      std::string customPath = ""
    );

    void getLines(
      std::vector<std::unique_ptr<Line>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById,
      std::map<boost::uuids::uuid, int>& agencyIndexesByUuid,
      std::map<std::string, int>& modeIndexesByShortname,
      Parameters& params,
      std::string customPath = ""
    );

    void getPaths(
      std::vector<std::unique_ptr<Path>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById,
      std::map<boost::uuids::uuid, int>& lineIndexesByUuid,
      std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
      Parameters& params,
      std::string customPath = ""
    );

    void getScenarios(
      std::vector<std::unique_ptr<Scenario>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById,
      std::map<boost::uuids::uuid, int>& serviceIndexesByUuid,
      std::map<boost::uuids::uuid, int>& lineIndexesByUuid,
      std::map<boost::uuids::uuid, int>& agencyIndexesByUuid,
      std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
      std::map<std::string, int>& modeIndexesByShortname,
      Parameters& params,
      std::string customPath = ""
    );

    void getNetworks(
      std::vector<std::unique_ptr<Network>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesByUuid,
      std::map<boost::uuids::uuid, int>& agencyIndexesByUuid,
      std::map<boost::uuids::uuid, int>& serviceIndexesByUuid,
      std::map<boost::uuids::uuid, int>& scenarioIndexesByUuid,
      Parameters& params,
      std::string customPath = ""
    );

    void getSchedules(
      std::vector<std::unique_ptr<Trip>>& trips,
      std::vector<std::unique_ptr<Line>>& lines,
      std::vector<std::unique_ptr<Path>>& paths,
      std::map<boost::uuids::uuid, int>& tripIndexesByUuid,
      std::map<boost::uuids::uuid, int>& serviceIndexesByUuid,
      std::map<boost::uuids::uuid, int>& lineIndexesByUuid,
      std::map<boost::uuids::uuid, int>& pathIndexesByUuid,
      std::map<boost::uuids::uuid, int>& agencyIndexesByUuid,
      std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
      std::map<std::string, int>& modeIndexesByShortname,
      std::vector<std::vector<std::unique_ptr<int>>>&   tripConnectionDepartureTimes,
      std::vector<std::vector<std::unique_ptr<float>>>& tripConnectionDemands,
      std::vector<std::shared_ptr<std::tuple<int,int,int,int,int,short,short,int,int,int,short,short>>>& forwardConnections, 
      std::vector<std::shared_ptr<std::tuple<int,int,int,int,int,short,short,int,int,int,short,short>>>& reverseConnections,
      Parameters& params,
      std::string customPath = ""
    );
        
  private:
    
  };
    
}

#endif // TR_CACHE_FETCHER
