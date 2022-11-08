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
      std::map<boost::uuids::uuid, DataSource>& ,
      std::string = "") {return 0;}

    virtual int getPersons(
      std::map<boost::uuids::uuid, Person>& ,
      const std::map<boost::uuids::uuid, DataSource>& ,
      std::string = ""
                           ) {return 0;}

    virtual int getOdTrips(
      std::map<boost::uuids::uuid, OdTrip>& ,
      const std::map<boost::uuids::uuid, DataSource>& ,
      const std::map<boost::uuids::uuid, Person>& ,
      const std::map<boost::uuids::uuid, Node>& ,
      std::string = ""
                           ) {return 0;}

    virtual int getAgencies(
      std::map<boost::uuids::uuid, Agency>& ,
      std::string = ""
                            ) {return 0;}

    virtual int getServices(
      std::map<boost::uuids::uuid, Service>& ,
      std::string = ""
                            ) {return 0;}

    virtual int getNodes(
      std::map<boost::uuids::uuid, Node>& ,
      std::string = ""
                         ) {return 0;}

    virtual int getLines(
      std::map<boost::uuids::uuid, Line>& ,
      const std::map<boost::uuids::uuid, Agency>& ,
      const std::map<std::string, Mode>& ,
      std::string = ""
                         ) {return 0;}

    virtual int getPaths(
      std::map<boost::uuids::uuid, Path>& ,
      const std::map<boost::uuids::uuid, Line>& ,
      const std::map<boost::uuids::uuid, Node>& ,
      std::string = ""
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
      std::map<boost::uuids::uuid, Scenario>& ,
      const std::map<boost::uuids::uuid, Service>& ,
      const std::map<boost::uuids::uuid, Line>& ,
      const std::map<boost::uuids::uuid, Agency>& ,
      const std::map<boost::uuids::uuid, Node>& ,
      const std::map<std::string, Mode>& ,
      std::string = ""
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
      std::map<boost::uuids::uuid, Trip>& ,
      const std::map<boost::uuids::uuid, Line>& ,
      std::map<boost::uuids::uuid, Path>& ,
      const std::map<boost::uuids::uuid, Service>& ,
      std::vector<std::shared_ptr<ConnectionTuple>>& ,
      std::string = "") {return 0;}

  };    
}
#endif
