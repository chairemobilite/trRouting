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
  class Service;
  class Line;

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
        // Place was not specified, but is mandatory
        MISSING_PLACE,
        // The selected scenario does not contain any trips
        EMPTY_SCENARIO,
        // Origin data received is invalid. Expected comma-separated lon/lat
        INVALID_ORIGIN,
        // Destination data received is invalid. Expected comma-separated lon/lat
        INVALID_DESTINATION,
        // Place data received is invalid. Expected comma-separated lon/lat
        INVALID_PLACE,
        // Some parameter value is invalid. Expected an integer
        INVALID_NUMERICAL_DATA
      };
      ParameterException(Type type_) : std::exception(), type(type_) {};
      Type getType() const { return type; };

    private:
      Type type;
  };

  class CommonParameters {
    private:

      // FIXME The scenario pointer is required for now, even if we extract its
      // data, because alternatives calculations will need to create new objects
      // from this RouteParameters object, but changing one field (the max travel
      // time). When calculation specific parameters are implemented, this field
      // will not be required in this class anymore.
      const Scenario& scenario;

      int timeOfTrip;
      int minWaitingTimeSeconds;
      int maxTotalTravelTimeSeconds;
      int maxAccessWalkingTravelTimeSeconds;
      int maxEgressWalkingTravelTimeSeconds;
      int maxTransferWalkingTravelTimeSeconds;
      int maxFirstWaitingTimeSeconds;

      boost::uuids::uuid scenarioUuid;
      std::vector<std::reference_wrapper<const Service>> onlyServices;
      std::vector<std::reference_wrapper<const Line>> onlyLines;
      std::vector<std::reference_wrapper<const Agency>> onlyAgencies;
      std::vector<std::reference_wrapper<const Mode>> onlyModes;
      std::vector<std::reference_wrapper<const Node>> onlyNodes;
      //TODO exceptServices is never filled with anything
      std::vector<std::reference_wrapper<const Service>> exceptServices;
      // FIXME: Temporarily moved to public until calculation specific parameters exist. This is used directly by alternatives routing.
      // see https://github.com/chairemobilite/trRouting/issues/95
    public:
      std::vector<std::reference_wrapper<const Line>> exceptLines;
    private:
      std::vector<std::reference_wrapper<const Agency>> exceptAgencies;
      std::vector<std::reference_wrapper<const Mode>> exceptModes;
      std::vector<std::reference_wrapper<const Node>> exceptNodes;
      bool forwardCalculation; // forward calculation: default true. if false: reverse calculation, will ride connections backward (useful when setting the arrival time)

    protected:
      CommonParameters(const CommonParameters& baseParams);
      
    public:
      CommonParameters(const Scenario& scenario,
        int timeOfTrip,
        int minWaitingTime,
        int maxTotalTime,
        int maxAccessTime,
        int maxEgressTime,
        int maxTransferTime,
        int maxFirstWaitingTime,
        bool forward
      );
      virtual ~CommonParameters() {}
      // FIXME Temporary method, will be removed once calculation specific parameters are implemented. Try not to use.
      const Scenario& getScenario() { return scenario; }
      int getTimeOfTrip() const { return timeOfTrip; }
      int getMinWaitingTimeSeconds() const { return minWaitingTimeSeconds; }
      int getMaxTotalTravelTimeSeconds() const { return maxTotalTravelTimeSeconds; }
      int getMaxAccessWalkingTravelTimeSeconds() const { return maxAccessWalkingTravelTimeSeconds; }
      int getMaxEgressWalkingTravelTimeSeconds() const { return maxEgressWalkingTravelTimeSeconds; }
      int getMaxTransferWalkingTravelTimeSeconds() const { return maxTransferWalkingTravelTimeSeconds; }
      int getMaxFirstWaitingTimeSeconds() const { return maxFirstWaitingTimeSeconds; }
      bool isForwardCalculation() { return forwardCalculation; }
      const std::vector<std::reference_wrapper<const Service>>& getOnlyServices() const { return onlyServices; }
      const std::vector<std::reference_wrapper<const Service>>& getExceptServices() const { return exceptServices; }
      const std::vector<std::reference_wrapper<const Line>>& getOnlyLines() const { return onlyLines; }
      const std::vector<std::reference_wrapper<const Line>>& getExceptLines() const { return exceptLines; }
      const std::vector<std::reference_wrapper<const Mode>>& getOnlyModes() const { return onlyModes; }
      const std::vector<std::reference_wrapper<const Mode>>& getExceptModes() const { return exceptModes; }
      const std::vector<std::reference_wrapper<const Agency>>& getOnlyAgencies() const { return onlyAgencies; }
      const std::vector<std::reference_wrapper<const Agency>>& getExceptAgencies() const { return exceptAgencies; }
      const std::vector<std::reference_wrapper<const Node>>& getOnlyNodes() const { return onlyNodes; }
      const std::vector<std::reference_wrapper<const Node>>& getExceptNodes() const { return exceptNodes; }

      static CommonParameters createCommonParameter(std::vector<std::pair<std::string, std::string>> &parameters,
                                                    const std::map<boost::uuids::uuid, Scenario> &scenarios
      );
  };

  class RouteParameters : public CommonParameters {
    private:
      std::unique_ptr<Point> origin;
      std::unique_ptr<Point> destination;
      bool withAlternatives; // calculate alternatives or not
      
    public:
      RouteParameters(std::unique_ptr<Point> orig,
        std::unique_ptr<Point> dest,
        const Scenario& scenario,
        int timeOfTrip,
        int minWaitingTime,
        int maxTotalTime,
        int maxAccessTime,
        int maxEgressTime,
        int maxTransferTime,
        int maxFirstWaitingTime,
        bool alternatives,
        bool forward
      );
      RouteParameters(std::unique_ptr<Point> orig,
        std::unique_ptr<Point> dest,
        bool alternatives,
        const CommonParameters &common_
      );
      RouteParameters(const RouteParameters& routeParams);
      virtual ~RouteParameters() {}
      // TODO Should Point be const here?
      Point* getOrigin() const { return origin.get(); }
      Point* getDestination() const { return destination.get(); }
      bool isWithAlternatives() { return withAlternatives; }

      /**
       * Factory function to create a routeParameters object from  a map of
       * parameters coming from the origin/destination route. It returns a new
       * immutable routeParameters object with complete parameter initialization.
       *
       * If there are missing parameters, this function will throw a
       * ParameterException error
       **/
      static RouteParameters createRouteODParameter(std::vector<std::pair<std::string, std::string>> &parameters,
                                                    const std::map<boost::uuids::uuid, Scenario> &scenarios
      );
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
      //TODO We could convert those to NodeTimeDistance object
      std::vector<std::reference_wrapper<const Node>> accessNodesRef;
      std::vector<int> accessNodeTravelTimesSeconds;
      std::vector<int> accessNodeDistancesMeters;
      std::vector<std::reference_wrapper<const Node>> egressNodesRef;
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

      ~Parameters() {}
      void setDefaultValues();
      /**
       * @deprecated ServerParameters should not be updated this way, directly create a RouteParameters, but legacy endpoints still use this method.
       * */
      RouteParameters update(std::vector<std::string> &parameters,
                             const std::map<boost::uuids::uuid, Scenario> &scenario,
                             const std::map<boost::uuids::uuid, OdTrip> &odTrips,
                             const std::map<boost::uuids::uuid, Node> &nodes,
                             const std::map<boost::uuids::uuid, DataSource> &dataSources);

  };

}


#endif // TR_PARAMETERS
