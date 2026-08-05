#ifndef TR_PARAMETERS
#define TR_PARAMETERS

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <boost/uuid/uuid.hpp>
#include "toolbox.hpp" //MAX_INT

namespace TrRouting
{

  class Point;
  class Scenario;
  class Node;
  class Mode;
  class Agency;
  class Service;
  class Line;

  // Default values for parameters
  static const int DEFAULT_MIN_WAITING_TIME = 3 * 60;
  static const int DEFAULT_MAX_TOTAL_TIME = MAX_INT;
  static const int DEFAULT_MAX_ACCESS_TRAVEL_TIME = 20 * 60;
  static const int DEFAULT_MAX_EGRESS_TRAVEL_TIME = 20 * 60;
  static const int DEFAULT_MAX_TRANSFER_TRAVEL_TIME = 20 * 60;
  // 30 minutes: Buffer time added to time_of_trip when searching for first/last connections within the trip
  static const int DEFAULT_MAX_INNER_TIME_OF_TRIP_BUFFER = 30 * 60;

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
        // The specified scenario id is invalid
        INVALID_SCENARIO,
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
      int maxInnerTimeOfTripBufferSeconds;

      boost::uuids::uuid scenarioUuid;
      //TODO We don't do anything with those filter at the moment
      std::vector<std::reference_wrapper<const Service>> onlyServices;
      std::vector<std::reference_wrapper<const Line>> onlyLines;
      std::vector<std::reference_wrapper<const Agency>> onlyAgencies;
      std::vector<std::reference_wrapper<const Mode>> onlyModes;
      std::vector<std::reference_wrapper<const Node>> onlyNodes;
      //TODO exceptServices is never filled with anything
      std::vector<std::reference_wrapper<const Service>> exceptServices;
      std::vector<std::reference_wrapper<const Line>> exceptLines;
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
        int maxTimeOfTripBuffer,
        bool forward
      );
      virtual ~CommonParameters() {}
      // FIXME Temporary method, will be removed once calculation specific parameters are implemented. Try not to use.
      const Scenario& getScenario() const { return scenario; }
      int getTimeOfTrip() const { return timeOfTrip; }
      int getMinWaitingTimeSeconds() const { return minWaitingTimeSeconds; }
      int getMaxTotalTravelTimeSeconds() const { return maxTotalTravelTimeSeconds; }
      int getMaxAccessWalkingTravelTimeSeconds() const { return maxAccessWalkingTravelTimeSeconds; }
      int getMaxEgressWalkingTravelTimeSeconds() const { return maxEgressWalkingTravelTimeSeconds; }
      int getMaxTransferWalkingTravelTimeSeconds() const { return maxTransferWalkingTravelTimeSeconds; }
      /**
       * @brief Gets the value of the inner time of trip buffer, in seconds
       *
       * Get the time of trip buffer in seconds, used to bound initial
       * connection searches. This buffer will be added to the time of trip if
       * it is a departure time, or substracted if it is an arrival time.
       *
       * @return int 
       */
      int getMaxInnerTimeOfTripBufferSeconds() const { return maxInnerTimeOfTripBufferSeconds; }
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
      float getWalkingSpeedFactor() const { return 1.0; } // all walking segments are weighted with this value. > 1.0 means faster walking, < 1.0 means slower walking
      float getWalkingSpeedMetersPerSecond() const { return 5/3.6; } // 5 km/h;

      static CommonParameters createCommonParameter(const std::vector<std::pair<std::string, std::string>> &parameters,
                                                    const std::map<boost::uuids::uuid, Scenario> &scenarios
      );

      /**
       * @brief Helper function to get an integer value from a string, or throw
       * a ParameterException with an INVALID_NUMERICAL_VALUE type if the string
       * is not an integer. This prevents from having to handle an
       * invalid_argument exception at a higher level if the string value is not
       * of the proper type
       *
       * @param strValue Integer string value
       * @return int The converted integer
       */
      static int getIntegerValue(std::string strValue);
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

      // TODO Those values used to be in the legacy parameters object. They are not exposed
      // in the V2 api yet, but we used the default values in the alternative calculation.
      // For the moment, let's return default constant values
      int getMaxAlternatives() { return 200; } // number of alternatives to calculate before returning results (when alternatives parameter is set to true)
      float getAlternativesMaxTravelTimeRatio() { return 1.75; } // travel time of fastest route is multiplied by this ratio to find plausible alternative with a max travel time.
      float getMinAlternativeMaxTravelTimeSeconds() { return 30*60; } // if multiplying max travel time ratio with max travel time is too small, keep max travel time to this minimum.
      int getAlternativesMaxAddedTravelTimeSeconds() { return 60*60; } // how many seconds to add to fastest travel time to limit alternatives travel time.
      int getMaxValidAlternatives() { return 50; } // max number of valid alternatives to return

      /**
       * Factory function to create a routeParameters object from  a map of
       * parameters coming from the origin/destination route. It returns a new
       * immutable routeParameters object with complete parameter
       * initialization.
       *
       * If there are missing or invalid parameters, this function will throw a
       * ParameterException error
       **/
      static RouteParameters createRouteODParameter(const std::vector<std::pair<std::string, std::string>> &parameters,
                                                    const std::map<boost::uuids::uuid, Scenario> &scenarios
      );
  };

  class AccessibilityParameters : public CommonParameters {
    private:
      std::unique_ptr<Point> place;
      
    public:
      AccessibilityParameters(std::unique_ptr<Point> place,
        const Scenario& scenario,
        int timeOfTrip,
        int minWaitingTime,
        int maxTotalTime,
        int maxAccessTime,
        int maxEgressTime,
        int maxTransferTime,
        int maxFirstWaitingTime,
        bool forward
      );
      AccessibilityParameters(std::unique_ptr<Point> place,
        const CommonParameters &common_
      );
      virtual ~AccessibilityParameters() {}
      // TODO Should Point be const here?
      Point* getPlace() const { return place.get(); }

      /**
       * Factory function to create a AccessibilityParameters object from  a map of
       * parameters coming from the accessibility endpoint. It returns a new
       * immutable AccessibilityParameters object with complete parameter initialization.
       *
       * If there are missing or invalid parameters, this function will throw a
       * ParameterException error
       **/
      static AccessibilityParameters createAccessibilityParameter(const std::vector<std::pair<std::string, std::string>> &parameters,
                                                    const std::map<boost::uuids::uuid, Scenario> &scenarios
      );
  };
}


#endif // TR_PARAMETERS
