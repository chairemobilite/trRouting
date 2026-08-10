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
  class AlternativeFilter;

  class Calculator {

  public:

    Calculator(const TransitData &_transitData, GeoFilter &_geofilter);

    void reset(const CommonParameters &parameters, std::optional<std::reference_wrapper<const Point>> origin, std::optional<std::reference_wrapper<const Point>> destination, bool resetAccessPaths = true, AlternativeFilter *alternativeFilter = nullptr);
    // TODO This function supports both allNodes and simple calculation, which
    // are 2 very different return values. They should be split so it can return
    // a concrete result object instead of pointer (that alternatives could use directly), but still
    // use common calculation functions
    // TODO Once the split is done, we can get rid of the unique_ptr return and have the right concret type returned directly
    std::unique_ptr<SingleCalculationResult> calculateSingle(const RouteParameters &parameters, bool resetAccessPaths = true, AlternativeFilter *alternativeFilter = nullptr);
    std::unique_ptr<AllNodesResult> calculateAllNodes(const AccessibilityParameters &parameters);

    // Forward and and reverse calculation, in addition to their return values will fill up their JourneysSteps map
    std::optional<std::tuple<int, std::reference_wrapper<const Node>>> forwardCalculation(const RouteParameters &parameters, std::unordered_map<Node::uid_t, JourneyStep> & forwardEgressJourneysSteps); // best arrival time,   best egress node
    void forwardCalculationAllNodes(const AccessibilityParameters &parameters, std::unordered_map<Node::uid_t, JourneyStep> & forwardEgressJourneysSteps);

    std::optional<std::tuple<int, std::reference_wrapper<const Node>>> reverseCalculation(const RouteParameters &parameters, std::unordered_map<Node::uid_t, JourneyStep> & reverseAccessJourneysSteps); // best departure time, best access node
    void reverseCalculationAllNodes(const AccessibilityParameters &parameters, std::unordered_map<Node::uid_t, JourneyStep> & reverseAccessJourneysSteps);

    // TODO See calculate
    std::unique_ptr<SingleCalculationResult> forwardJourneyStep(const RouteParameters &parameters, std::optional<std::reference_wrapper<const Node>> bestEgressNode, const std::unordered_map<Node::uid_t, JourneyStep> & forwardEgressJourneysSteps);
    std::unique_ptr<AllNodesResult> forwardJourneyStepAllNodes(const AccessibilityParameters &parameters, const std::unordered_map<Node::uid_t, JourneyStep> & forwardEgressJourneysSteps);

    // TODO See calculate
    std::unique_ptr<SingleCalculationResult> reverseJourneyStep(const RouteParameters &parameters, int bestDepartureTime, std::optional<std::reference_wrapper<const Node>> bestAccessNode, const std::unordered_map<Node::uid_t, JourneyStep> & reverseAccessJourneysSteps);
    std::unique_ptr<AllNodesResult> reverseJourneyStepAllNodes(const AccessibilityParameters &parameters, const std::unordered_map<Node::uid_t, JourneyStep> & reverseAccessJourneysSteps);

    AlternativesResult alternativesRouting(const RouteParameters &parameters);

    std::vector<int>        optimizeJourney(std::deque<JourneyStep> &journey);

    // Start the timer used by the per-step debug timing logs. The constructor
    // does this for a fresh Calculator; a reused one needs it called once at
    // each request boundary. Deliberately not done in reset(), since
    // alternativesRouting() resets once per alternative while the timer is
    // meant to span the whole request.
    void startRequestTimer() { algorithmCalculationTime.start(); }

  private:
    void initializeCalculationData();
    bool resetAccessFootpaths(const CommonParameters &parameters, const Point& origin);
    bool resetEgressFootpaths(const CommonParameters &parameters, const Point& destination);
    // Convert the optimization case ID returned by optimizeJourney to a string
    std::string optimizeCasesToString(const std::vector<int> optimizeCases);
    std::unique_ptr<SingleCalculationResult> calculateSingleReverse(const RouteParameters &parameters);
    std::unique_ptr<SingleCalculationResult> calculateSingleForward(const RouteParameters &parameters);

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

    std::vector<int> nodesTentativeTime; // arrival time at node, using the Node::id as index
    std::vector<int> nodesReverseTentativeTime; // departure time at node
    std::unordered_map<Node::uid_t, NodeTimeDistance> nodesAccess; // travel time/distance from origin to accessible nodes
    std::unordered_map<Node::uid_t, NodeTimeDistance> nodesEgress; // travel time/distance to reach destination;

    // Store additionnal trip info during a query processing, indexed by Trip::uid. This includes the `disabled`
    // flag used by alternatives to deactivate some of the scenario trips, which lives here rather than in a
    // separate map so the connection scan reads it from the entry it already loaded.
    std::vector<TripQueryData> tripsQueryOverlay;
    // A subset of the connections that are used for the current query.
    std::shared_ptr<ConnectionSet> connectionSet;

    std::vector<NodeTimeDistance> accessFootpaths; // pair: accessNodeIndex, walkingTravelTimeSeconds, walkingDistanceMeters
    std::vector<NodeTimeDistance> egressFootpaths; // pair: egressNodeIndex, walkingTravelTimeSeconds, walkingDistanceMeters
    std::vector<JourneyStep> forwardJourneysSteps; // indexed by Node::uid
    std::vector<JourneyStep> reverseJourneysSteps; // indexed by Node::uid

  };

}

#endif // TR_CALCULATOR
