#ifndef TR_PARAMETERS
#define TR_PARAMETERS

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <boost/uuid/uuid.hpp>
#include "data_source.hpp"

namespace TrRouting
{

  class Point;
  class OdTrip;
  class Scenario;
  class Node;
  class Mode;
  class DataSource;
  class Agency;

  class ParameterException : public std::exception
  {
    public:
      enum class Type
      {
        // No scenario was specified
        MISSING_SCENARIO = 0,
        // Origin was not specified, but is mandatory
        MISSING_ORIGIN,
        // Destination was not specified, but is mandatory
        MISSING_DESTINATION,
        // time_of_trip should be specified
        MISSING_TIME_OF_TRIP,
        // The selected scenario does not contain any trips
        EMPTY_SCENARIO,
        // Origin data received is invalid. Expected comma-separate lat/lon
        INVALID_ORIGIN,
        // Destination data received is invalid. Expected comma-separate lat/lon
        INVALID_DESTINATION,
        // Some parameter value is invalid. Expected an integer
        INVALID_NUMERICAL_DATA
      };
      ParameterException(Type type_) : std::exception(), type(type_) {};
      Type getType() const { return type; };

    private:
      Type type;
  };

  class RouteParameters {
    private:
      boost::uuids::uuid scenarioUuid;
      // FIXME The scenario pointer is required for now, even if we extract its
      // data, because alternatives calculations will need to create new objects
      // from this RouteParameters object, but changing one field (the max travel
      // time). When calculation specific parameters are implemented, this field
      // will not be required in this class anymore.
      Scenario& scenario;
      std::unique_ptr<Point> origin;
      std::unique_ptr<Point> destination;

      int timeOfTrip;
      int minWaitingTimeSeconds;
      int maxTotalTravelTimeSeconds;
      int maxAccessWalkingTravelTimeSeconds;
      int maxEgressWalkingTravelTimeSeconds;
      int maxTransferWalkingTravelTimeSeconds;
      int maxFirstWaitingTimeSeconds;

      std::vector<int> onlyServicesIdx;
      std::vector<int> exceptServicesIdx;
      std::vector<int> onlyLinesIdx;
      // FIXME: Temporarily moved to public until calculation specific parameters exist. This is used directly by alternatives routing.
      // see https://github.com/chairemobilite/trRouting/issues/95
      // std::vector<int> exceptLinesIdx;
      std::vector<std::reference_wrapper<const Mode>> onlyModes;
      std::vector<std::reference_wrapper<const Mode>> exceptModes;
      std::vector<std::reference_wrapper<const Agency>> onlyAgencies;
      std::vector<std::reference_wrapper<const Agency>> exceptAgencies;
      std::vector<int> onlyNodesIdx;
      std::vector<int> exceptNodesIdx;
      bool withAlternatives; // calculate alternatives or not
      bool forwardCalculation; // forward calculation: default true. if false: reverse calculation, will ride connections backward (useful when setting the arrival time)

    public:
      RouteParameters(std::unique_ptr<Point> orig,
        std::unique_ptr<Point> dest,
        Scenario& scenario,
        int timeOfTrip,
        int minWaitingTime,
        int maxTotalTime,
        int maxAccessTime,
        int maxEgressTime,
        int maxTransferTime,
        int maxFirstWaitingTime,
        bool alt,
        bool forward
      );
      RouteParameters(const RouteParameters& routeParams);
      Point* getOrigin() { return origin.get(); }
      Point* getDestination() { return destination.get(); }
      // FIXME Temporary method, will be removed once calculation specific parameters are implemented. Try not to use.
      Scenario& getScenario() { return scenario; }
      int getTimeOfTrip() const { return timeOfTrip; }
      int getMinWaitingTimeSeconds() const { return minWaitingTimeSeconds; }
      int getMaxTotalTravelTimeSeconds() const { return maxTotalTravelTimeSeconds; }
      int getMaxAccessWalkingTravelTimeSeconds() const { return maxAccessWalkingTravelTimeSeconds; }
      int getMaxEgressWalkingTravelTimeSeconds() const { return maxEgressWalkingTravelTimeSeconds; }
      int getMaxTransferWalkingTravelTimeSeconds() const { return maxTransferWalkingTravelTimeSeconds; }
      int getMaxFirstWaitingTimeSeconds() const { return maxFirstWaitingTimeSeconds; }
      bool isWithAlternatives() { return withAlternatives; }
      bool isForwardCalculation() { return forwardCalculation; }

