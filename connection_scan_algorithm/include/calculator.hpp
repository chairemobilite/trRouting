#ifndef TR_CALCULATOR
#define TR_CALCULATOR

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <deque>
#include <tuple>

#include <boost/uuid/uuid.hpp>

#include "calculation_time.hpp"

namespace TrRouting
{
  class Parameters;
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
  class Node;
  class Stop;
  class Line;
  class Path;
  class Scenario;
  class Trip;
  class RoutingResult;
  class AlternativesResult;

  // tuple representing a connection: departureNodeIndex, arrivalNodeIndex, departureTimeSeconds, arrivalTimeSeconds, tripIndex, canBoard, canUnboard, sequence in trip, lineIndex, blockIndex, canTransferSameLine, minWaitingTimeSeconds (-1 to inherit from parameters)
  using ConnectionTuple = std::tuple<int,int,int,int,int,short,short,int,int,int,short,short>;

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

    Calculator(Parameters& theParams);

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
    std::tuple<int,int,int,int> forwardCalculation(RouteParameters &parameters); // best arrival time,   best egress node index, best egress travel time: MAX_INT,-1,-1 if non routable, too long or all nodes result
    std::tuple<int,int,int,int> reverseCalculation(RouteParameters &parameters); // best departure time, best access node index, best access travel time: -1,-1,-1 if non routable, too long or all nodes result
    // TODO See calculate
    std::unique_ptr<RoutingResult> forwardJourneyStep(RouteParameters &parameters, int bestArrivalTime, int bestEgressNodeIndex, int bestEgressTravelTime, int bestEgressDistance);
    // TODO See calculate
    std::unique_ptr<RoutingResult> reverseJourneyStep(RouteParameters &parameters, int bestDepartureTime, int bestAccessNodeIndex, int bestAccessTravelTime, int bestAccessDistance);
    AlternativesResult alternativesRouting(RouteParameters &parameters);
    std::string             odTripsRouting(RouteParameters &parameters);

    std::vector<int>        optimizeJourney(std::deque<std::tuple<int,int,int,int,int,short,int>> &journey);

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
    int                    updateHouseholdsFromCache (std::string customPath = "");
    int                    updatePersonsFromCache    (std::string customPath = "");
    int                    updateOdTripsFromCache    (std::string customPath = "");
    int                    updatePlacesFromCache     (std::string customPath = "");

    int                    updateStationsFromCache   (std::string customPath = "");
    int                    updateAgenciesFromCache   (std::string customPath = "");
    int                    updateServicesFromCache   (std::string customPath = "");
    int                    updateNodesFromCache      (std::string customPath = "");
    int                    updateStopsFromCache      (std::string customPath = "");
    int                    updateLinesFromCache      (std::string customPath = "");
    int                    updatePathsFromCache      (std::string customPath = "");
    int                    updateScenariosFromCache  (std::string customPath = "");
    int                    updateSchedulesFromCache  (std::string customPath = "");

    int                    setConnections(std::vector<std::shared_ptr<ConnectionTuple>> connections);

    int               countStations();
    int               countAgencies();
    int               countServices();
    int               countNodes();
    int               countStops();
    int               countLines();
    int               countPaths();
    int               countScenarios();
    int               countTrips();
    long long         countConnections();
    int               countNetworks();

    // Public for testing, this function initializes the calculation vectors and should be called whenever nodes and schedules are updated
    // TODO As part of issue https://github.com/chairemobilite/trRouting/issues/95, this will be removed
    void initializeCalculationData();


    std::vector<Mode>                        modes;
    std::map<std::string, int>               modeIndexesByShortname;


    std::vector<std::unique_ptr<DataSource>> dataSources;
    std::map<boost::uuids::uuid, int>        dataSourceIndexesByUuid;

    std::vector<std::unique_ptr<Household>>  households;
    std::map<boost::uuids::uuid, int>        householdIndexesByUuid;

    std::vector<std::unique_ptr<Person>>     persons;
    std::map<boost::uuids::uuid, int>        personIndexesByUuid;

    std::vector<std::unique_ptr<OdTrip>>     odTrips;
    std::map<boost::uuids::uuid, int>        odTripIndexesByUuid;

    std::vector<std::unique_ptr<Place>>      places;
    std::map<boost::uuids::uuid, int>        placeIndexesByUuid;


    std::vector<std::unique_ptr<Agency>>     agencies;
    std::map<boost::uuids::uuid, int>        agencyIndexesByUuid;

    std::vector<std::unique_ptr<Service>>    services;
    std::map<boost::uuids::uuid, int>        serviceIndexesByUuid;

    std::vector<std::unique_ptr<Station>>    stations;
    std::map<boost::uuids::uuid, int>        stationIndexesByUuid;

    std::vector<std::unique_ptr<Node>>       nodes;
    std::map<boost::uuids::uuid, int>        nodeIndexesByUuid;

    std::vector<std::unique_ptr<Stop>>       stops;
    std::map<boost::uuids::uuid, int>        stopIndexesByUuid;

