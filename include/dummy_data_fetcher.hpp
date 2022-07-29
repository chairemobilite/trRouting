#ifndef TR_DUMMY_DATA_FETCHER
#define TR_DUMMY_DATA_FETCHER

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <boost/uuid/uuid.hpp>
#include "data_fetcher.hpp"

namespace TrRouting
{
  class DataSource;
  class Household;
  class Person;
  class OdTrip;
  class Place;
  class Agency;
  class Service;
  class Node;
  class Stop;
  class Line;
  class Path;
  class Scenario;
  class Trip;
  class Mode;

  /** Dummy class, which returns empty data. Useful for simple testing
   */
  class DummyDataFetcher : public DataFetcher
  {
  public:
    DummyDataFetcher() {}
    virtual ~DummyDataFetcher() {}
    virtual const  std::map<std::string, Mode> getModes() {
      std::map<std::string, Mode> modes;
      return modes;
    };
    
    virtual int getDataSources(
      std::map<boost::uuids::uuid, DataSource>& ts,
      std::string customPath = "") {return 0;}

    virtual int getPersons(
      std::vector<std::unique_ptr<Person>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById, 
      const std::map<boost::uuids::uuid, DataSource>& dataSources,
      std::string customPath = ""
                           ) {return 0;}

    virtual int getOdTrips(
      std::vector<std::unique_ptr<OdTrip>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById, 
      const std::map<boost::uuids::uuid, DataSource>& dataSources,
      const std::map<boost::uuids::uuid, int>& personIndexesByUuid,     
      const std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
      std::string customPath = ""
                           ) {return 0;}

    virtual int getAgencies(
      std::map<boost::uuids::uuid, Agency>& ts,
      std::string customPath = ""
                            ) {return 0;}

    virtual int getServices(
      std::map<boost::uuids::uuid, Service>& ts,
      std::string customPath = ""
                            ) {return 0;}

    virtual int getNodes(
      std::vector<std::unique_ptr<Node>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById,
      std::string customPath = ""
                         ) {return 0;}

    virtual int getLines(
      std::vector<std::unique_ptr<Line>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById,
      const std::map<boost::uuids::uuid, Agency>& agencies,
      const std::map<std::string, Mode>& modes,
      std::string customPath = ""
                         ) {return 0;}

    virtual int getPaths(
      std::vector<std::unique_ptr<Path>>& ts,
      std::map<boost::uuids::uuid, int>& tIndexesById,
      const std::map<boost::uuids::uuid, int>& lineIndexesByUuid,
      const std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
      std::string customPath = ""
                         ) {return 0;}

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
                             ) {return 0;}

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
      std::string customPath = "") {return 0;}

  };    
}
#endif