      /**
       * Factory function to create a routeParameters object from  a map of
       * parameters coming from the origin/destination route. It returns a new
       * immutable routeParameters object with complete parameter initialization.
       *
       * If there are missing parameters, this function will throw a
       * ParameterException error
       **/
      static RouteParameters createRouteODParameter(std::vector<std::pair<std::string, std::string>> &parameters,
        std::map<boost::uuids::uuid, int> &scenarioIndexesByUuid,
        std::vector<std::unique_ptr<Scenario>> &scenarios
      );
      std::vector<int>* getOnlyServicesIdx() { return &onlyServicesIdx; }
      std::vector<int>* getExceptServicesIdx() { return &exceptServicesIdx; }
      std::vector<int>* getOnlyLinesIdx() { return &onlyLinesIdx; }
      std::vector<int>* getExceptLinesIdx() { return &exceptLinesIdx; }
      const std::vector<std::reference_wrapper<const Mode>>& getOnlyModes() { return onlyModes; }
      const std::vector<std::reference_wrapper<const Mode>>& getExceptModes() { return exceptModes; }
      const std::vector<std::reference_wrapper<const Agency>>& getOnlyAgencies() { return onlyAgencies; }
      const std::vector<std::reference_wrapper<const Agency>>& getExceptAgencies() { return exceptAgencies; }
      std::vector<int>* getOnlyNodesIdx() { return &onlyNodesIdx; }
      std::vector<int>* getExceptNodesIdx() { return &exceptNodesIdx; }

      // FIXME: Temporarily moved to public until calculation specific parameters exist. This is used directly by alternatives routing.
      // see https://github.com/chairemobilite/trRouting/issues/95
      std::vector<int> exceptLinesIdx;
  };

  class Parameters {

    public:

      int batchNumber;
      int batchesCount;
      int odTripsSampleSize;
      unsigned int seed;

      int routingDateYear;   // not implemented, use onlyServicesIdx or exceptServicesIdx for now
      int routingDateMonth;  // not implemented, use onlyServicesIdx or exceptServicesIdx for now
      int routingDateDay;    // not implemented, use onlyServicesIdx or exceptServicesIdx for now
      //TODO Make it a reference or not??
      std::optional<DataSource> onlyDataSource;
      std::vector<int> accessNodesIdx;
      std::vector<int> accessNodeTravelTimesSeconds;
      std::vector<int> accessNodeDistancesMeters;
      std::vector<int> egressNodesIdx;
      std::vector<int> egressNodeTravelTimesSeconds;
      std::vector<int> egressNodeDistancesMeters;

      std::vector<std::pair<int,int>> odTripsPeriods; // pair: start_at_seconds, end_at_seconds
      std::vector<std::string>        odTripsGenders;
      std::vector<std::string>        odTripsAgeGroups;
      std::vector<std::string>        odTripsOccupations;
      std::vector<std::string>        odTripsActivities;
      std::vector<std::string>        odTripsModes;

      int maxNumberOfTransfers;
      int transferPenaltySeconds;
      int maxAccessWalkingDistanceMeters;
      int maxTotalWalkingTravelTimeSeconds;
      float odTripsSampleRatio;
      float maxOnlyWalkingAccessTravelTimeRatio;
      float walkingSpeedFactor;
      float walkingSpeedMetersPerSecond;
      float drivingSpeedMetersPerSecond;
      float cyclingSpeedMetersPerSecond;

      int originNodeIdx;
      int destinationNodeIdx;
      bool calculateAllOdTrips;
      std::optional<boost::uuids::uuid> odTripUuid;
      std::optional<boost::uuids::uuid> startingNodeUuid;
      std::optional<boost::uuids::uuid> endingNodeUuid;

      std::string accessMode;
      std::string egressMode;
      bool tryNextModeIfRoutingFails;
      std::string noResultSecondMode;
      int noResultNextAccessTimeSecondsIncrement;
      int maxNoResultNextAccessTimeSeconds;
      int maxAlternatives; // number of alternatives to calculate before returning results (when alternatives parameter is set to true)
      float alternativesMaxTravelTimeRatio; // travel time of fastest route is multiplied by this ratio to find plausible alternative with a max travel time.
      float minAlternativeMaxTravelTimeSeconds; // if multiplying max travel time ratio with max travel time is too small, keep max travel time to this minimum.
      int   alternativesMaxAddedTravelTimeSeconds; // how many seconds to add to fastest travel time to limit alternatives travel time.
      int   maxValidAlternatives; // max number of valid alternatives to return

      bool returnAllNodesResult;         // keep results for all nodes (used in creating accessibility map)
      bool forwardCalculation;           // forward calculation: default. if false: reverse calculation, will ride connections backward (useful when setting the arrival time)
      bool detailedResults;              // return detailed results when using results for all nodes
      bool transferBetweenSameLine;      // allow transfers between the same line
      bool calculateByNumberOfTransfers; // calculate first the fastest route, then calculate with decreasing number of transfers until no route is found, return results for each number of transfers.
      bool calculateProfiles;            // calculate profiles for lines, paths and trips (od trips only)

      void setDefaultValues();
      /**
       * @deprecated ServerParameters should not be updated this way, directly create a RouteParameters, but legacy endpoints still use this method.
       * */
      RouteParameters update(std::vector<std::string> &parameters,
        std::map<boost::uuids::uuid, int> &scenarioIndexesByUuid,
        std::vector<std::unique_ptr<Scenario>> &scenarios,
        std::map<boost::uuids::uuid, int> &odTripIndexesByUuid,
        std::vector<std::unique_ptr<OdTrip>> &odTrips,
        std::map<boost::uuids::uuid, int> &nodeIndexesByUuid,
        std::vector<std::unique_ptr<Node>> &nodes,
                             const std::map<boost::uuids::uuid, DataSource> &dataSources);

  };

}


#endif // TR_PARAMETERS
