#ifndef TR_DATA_FETCHER
#define TR_DATA_FETCHER

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <boost/uuid/uuid.hpp>

namespace TrRouting
{
  class DataSource;
  //class Household;  //TODO #167
  class Person;
  class OdTrip;
  //class Place;  //TODO #167
  class Agency;
  class Service;
  class Node;
  class Line;
  class Path;
  class Scenario;
  class Trip;
  class Mode;

  /** Base class for all data access mode (cache, gtfs, db, etc)
   */
  class DataFetcher
  {
  public:
      virtual const std::map<std::string, Mode> getModes() = 0;

      //TODO the customPath does not make much sense to a generic data access pattern (see issue #160)
    /**
     * Read the data sources cache file and fill the data sources vector.
     * 
     * @return 0 in case of success, values below 0 when errors occurred:
     * -EBADMSG if deserialization did not work
     * -ENOENT if the file does not exist
     * -EINVAL For any other data related error
     * -(error codes from the open system call)
     */
    virtual int getDataSources(
      std::map<boost::uuids::uuid, DataSource>& ts, 
      std::string customPath = "") = 0;

    /**
     * Read the households cache file and fill the households vector.
     * 
     * @return 0 in case of success, values below 0 when errors occurred:
     * -EBADMSG if deserialization did not work
     * -ENOENT if the file does not exist
     * -EINVAL For any other data related error
     * -(error codes from the open system call)
     */
    //TODO #167 Place/Household removed while refactoring
    // virtual int getHouseholds(
    //  std::vector<std::unique_ptr<Household>>& ts,
    //  std::map<boost::uuids::uuid, int>& tIndexesById,
    //  const std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
    //  std::string customPath = "") = 0;

    /**
     * Read the persons cache file and fill the persons vector.
     * 
     * @return 0 in case of success, values below 0 when errors occurred:
     * -EBADMSG if deserialization did not work
     * -ENOENT if the file does not exist
     * -EINVAL For any other data related error
     * -(error codes from the open system call)
     */
    virtual int getPersons(
      std::vector<std::unique_ptr<Person>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById, 
      const std::map<boost::uuids::uuid, DataSource>& dataSources,
      std::string customPath = ""
    ) = 0;

    /**
     * Read the odTrips cache file and fill the odTrips vector.
     * 
     * @return 0 in case of success, values below 0 when errors occurred:
     * -EBADMSG if deserialization did not work
     * -ENOENT if the file does not exist
     * -EINVAL For any other data related error
     * -(error codes from the open system call)
     */
    virtual int getOdTrips(
      std::vector<std::unique_ptr<OdTrip>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById, 
      const std::map<boost::uuids::uuid, DataSource>& dataSources,
      const std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
      const std::map<boost::uuids::uuid, int>& personIndexesByUuid,
      std::string customPath = ""
    ) = 0;

    /**
     * Read the places cache file and fill the places vector.
     * 
     * @return 0 in case of success, values below 0 when errors occurred:
     * -EBADMSG if deserialization did not work
     * -ENOENT if the file does not exist
     * -EINVAL For any other data related error
     * -(error codes from the open system call)
     */
    //TODO #167 Place/Household removed while refactoring
    //virtual int getPlaces(
    //  std::vector<std::unique_ptr<Place>>& ts,
    //  std::map<boost::uuids::uuid, int>& tIndexesById,
    //  const std::map<boost::uuids::uuid, int>& dataSourceIndexesByUuid,
    //  const std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
    //  std::string customPath = ""
    //) = 0;

    /**
     * Read the agencies cache file and fill the agencies vector.
     * 
     * @return 0 in case of success, values below 0 when errors occurred:
     * -EBADMSG if deserialization did not work
     * -ENOENT if the file does not exist
     * -EINVAL For any other data related error
     * -(error codes from the open system call)
     */
    virtual int getAgencies(
      std::map<boost::uuids::uuid, Agency>& ts,
      std::string customPath = ""
    ) = 0;

