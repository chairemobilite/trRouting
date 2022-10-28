#ifndef TR_CALCULATOR
#define TR_CALCULATOR

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <deque>
#include <tuple>

#include <boost/uuid/uuid.hpp>

#include "calculation_time.hpp"
#include "parameters.hpp"
#include "connection.hpp"
#include "node.hpp"

namespace TrRouting
{

  class RouteParameters;
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
  class Trip;
  class RoutingResult;
  class AlternativesResult;
  class DataFetcher;

  using JourneyStep = std::tuple<int,int,int,int,short,int>; //final enter connection, final exit connection, final trip index, transfer travel time, is same node transfer (first, second, third and fourth values = -1 for access and egress journeys)

  class Calculator {

  public:

    enum class DataStatus
    {
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

    std::map<std::string, int> benchmarking;

    Calculator(DataFetcher& dataFetcher);

    /**
     * Prepare the data for the calculator and validate that there are at least
     * some data to do calculations on
     *
     * @return A DataStatus corresponding to the data readiness. Only if the
     * status is READY will queries be served on this data
     */
    DataStatus              prepare();
    void                    reset(RouteParameters &parameters, bool resetAccessPaths = true, bool resetFilters = true);
    // TODO This function supports both allNodes and simple calculation, which
    // are 2 very different return values. They should be split so it can return
    // a concrete result object instead of pointer (that alternatives could use directly), but still
    // use common calculation functions
    std::unique_ptr<RoutingResult> calculate(RouteParameters &parameters, bool resetAccessPaths = true, bool resetFilters = true);
    std::optional<std::tuple<int, std::reference_wrapper<const Node>>> forwardCalculation(RouteParameters &parameters); // best arrival time,   best egress node
    std::optional<std::tuple<int, std::reference_wrapper<const Node>>> reverseCalculation(RouteParameters &parameters); // best departure time, best access node
    // TODO See calculate
    std::unique_ptr<RoutingResult> forwardJourneyStep(RouteParameters &parameters, int bestArrivalTime, std::optional<std::reference_wrapper<const Node>> bestEgressNode);
    // TODO See calculate
    std::unique_ptr<RoutingResult> reverseJourneyStep(RouteParameters &parameters, int bestDepartureTime, std::optional<std::reference_wrapper<const Node>> bestAccessNode);
    AlternativesResult alternativesRouting(RouteParameters &parameters);
    std::string             odTripsRouting(RouteParameters &parameters);

    std::vector<int>        optimizeJourney(std::deque<JourneyStep> &journey);

    /**
     * The update*FromCache methods get the data from the cache
     *
     * @return 0 in case of success, values below 0 when errors occurred:
     * -EBADMSG if deserialization did not work
     * -ENOENT if the file does not exist
     * -EINVAL For any other data related error
     * -(error codes from the open system call)
     */
    int                    updateDataSourcesFromCache(std::string customPath = "");
    // TODO #167
    //int                    updateHouseholdsFromCache (std::string customPath = "");
    int                    updatePersonsFromCache    (std::string customPath = "");
    int                    updateOdTripsFromCache    (std::string customPath = "");
    // TODO #167
    //int                    updatePlacesFromCache     (std::string customPath = "");

    int                    updateStationsFromCache   (std::string customPath = "");
    int                    updateAgenciesFromCache   (std::string customPath = "");
    int                    updateServicesFromCache   (std::string customPath = "");
    int                    updateNodesFromCache      (std::string customPath = "");
    int                    updateLinesFromCache      (std::string customPath = "");
    int                    updatePathsFromCache      (std::string customPath = "");
    int                    updateScenariosFromCache  (std::string customPath = "");
    int                    updateSchedulesFromCache  (std::string customPath = "");

    int                    setConnections(std::vector<std::shared_ptr<ConnectionTuple>> connections);

    int               countAgencies();
    int               countServices();
    int               countNodes();
    int               countLines();
    int               countPaths();
    int               countScenarios();
    int               countTrips();
    long long         countConnections();

    // Public for testing, this function initializes the calculation vectors and should be called whenever nodes and schedules are updated
    // TODO As part of issue https://github.com/chairemobilite/trRouting/issues/95, this will be removed
    void initializeCalculationData();

    std::map<std::string, Mode>              modes;
    // Prefer using getModes to access the mode object, so that we have read-only version
    // TODO eventually, this will be moved to a const data container
    const std::map<std::string, Mode> & getModes() {return modes;}

    std::map<boost::uuids::uuid, DataSource> dataSources;
    const std::map<boost::uuids::uuid, DataSource> & getDataSources() {return dataSources;}

    std::map<boost::uuids::uuid, Person>     persons;
    const std::map<boost::uuids::uuid, Person> & getPersons() {return persons;}

