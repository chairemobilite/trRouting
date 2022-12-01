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
#include "trip.hpp"
#include "journey_step.hpp"

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
  class RoutingResult;
  class SingleCalculationResult;
  class AllNodesResult;
  class AlternativesResult;
  class TransitData;

  class Calculator {

  public:

    std::map<std::string, int> benchmarking;

    Calculator(const TransitData &_transitData);

    /**
     * Prepare the data for the calculator and validate that there are at least
     * some data to do calculations on
     *
     * @return A DataStatus corresponding to the data readiness. Only if the
     * status is READY will queries be served on this data
     */
    void                    reset(RouteParameters &parameters, bool resetAccessPaths = true, bool resetFilters = true);
    // TODO This function supports both allNodes and simple calculation, which
    // are 2 very different return values. They should be split so it can return
    // a concrete result object instead of pointer (that alternatives could use directly), but still
    // use common calculation functions
    // TODO Once the split is done, we can get rid of the unique_ptr return and have the right concret type returned directly
    std::unique_ptr<RoutingResult> calculate(RouteParameters &parameters, bool resetAccessPaths = true, bool resetFilters = true);
    // Forward and and reverse calculation, in addition to their return values will fill up their JourneysSteps map
    std::optional<std::tuple<int, std::reference_wrapper<const Node>>> forwardCalculation(RouteParameters &parameters, std::unordered_map<Node::uid_t, JourneyStep> & forwardEgressJourneysSteps); // best arrival time,   best egress node
    std::optional<std::tuple<int, std::reference_wrapper<const Node>>> reverseCalculation(RouteParameters &parameters, std::unordered_map<Node::uid_t, JourneyStep> & reverseAccessJourneysSteps); // best departure time, best access node
    // TODO See calculate
    std::unique_ptr<SingleCalculationResult> forwardJourneyStep(RouteParameters &parameters, std::optional<std::reference_wrapper<const Node>> bestEgressNode, const std::unordered_map<Node::uid_t, JourneyStep> & forwardEgressJourneysSteps);
    std::unique_ptr<AllNodesResult> forwardJourneyStepAllNodes(RouteParameters &parameters, const std::unordered_map<Node::uid_t, JourneyStep> & forwardEgressJourneysSteps);

    // TODO See calculate
    std::unique_ptr<RoutingResult> reverseJourneyStep(RouteParameters &parameters, int bestDepartureTime, std::optional<std::reference_wrapper<const Node>> bestAccessNode, const std::unordered_map<Node::uid_t, JourneyStep> & reverseAccessJourneysSteps);
    AlternativesResult alternativesRouting(RouteParameters &parameters);
    std::string             odTripsRouting(RouteParameters &parameters);

    std::vector<int>        optimizeJourney(std::deque<JourneyStep> &journey);

    // Public for testing, this function initializes the calculation vectors and should be called whenever nodes and schedules are updated
    // TODO As part of issue https://github.com/chairemobilite/trRouting/issues/95, this will be removed
    void initializeCalculationData();

    Parameters params;
    CalculationTime algorithmCalculationTime;

    // TODO Added Glob suffix to easily track which one was local and which was global
    std::optional<std::reference_wrapper<const OdTrip>> odTripGlob;

  private:
    bool resetAccessFootpaths(const RouteParameters &parameters);
    bool resetEgressFootpaths(const RouteParameters &parameters);
    void resetFilters(const RouteParameters &parameters);

    //TODO set it mutable so it can be changed/reset?
    const TransitData &transitData;

    int              departureTimeSeconds;
    int              initialDepartureTimeSeconds;
    int              arrivalTimeSeconds;
    int              minAccessTravelTime;
    int              maxEgressTravelTime;
    int              maxAccessTravelTime;
    int              minEgressTravelTime;
    long long        calculationTime;

    std::unordered_map<Node::uid_t, int> nodesTentativeTime; // arrival time at node
    std::unordered_map<Node::uid_t, int> nodesReverseTentativeTime; // departure time at node
    std::unordered_map<Node::uid_t, NodeTimeDistance> nodesAccess; // travel time/distance from origin to accessible nodes
    std::unordered_map<Node::uid_t, NodeTimeDistance> nodesEgress; // travel time/distance to reach destination;

    std::unordered_map<Trip::uid_t, bool> tripsEnabled;
    std::unordered_map<Trip::uid_t, TripQueryData> tripsQueryOverlay; // Store addition trip info during a query processing

    std::vector<NodeTimeDistance> accessFootpaths; // pair: accessNodeIndex, walkingTravelTimeSeconds, walkingDistanceMeters
    std::vector<NodeTimeDistance> egressFootpaths; // pair: egressNodeIndex, walkingTravelTimeSeconds, walkingDistanceMeters
    std::unordered_map<Node::uid_t, JourneyStep> forwardJourneysSteps; 
    std::unordered_map<Node::uid_t, JourneyStep> reverseJourneysSteps;

  };

}

#endif // TR_CALCULATOR