    /**
     * Read the services cache file and fill the services vector.
     * 
     * @return 0 in case of success, values below 0 when errors occurred:
     * -EBADMSG if deserialization did not work
     * -ENOENT if the file does not exist
     * -EINVAL For any other data related error
     * -(error codes from the open system call)
     */
    virtual int getServices(
      std::map<boost::uuids::uuid, Service>& ts,
      std::string customPath = ""
    ) = 0;

    /**
     * Read the nodes cache file and fill the nodes and stations vector.
     * 
     * @return 0 in case of success, values below 0 when errors occurred:
     * -EBADMSG if deserialization did not work
     * -ENOENT if the file does not exist
     * -EINVAL For any other data related error
     * -(error codes from the open system call)
     */
    virtual int getNodes(
      std::vector<std::unique_ptr<Node>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById,
      std::string customPath = ""
    ) = 0;

    /**
     * Read the lines cache file and fill the lines vector.
     * 
     * @return 0 in case of success, values below 0 when errors occurred:
     * -EBADMSG if deserialization did not work
     * -ENOENT if the file does not exist
     * -EINVAL For any other data related error
     * -(error codes from the open system call)
     */
    virtual int getLines(
      std::vector<std::unique_ptr<Line>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById,
      const std::map<boost::uuids::uuid, Agency>& agencies,
      const std::map<std::string, Mode>& modes,
      std::string customPath = ""
    ) = 0;

    /**
     * Read the paths cache file and fill the paths vector.
     * 
     * @return 0 in case of success, values below 0 when errors occurred:
     * -EBADMSG if deserialization did not work
     * -ENOENT if the file does not exist
     * -EINVAL For any other data related error
     * -(error codes from the open system call)
     */
    virtual int getPaths(
      std::vector<std::unique_ptr<Path>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById,
      const std::map<boost::uuids::uuid, int>& lineIndexesByUuid,
      const std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
      std::string customPath = ""
    ) = 0;

    /**
     * Read the scenarios cache file and fill the scenarios vector.
     * 
     * @return 0 in case of success, values below 0 when errors occurred:
     * -EBADMSG if deserialization did not work
     * -ENOENT if the file does not exist
     * -EINVAL For any other data related error
     * -(error codes from the open system call)
     */
    virtual int getScenarios(
      std::vector<std::unique_ptr<Scenario>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById,
      const std::map<boost::uuids::uuid, Service>& services,
      const std::map<boost::uuids::uuid, int>& lineIndexesByUuid,
      const std::map<boost::uuids::uuid, Agency>& agencies,
      const std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
      const std::map<std::string, Mode>& modes,
      std::string customPath = ""
    ) = 0;

    /**
     * Read the lines with schedules cache file and fill the trips vector.
     * 
     * @return 0 in case of success, values below 0 when errors occurred:
     * -EBADMSG if deserialization did not work
     * -ENOENT if the file does not exist
     * -EINVAL For any other data related error
     * -(error codes from the open system call)
     */
    virtual int getSchedules(
      std::vector<std::unique_ptr<Trip>>& trips,
      const std::vector<std::unique_ptr<Line>>& lines,
      std::vector<std::unique_ptr<Path>>& paths,
      std::map<boost::uuids::uuid, int>& tripIndexesByUuid,
      const std::map<boost::uuids::uuid, Service>& services,
      const std::map<boost::uuids::uuid, int>& lineIndexesByUuid,
      const std::map<boost::uuids::uuid, int>& pathIndexesByUuid,
      const std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
      std::vector<std::vector<std::unique_ptr<int>>>&   tripConnectionDepartureTimes,
      std::vector<std::vector<std::unique_ptr<float>>>& tripConnectionDemands,
      std::vector<std::shared_ptr<std::tuple<int,int,int,int,int,short,short,int,int,int,short,short>>>& connections, 
      std::string customPath = "") = 0;

  };    
}
#endif
