#ifndef TR_TRANSIT_DATA
#define TR_TRANSIT_DATA

#include <map>
#include <string>
#include <vector>
#include <memory>

#include <boost/uuid/uuid.hpp>
#include "connection.hpp"
#include "connection_cache.hpp"


namespace TrRouting {

  class DataFetcher;
  class Mode;
  class DataSource;
  class Household;
  class Person;
  class OdTrip;
  class Place;
  class Agency;
  class Service;
  class Station;
  class Line;
  class Path;
  class Scenario;
  class Node;
  class Trip;
  
  enum class DataStatus {
    // Data is ready for query
    READY = 0,
    // Error reading the data source, no data available
    DATA_READ_ERROR,
    // There are no agencies in the data
    NO_AGENCIES,
    // There are no lines in the data
    NO_LINES,
    // There are no paths in the data
    NO_PATHS,
    // There are no services in the data
    NO_SERVICES,
    // There are no scenarios in the data
    NO_SCENARIOS,
    // There are no schedules in the data
    NO_SCHEDULES,
    // There are no nodes in the data
    NO_NODES
  };

  class TransitData {
  public:
    TransitData(DataFetcher& dataFetcher);
    DataStatus getDataStatus() const;
    
    // const data access functions
    const std::map<std::string, Mode> & getModes() const {return modes;}
    const std::map<boost::uuids::uuid, DataSource> & getDataSources() const {return dataSources;}
    const std::map<boost::uuids::uuid, Person> & getPersons() const {return persons;}
    const std::map<boost::uuids::uuid, OdTrip> & getOdTrips() const {return odTrips;}
    const std::map<boost::uuids::uuid, Agency> & getAgencies() const {return agencies;}
    const std::map<boost::uuids::uuid, Service> & getServices() const {return services;}
    const std::map<boost::uuids::uuid, Node> & getNodes() const {return nodes;}
    const std::map<boost::uuids::uuid, Line> & getLines() const {return lines;}
    const std::map<boost::uuids::uuid, Path> & getPaths() const {return paths;}
    const std::map<boost::uuids::uuid, Scenario> & getScenarios() const {return scenarios;}
    const std::map<boost::uuids::uuid, Trip> & getTrips() const {return trips;}
    unsigned int getConnectionCount() const {return connections.size();}

    std::shared_ptr<ConnectionSet> getConnectionsForScenario(const Scenario & scenario) const;

    /**
     * The update* methods get the data from the data fetcher
     *
     * @return 0 in case of success, values below 0 when errors occurred:
     * -EBADMSG if deserialization did not work
     * -ENOENT if the file does not exist
     * -EINVAL For any other data related error
     * -(error codes from the open system call)
     */
    int updateDataSources(std::string customPath = "");
    // TODO #167
    //int updateHouseholds(std::string customPath = "");
    int updatePersons(std::string customPath = "");
    int updateOdTrips(std::string customPath = "");
    // TODO #167
    //int updatePlaces(std::string customPath = "");

    int updateStations(std::string customPath = "");
    int updateAgencies(std::string customPath = "");
    int updateServices(std::string customPath = "");
    int updateNodes(std::string customPath = "");
    int updateLines(std::string customPath = "");
    int updatePaths(std::string customPath = "");
    int updateScenarios(std::string customPath = "");
    int updateSchedules(std::string customPath = "");
    
  protected:
    DataStatus loadAllData();
    int generateForwardAndReverseConnections();

    DataFetcher &dataFetcher;

    std::map<std::string, Mode>              modes;
    std::map<boost::uuids::uuid, DataSource> dataSources;
    std::map<boost::uuids::uuid, Person>     persons;
    std::map<boost::uuids::uuid, OdTrip>     odTrips;
    std::map<boost::uuids::uuid, Agency>     agencies;
    std::map<boost::uuids::uuid, Service>    services;
    std::map<boost::uuids::uuid, Node>       nodes;
    std::map<boost::uuids::uuid, Line>       lines;
    std::map<boost::uuids::uuid, Path>       paths;
    std::map<boost::uuids::uuid, Scenario>   scenarios;
    std::map<boost::uuids::uuid, Trip>       trips;

    std::vector<Connection> connections;
    std::vector<std::reference_wrapper<const Connection>> forwardConnections; // Forward connections, sorted by departure time ascending
    std::vector<std::reference_wrapper<const Connection>> reverseConnections; // Reverse connections, sorted by arrival time descending

    mutable ScenarioConnectionCache scenarioConnectionCache = ScenarioConnectionCache();
  };

}
#endif
