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
  class ConnectionSet;
  class Point;
  class GeoFilter;

  class Calculator {

  public:

    Calculator(const TransitData &_transitData, GeoFilter &_geofilter);

    void reset(CommonParameters &parameters, std::optional<std::reference_wrapper<const Point>> origin, std::optional<std::reference_wrapper<const Point>> destination, bool resetAccessPaths = true, bool resetFilters = true);
    // TODO This function supports both allNodes and simple calculation, which
    // are 2 very different return values. They should be split so it can return
    // a concrete result object instead of pointer (that alternatives could use directly), but still
    // use common calculation functions
    // TODO Once the split is done, we can get rid of the unique_ptr return and have the right concret type returned directly
    std::unique_ptr<SingleCalculationResult> calculateSingle(RouteParameters &parameters, bool resetAccessPaths = true, bool resetFilters = true);
    std::unique_ptr<AllNodesResult> calculateAllNodes(AccessibilityParameters &parameters);

    // Forward and and reverse calculation, in addition to their return values will fill up their JourneysSteps map
    std::optional<std::tuple<int, std::reference_wrapper<const Node>>> forwardCalculation(RouteParameters &parameters, std::unordered_map<Node::uid_t, JourneyStep> & forwardEgressJourneysSteps); // best arrival time,   best egress node
    void forwardCalculationAllNodes(AccessibilityParameters &parameters, std::unordered_map<Node::uid_t, JourneyStep> & forwardEgressJourneysSteps);

    std::optional<std::tuple<int, std::reference_wrapper<const Node>>> reverseCalculation(RouteParameters &parameters, std::unordered_map<Node::uid_t, JourneyStep> & reverseAccessJourneysSteps); // best departure time, best access node
    void reverseCalculationAllNodes(AccessibilityParameters &parameters, std::unordered_map<Node::uid_t, JourneyStep> & reverseAccessJourneysSteps);

    // TODO See calculate
    std::unique_ptr<SingleCalculationResult> forwardJourneyStep(RouteParameters &parameters, std::optional<std::reference_wrapper<const Node>> bestEgressNode, const std::unordered_map<Node::uid_t, JourneyStep> & forwardEgressJourneysSteps);
    std::unique_ptr<AllNodesResult> forwardJourneyStepAllNodes(AccessibilityParameters &parameters, const std::unordered_map<Node::uid_t, JourneyStep> & forwardEgressJourneysSteps);

    // TODO See calculate
    std::unique_ptr<SingleCalculationResult> reverseJourneyStep(RouteParameters &parameters, int bestDepartureTime, std::optional<std::reference_wrapper<const Node>> bestAccessNode, const std::unordered_map<Node::uid_t, JourneyStep> & reverseAccessJourneysSteps);
    std::unique_ptr<AllNodesResult> reverseJourneyStepAllNodes(AccessibilityParameters &parameters, const std::unordered_map<Node::uid_t, JourneyStep> & reverseAccessJourneysSteps);

    AlternativesResult alternativesRouting(RouteParameters &parameters);
    std::string             odTripsRouting(RouteParameters &parameters);

    std::vector<int>        optimizeJourney(std::deque<JourneyStep> &journey);

  private:
    void initializeCalculationData();
    bool resetAccessFootpaths(const CommonParameters &parameters, const Point& origin);
    bool resetEgressFootpaths(const CommonParameters &parameters, const Point& destination);
    void resetFilters(const CommonParameters &parameters);
    // Convert the optimization case ID returned by optimizeJourney to a string
    std::string optimizeCasesToString(const std::vector<int> optimizeCases);
    std::unique_ptr<SingleCalculationResult> calculateSingleReverse(RouteParameters &parameters);

    CalculationTime algorithmCalculationTime;
    //TODO set it mutable so it can be changed/reset?
    const TransitData &transitData;
    //TODO Should it be const?
    GeoFilter &geoFilter;

    int              departureTimeSeconds;
    int              arrivalTimeSeconds;
    int              minAccessTravelTime;
    int              maxEgressTravelTime;
    int              maxAccessTravelTime;
    int              minEgressTravelTime;
    long long        calculationTime;

    // TODO Added Glob suffix to easily track which one was local and which was global
    std::optional<std::reference_wrapper<const OdTrip>> odTripGlob; //Used to tell the reset function that we are doing an OdTrip calculations

    std::vector<int> nodesTentativeTime; // arrival time at node, using the Node::id as index
    std::vector<int> nodesReverseTentativeTime; // departure time at node
    std::unordered_map<Node::uid_t, NodeTimeDistance> nodesAccess; // travel time/distance from origin to accessible nodes
    std::unordered_map<Node::uid_t, NodeTimeDistance> nodesEgress; // travel time/distance to reach destination;

    // Field used at the time of the query, mostly for alternatives, which deactivate some of the scenario trips. Typically, there are less disabled trips and enabled ones, so we keep only those
    // Since the number of disabled trip is more commonly low, the map query is fast
    std::unordered_map<Trip::uid_t, bool> tripsDisabled;
    bool isTripDisabled(Trip::uid_t uid) const { return tripsDisabled.find(uid) != tripsDisabled.end(); }
    std::vector<TripQueryData> tripsQueryOverlay; // Store additionnal trip info during a query processing, indexed by Trip::uid
    // A subset of the connections that are used for the current query.
    std::shared_ptr<ConnectionSet> connectionSet;

    std::vector<NodeTimeDistance> accessFootpaths; // pair: accessNodeIndex, walkingTravelTimeSeconds, walkingDistanceMeters
    std::vector<NodeTimeDistance> egressFootpaths; // pair: egressNodeIndex, walkingTravelTimeSeconds, walkingDistanceMeters
    std::vector<JourneyStep> forwardJourneysSteps; // indexed by Node::uid
    std::vector<JourneyStep> reverseJourneysSteps; // indexed by Node::uid

  };

}

#endif // TR_CALCULATOR
