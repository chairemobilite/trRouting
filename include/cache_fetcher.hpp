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
#include "block.hpp"
#include "stop.hpp"
#include "mode.hpp"
#include "station.hpp"
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
      std::vector<std::unique_ptr<Person>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById,
      const std::map<boost::uuids::uuid, DataSource>& dataSources,
      std::string customPath = ""
    );

    virtual int getOdTrips(
      std::vector<std::unique_ptr<OdTrip>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById, 
      const std::map<boost::uuids::uuid, DataSource>& dataSources,
      const std::map<boost::uuids::uuid, int>& personIndexesByUuid,
      const std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
      std::string customPath = ""
    );

    virtual int getAgencies(
      std::vector<std::unique_ptr<Agency>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById,
      std::string customPath = ""
    );

    virtual int getServices(
      std::vector<std::unique_ptr<Service>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById,
      std::string customPath = ""
    );

    virtual int getStations(
      std::vector<std::unique_ptr<Station>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById,
      std::string customPath = ""
    );

    virtual int getNodes(
      std::vector<std::unique_ptr<Node>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById,
      const std::map<boost::uuids::uuid, int>& stationIndexesById,
      std::string customPath = ""
    );

    virtual int getLines(
      std::vector<std::unique_ptr<Line>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById,
      const std::map<boost::uuids::uuid, int>& agencyIndexesByUuid,
      const std::map<std::string, Mode>& modes,
      std::string customPath = ""
    );

    virtual int getPaths(
      std::vector<std::unique_ptr<Path>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById,
      const std::map<boost::uuids::uuid, int>& lineIndexesByUuid,
      const std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
      std::string customPath = ""
    );

    virtual int getScenarios(
      std::vector<std::unique_ptr<Scenario>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById,
      const std::map<boost::uuids::uuid, int>& serviceIndexesByUuid,
      const std::map<boost::uuids::uuid, int>& lineIndexesByUuid,
      const std::map<boost::uuids::uuid, int>& agencyIndexesByUuid,
      const std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
      const std::map<std::string, Mode>& modes,
      std::string customPath = ""
    );

    virtual int getSchedules(
      std::vector<std::unique_ptr<Trip>>& trips,
      const std::vector<std::unique_ptr<Line>>& lines,
      std::vector<std::unique_ptr<Path>>& paths,
      std::map<boost::uuids::uuid, int>& tripIndexesByUuid,
      const std::map<boost::uuids::uuid, int>& serviceIndexesByUuid,
      const std::map<boost::uuids::uuid, int>& lineIndexesByUuid,
      const std::map<boost::uuids::uuid, int>& pathIndexesByUuid,
      const std::map<boost::uuids::uuid, int>& agencyIndexesByUuid,
      const std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
      std::vector<std::vector<std::unique_ptr<int>>>&   tripConnectionDepartureTimes,
      std::vector<std::vector<std::unique_ptr<float>>>& tripConnectionDemands,
      std::vector<std::shared_ptr<std::tuple<int,int,int,int,int,short,short,int,int,int,short,short>>>& connections, 
      std::string customPath = ""
    );
        
  private:
    std::string cacheDirectoryPath;
  };
    
}

#endif // TR_CACHE_FETCHER