    std::vector<std::unique_ptr<OdTrip>>     odTrips;
    std::map<boost::uuids::uuid, int>        odTripIndexesByUuid;

    std::map<boost::uuids::uuid, Agency>     agencies;
    const std::map<boost::uuids::uuid, Agency> & getAgencies() {return agencies;}

    std::map<boost::uuids::uuid, Service>    services;
    const std::map<boost::uuids::uuid, Service> & getServices() {return services;}

    std::map<boost::uuids::uuid, Node>       nodes;
    const std::map<boost::uuids::uuid, Node> & getNodes() {return nodes;}

    std::map<boost::uuids::uuid, Line>       lines;
    const std::map<boost::uuids::uuid, Line> & getLines() {return lines;}

    std::map<boost::uuids::uuid, Path>       paths;
    const std::map<boost::uuids::uuid, Path> & getPaths() {return paths;}

    std::map<boost::uuids::uuid, Scenario>   scenarios;
    const std::map<boost::uuids::uuid, Scenario> & getScenarios() {return scenarios;}


    std::vector<std::unique_ptr<Trip>>       trips;
    std::map<boost::uuids::uuid, int>        tripIndexesByUuid;

    std::vector<std::vector<std::unique_ptr<int>>>   tripConnectionDepartureTimes; // tripIndex: [connectionIndex (sequence in trip): departureTimeSeconds]
    std::vector<std::vector<std::unique_ptr<float>>> tripConnectionDemands;        // tripIndex: [connectionIndex (sequence in trip): sum of od trips weights using this connection (demand)]
    //std::vector<std::unique_ptr<std::vector<std::unique_ptr<int>>>>   tripIndexesByPathIndex;

    Parameters params;
    CalculationTime algorithmCalculationTime;
    DataFetcher &dataFetcher;

    OdTrip * odTrip;

  private:

    enum journeyStepIndexes: short { FINAL_ENTER_CONNECTION = 0, FINAL_EXIT_CONNECTION = 1, FINAL_TRIP = 2, TRANSFER_TRAVEL_TIME = 3, IS_SAME_NODE_TRANSFER = 4, TRANSFER_DISTANCE = 5 };

    int              departureTimeSeconds;
    int              initialDepartureTimeSeconds;
    int              arrivalTimeSeconds;
    int              maxTimeValue;
    int              minAccessTravelTime;
    int              maxEgressTravelTime;
    int              maxAccessTravelTime;
    int              minEgressTravelTime;
    int              maxAccessWalkingTravelTimeFromOriginToFirstNodeSeconds;
    int              maxAccessWalkingTravelTimeFromLastNodeToDestinationSeconds;
    long long        calculationTime;
    std::string      accessMode;
    std::string      egressMode;
    std::vector<int> forwardConnectionsIndexPerDepartureTimeHour;
    std::vector<int> reverseConnectionsIndexPerArrivalTimeHour;

    std::unordered_map<Node::uid_t, int> nodesTentativeTime; // arrival time at node
    std::unordered_map<Node::uid_t, int> nodesReverseTentativeTime; // departure time at node
    std::unordered_map<Node::uid_t, NodeTimeDistance> nodesAccess; // travel time/distance from origin to accessible nodes
    std::unordered_map<Node::uid_t, NodeTimeDistance> nodesEgress; // travel time/distance to reach destination;

    std::vector<int> tripsEnterConnection; // index of the entering connection for each trip index
    std::vector<int> tripsEnterConnectionTransferTravelTime; // index of the entering connection for each trip index
    std::vector<int> tripsExitConnection; // index of the exiting connection for each trip index
    std::vector<int> tripsExitConnectionTransferTravelTime; // index of the exiting connection for each trip index
    std::vector<int> tripsEnabled; // allow/disallow use of this trip during calculation
    std::vector<int> tripsUsable; // after forward calculation, keep a list of usable trips in time range for reverse calculation
    std::vector<NodeTimeDistance> accessFootpaths; // pair: accessNodeIndex, walkingTravelTimeSeconds, walkingDistanceMeters
    std::vector<NodeTimeDistance> egressFootpaths; // pair: egressNodeIndex, walkingTravelTimeSeconds, walkingDistanceMeters
    std::vector<std::shared_ptr<ConnectionTuple>> forwardConnections; // Forward connections, sorted by departure time ascending
    std::vector<std::shared_ptr<ConnectionTuple>> reverseConnections; // Reverse connections, sorted by arrival time descending
    std::unordered_map<Node::uid_t, JourneyStep> forwardJourneysSteps; 
    std::unordered_map<Node::uid_t, JourneyStep> forwardEgressJourneysSteps;
    std::unordered_map<Node::uid_t, JourneyStep> reverseJourneysSteps;
    std::unordered_map<Node::uid_t, JourneyStep> reverseAccessJourneysSteps;

  };

}

#endif // TR_CALCULATOR