    std::vector<std::unique_ptr<Line>>       lines;
    std::map<boost::uuids::uuid, int>        lineIndexesByUuid;

    std::vector<std::unique_ptr<Path>>       paths;
    std::map<boost::uuids::uuid, int>        pathIndexesByUuid;

    std::vector<std::unique_ptr<Scenario>>   scenarios;
    std::map<boost::uuids::uuid, int>        scenarioIndexesByUuid;

    std::vector<std::unique_ptr<Trip>>       trips;
    std::map<boost::uuids::uuid, int>        tripIndexesByUuid;

    /*std::vector<std::unique_ptr<Block>>      blocks;
    std::map<boost::uuids::uuid, int>        blockIndexesByUuid;*/

    std::vector<std::vector<std::unique_ptr<int>>>   tripConnectionDepartureTimes; // tripIndex: [connectionIndex (sequence in trip): departureTimeSeconds]
    std::vector<std::vector<std::unique_ptr<float>>> tripConnectionDemands;        // tripIndex: [connectionIndex (sequence in trip): sum of od trips weights using this connection (demand)]
    //std::vector<std::unique_ptr<std::vector<std::unique_ptr<int>>>>   tripIndexesByPathIndex;

    Parameters& params;
    CalculationTime algorithmCalculationTime;

    OdTrip * odTrip;

  private:

    enum connectionIndexes : short { NODE_DEP = 0, NODE_ARR = 1, TIME_DEP = 2, TIME_ARR = 3, TRIP = 4, CAN_BOARD = 5, CAN_UNBOARD = 6, SEQUENCE = 7, LINE = 8, BLOCK = 9, CAN_TRANSFER_SAME_LINE = 10, MIN_WAITING_TIME_SECONDS = 11 };
    enum journeyStepIndexes: short { FINAL_ENTER_CONNECTION = 0, FINAL_EXIT_CONNECTION = 1, FINAL_TRANSFERRING_NODE = 2, FINAL_TRIP = 3, TRANSFER_TRAVEL_TIME = 4, IS_SAME_NODE_TRANSFER = 5, TRANSFER_DISTANCE = 6 };

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
    std::vector<int> nodesTentativeTime; // arrival time at node (MAX_INT if not yet reached or unreachable)
    std::vector<int> nodesReverseTentativeTime; // departure time at node (MAX_INT if not yet reached or unreachable)
    std::vector<int> nodesAccessTravelTime; // travel time from origin to accessible nodes (-1 if unreachable by access mode)
    std::vector<int> nodesEgressTravelTime; // travel time to reach destination (-1 if unreachable by egress mode)
    std::vector<int> nodesAccessDistance; // distance from origin to accessible nodes (-1 if unreachable by access mode)
    std::vector<int> nodesEgressDistance; // distance to reach destination (-1 if unreachable by egress mode)
    std::vector<int> tripsEnterConnection; // index of the entering connection for each trip index
    std::vector<int> tripsEnterConnectionTransferTravelTime; // index of the entering connection for each trip index
    std::vector<int> tripsExitConnection; // index of the exiting connection for each trip index
    std::vector<int> tripsExitConnectionTransferTravelTime; // index of the exiting connection for each trip index
    std::vector<int> tripsEnabled; // allow/disallow use of this trip during calculation
    std::vector<int> tripsUsable; // after forward calculation, keep a list of usable trips in time range for reverse calculation
    std::vector<std::tuple<int,int,int>> accessFootpaths; // pair: accessNodeIndex, walkingTravelTimeSeconds, walkingDistanceMeters
    std::vector<std::tuple<int,int,int>> egressFootpaths; // pair: egressNodeIndex, walkingTravelTimeSeconds, walkingDistanceMeters
    std::vector<std::shared_ptr<ConnectionTuple>> forwardConnections; // Forward connections, sorted by departure time ascending
    std::vector<std::shared_ptr<ConnectionTuple>> reverseConnections; // Reverse connections, sorted by arrival time descending
    std::vector<std::tuple<int,int,int,int,int,short,int>> forwardJourneysSteps; // index = node index, tuple: final enter connection, final exit connection, final transferring node index, final trip index, transfer travel time, is same node transfer (first, second, third and fourth values = -1 for access and egress journeys)
    std::vector<std::tuple<int,int,int,int,int,short,int>> forwardEgressJourneysSteps; // index = node index, tuple: final enter connection, final exit connection, final transferring node index, final trip index, transfer travel time, is same node transfer (first, second, third and fourth values = -1 for access and egress journeys)
    std::vector<std::tuple<int,int,int,int,int,short,int>> reverseJourneysSteps; // index = node index, tuple: final enter connection, final exit connection, final transferring node index, final trip index, transfer travel time, is same node transfer (first, second, third and fourth values = -1 for access and egress journeys)
    std::vector<std::tuple<int,int,int,int,int,short,int>> reverseAccessJourneysSteps; // index = node index, tuple: final enter connection, final exit connection, final transferring node index, final trip index, transfer travel time, is same node transfer (first, second, third and fourth values = -1 for access and egress journeys)

  };

}

#endif // TR_CALCULATOR
