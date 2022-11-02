#ifndef TR_CACHE_FETCHER
#define TR_CACHE_FETCHER

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <boost/uuid/uuid.hpp>

#include "data_fetcher.hpp"

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
#include "mode.hpp"
#include "trip.hpp"

namespace TrRouting
{

  class CacheFetcher : public DataFetcher
  {
  
  public:
    
    CacheFetcher(const std::string &cacheDir);
    virtual ~CacheFetcher();
    
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
    std::string getFilePath  (         std::string cacheFilePath, std::string customPath = "");
    
    virtual const std::map<std::string, Mode> getModes();

    /** Refer to the base class for these functions documentations */
    virtual int getDataSources(
      std::map<boost::uuids::uuid, DataSource>& ts,
      std::string customPath = ""
    );

    virtual int getPersons(
      std::map<boost::uuids::uuid, Person>& ts,
      const std::map<boost::uuids::uuid, DataSource>& dataSources,
      std::string customPath = ""
    );

    virtual int getOdTrips(
      std::map<boost::uuids::uuid, OdTrip>& ts,
      const std::map<boost::uuids::uuid, DataSource>& dataSources,
      const std::map<boost::uuids::uuid, Person>& persons,
      const std::map<boost::uuids::uuid, Node>& nodes,
      std::string customPath = ""
    );

    virtual int getAgencies(
      std::map<boost::uuids::uuid, Agency>& ts,
      std::string customPath = ""
    );

    virtual int getServices(
      std::map<boost::uuids::uuid, Service>& ts,
      std::string customPath = ""
    );

    virtual int getNodes(
      std::map<boost::uuids::uuid, Node>& ts,
      std::string customPath = ""
    );

    virtual int getLines(
      std::map<boost::uuids::uuid, Line>& ts,
      const std::map<boost::uuids::uuid, Agency>& agencies,
      const std::map<std::string, Mode>& modes,
      std::string customPath = ""
    );

    virtual int getPaths(
      std::map<boost::uuids::uuid, Path>& ts,
      const std::map<boost::uuids::uuid, Line>& lines,
      const std::map<boost::uuids::uuid, Node>& nodes,
      std::string customPath = ""
    );

    virtual int getScenarios(
      std::map<boost::uuids::uuid, Scenario>& ts,
      const std::map<boost::uuids::uuid, Service>& services,
      const std::map<boost::uuids::uuid, Line>& lines,
      const std::map<boost::uuids::uuid, Agency>& agencies,
      const std::map<boost::uuids::uuid, Node>& nodes,
      const std::map<std::string, Mode>& modes,
      std::string customPath = ""
    );

    virtual int getSchedules(
      std::vector<std::unique_ptr<Trip>>& trips,
      const std::map<boost::uuids::uuid, Line>& lines,
      std::map<boost::uuids::uuid, Path>& paths,
      std::map<boost::uuids::uuid, int>& tripIndexesByUuid,
      const std::map<boost::uuids::uuid, Service>& services,
      std::vector<std::vector<std::unique_ptr<int>>>&   tripConnectionDepartureTimes,
      std::vector<std::shared_ptr<ConnectionTuple>>& connections,
      std::string customPath = ""
    );
        
  private:
    std::string cacheDirectoryPath;
  };
    
}

#endif // TR_CACHE_FETCHER
