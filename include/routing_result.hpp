#ifndef TR_ROUTING_RESULT
#define TR_ROUTING_RESULT

#include <vector>
#include <string>
#include <boost/uuid/uuid.hpp>
#include <stdlib.h>
#include "routing_result_visitor.hpp"
#include "json.hpp"
#include "point.hpp" //Not using a forward declaration, as we use it more directly, see issue #129

namespace TrRouting
{
  class Line;
  class Path;
  class Trip;

  // TODO These enums are used temporarily, while we need the class hierarchy to be able to determine which type is returned when dynamic cast is necessary
  enum result_type { SINGLE_CALCULATION, ALTERNATIVES, ALL_NODES };
  enum result_step_type { WALKING, BOARDING, UNBOARDING };

  // Walking step types
  enum walking_step_type { ACCESS, EGRESS, TRANSFER };

  /**
   * @brief Base class for routing results. It accepts a visitor for the class hierarchy
   * 
   * TODO: Even later, when function calls are detricated and split, there will be no use for a class hierarchy. Concrete result types
   */
  class RoutingResult {
  public:
    enum result_type resType;
    RoutingResult(result_type _resType): resType(_resType) {}
    virtual ~RoutingResult() {}
    template<class T>
    T accept(ResultVisitor<T> &visitor) {
        do_accept(visitor);
        return visitor.getResult();
    }
    virtual void do_accept(ResultVisitorBase &visitor) const = 0;
  };

  /**
   * @brief Base class for routing result steps. It accepts a visitor for the steps class hierarchy.
   * These steps are only used for the SingleCalculationResult class
   */
  class RoutingStep {
  public:
    enum result_step_type action;
    RoutingStep(
      result_step_type _action
    ): action(_action) {}
    virtual ~RoutingStep() {}
    template<class T>
    T accept(StepVisitor<T> &visitor) {
        do_accept(visitor);
        return visitor.getResult();
    }
    virtual void do_accept(StepVisitorBase &visitor) const = 0;
  };

  /**
   * @brief Base class for transit steps (either boarding or unboarding). It includes all the data 
   * for the transit agency and line used in this step.
   * 
   */
  class TransitRoutingStep : public RoutingStep {
  public:
    boost::uuids::uuid agencyUuid;
    std::string agencyAcronym;
    std::string agencyName;
    boost::uuids::uuid lineUuid;
    std::string lineShortname;
    std::string lineLongname;
    boost::uuids::uuid pathUuid;
    std::string modeName;
    std::string mode;
    boost::uuids::uuid tripUuid;
    int legSequenceInTrip;
    int stopSequenceInTrip;
    boost::uuids::uuid nodeUuid;
    std::string nodeCode;
    std::string nodeName;
    Point& nodeCoordinates;
    TransitRoutingStep(
      result_step_type _action,
      boost::uuids::uuid _agencyUuid,
      std::string _agencyAcronym,
      std::string _agencyName,
      boost::uuids::uuid _lineUuid,
      std::string _lineShortname,
      std::string _lineLongname,
      boost::uuids::uuid _pathUuid,
      std::string _modeName,
      std::string _mode,
      boost::uuids::uuid _tripUuid,
      int _legSequenceInTrip,
      int _stopSequenceInTrip,
      boost::uuids::uuid _nodeUuid,
      std::string _nodeCode,
      std::string _nodeName,
      Point& _nodeCoordinates
    ): RoutingStep(_action),
    agencyUuid(_agencyUuid),
    agencyAcronym(_agencyAcronym),
    agencyName(_agencyName),
    lineUuid(_lineUuid),
    lineShortname(_lineShortname),
    lineLongname(_lineLongname),
    pathUuid(_pathUuid),
    modeName(_modeName),
    mode(_mode),
    tripUuid(_tripUuid),
    legSequenceInTrip(_legSequenceInTrip),
    stopSequenceInTrip(_stopSequenceInTrip),
    nodeUuid(_nodeUuid),
    nodeCode(_nodeCode),
    nodeName(_nodeName),
    nodeCoordinates(_nodeCoordinates)
    {}
  };

  /**
   * @brief Details a boarding step, with departure time and waiting time
   */
  class BoardingStep : public TransitRoutingStep {
  public:
    int departureTime;
    int waitingTime;
    BoardingStep(
      boost::uuids::uuid _agencyUuid,
      std::string _agencyAcronym,
      std::string _agencyName,
      boost::uuids::uuid _lineUuid,
      std::string _lineShortname,
      std::string _lineLongname,
      boost::uuids::uuid _pathUuid,
      std::string _modeName,
      std::string _mode,
      boost::uuids::uuid _tripUuid,
      int _legSequenceInTrip,
      int _stopSequenceInTrip,
      boost::uuids::uuid _nodeUuid,
      std::string _nodeCode,
      std::string _nodeName,
      Point& _nodeCoordinates,
      int _departureTime,
      int _waitingTime
    ): TransitRoutingStep(result_step_type::BOARDING, _agencyUuid, _agencyAcronym, _agencyName,
      _lineUuid, _lineShortname, _lineLongname, _pathUuid,
      _modeName, _mode, _tripUuid, _legSequenceInTrip,
      _stopSequenceInTrip, _nodeUuid, _nodeCode, _nodeName,
      _nodeCoordinates), departureTime(_departureTime), waitingTime(_waitingTime)
    {}
    void do_accept(StepVisitorBase &visitor) const override {
      return visitor.visitBoardingStep(*this);
    }
  };

  /**
   * @brief It details an unboarding step, with details on the arrival time and in vehicle times and distance
   */
  class UnboardingStep : public TransitRoutingStep {
  public:
    int arrivalTime;
    int inVehicleTime;
    int inVehicleDistanceMeters;
    UnboardingStep(
      boost::uuids::uuid _agencyUuid,
      std::string _agencyAcronym,
      std::string _agencyName,
      boost::uuids::uuid _lineUuid,
      std::string _lineShortname,
      std::string _lineLongname,
      boost::uuids::uuid _pathUuid,
      std::string _modeName,
      std::string _mode,
      boost::uuids::uuid _tripUuid,
      int _legSequenceInTrip,
      int _stopSequenceInTrip,
      boost::uuids::uuid _nodeUuid,
      std::string _nodeCode,
      std::string _nodeName,
      Point& _nodeCoordinates,
      int _arrivalTime,
      int _inVehicleTime,
      int _inVehicleDistanceMeters
    ): TransitRoutingStep(result_step_type::UNBOARDING, _agencyUuid, _agencyAcronym, _agencyName,
      _lineUuid, _lineShortname, _lineLongname, _pathUuid,
      _modeName, _mode, _tripUuid, _legSequenceInTrip,
      _stopSequenceInTrip, _nodeUuid, _nodeCode, _nodeName,
      _nodeCoordinates), arrivalTime(_arrivalTime), inVehicleTime(_inVehicleTime),
      inVehicleDistanceMeters(_inVehicleDistanceMeters)
    {}
    void do_accept(StepVisitorBase &visitor) const override {
      return visitor.visitUnboardingStep(*this);
    }
  };

  /**
   * @brief Class to detail a walking step, either to access, egress or transfer
   * 
   */
  class WalkingStep : public RoutingStep {
  public:
    enum walking_step_type walkingType;
    int travelTime;
    int distanceMeters;
    int departureTime;
    int arrivalTime;
    int readyToBoardAt;
    WalkingStep(walking_step_type walkType,
      int _travelTime,
      int _distanceMeters,
      int _departureTime,
      int _arrivalTime,
      int _readyToBoardAt = -1
    ): RoutingStep(result_step_type::WALKING),
      walkingType(walkType),
      travelTime(_travelTime),
      distanceMeters(_distanceMeters),
      departureTime(_departureTime),
      arrivalTime(_arrivalTime),
      readyToBoardAt(_readyToBoardAt)
    {}
    void do_accept(StepVisitorBase &visitor) const override {
      return visitor.visitWalkingStep(*this);
    }
  };

  /**
   * @brief Class detailing a single trip detail. It describes a single alternative trip.
   * 
   */
   // tuple: trip, start connection index, end connection index
  typedef std::tuple<std::reference_wrapper<const Trip>, int, int> Leg;

  class SingleCalculationResult : public RoutingResult {
  public:
    int departureTime;
    int arrivalTime;
    int totalTravelTime;
    int totalDistance;
    int totalInVehicleTime;
    int totalInVehicleDistance;
    int totalNonTransitTravelTime;
    int totalNonTransitDistance;
    int numberOfBoardings;
    int numberOfTransfers;
    int transferWalkingTime;
    int transferWalkingDistance;
    int accessTravelTime;
    int accessDistance;
    int egressTravelTime;
    int egressDistance;
    int transferWaitingTime;
    int firstWaitingTime;
    int totalWaitingTime;
    std::vector<std::unique_ptr<RoutingStep>> steps;
    // TODO Legs are used in the od_trip_routing function. They are kept here to avoid having to rewrite this code handling now, but eventually, it should the steps detail instead
    std::vector<Leg> legs;
    SingleCalculationResult():
      RoutingResult(result_type::SINGLE_CALCULATION)
    {}
    void do_accept(ResultVisitorBase &visitor) const override {
      return visitor.visitSingleCalculationResult(*this);
    }
  };

  /**
   * @brief Result class describing many alternative trips for the queried parameters
   */
  class AlternativesResult : public RoutingResult {
  public:
    // Results should be of SingleCalculationResult, but it's not possible to cast a unique_ptr
    int totalAlternativesCalculated;
    std::vector<std::unique_ptr<RoutingResult>> alternatives;
    AlternativesResult(): RoutingResult(result_type::ALTERNATIVES) {}
    void do_accept(ResultVisitorBase &visitor) const override {
      return visitor.visitAlternativesResult(*this);
    }
  };

  /**
   * @brief A class describing a single accessible node details, with the time to reach the node
   * and the number of transfers for the requested departure time
   */
  class AccessibleNodes {
  public:
    boost::uuids::uuid nodeUuid;
    int arrivalTime;
    int totalTravelTime;
    int numberOfTransfers;
    AccessibleNodes(boost::uuids::uuid _uuid,
      int _arrivalTime,
      int _totalTravelTime,
      int _numberOfTransfers
    ): nodeUuid(_uuid),
      arrivalTime(_arrivalTime),
      totalTravelTime(_totalTravelTime),
      numberOfTransfers(_numberOfTransfers)
    {}
  };

  /**
   * @brief Result class describing all accessible nodes within the queried parameters. It contains a list
   * of all accessible nodes and the times it takes
   */
  class AllNodesResult : public RoutingResult {
  public:
    std::vector<AccessibleNodes> nodes;
    int numberOfReachableNodes;
    float percentOfReachableNodes;
    AllNodesResult(): RoutingResult(result_type::ALL_NODES) {}
    void do_accept(ResultVisitorBase &visitor) const override {
      return visitor.visitAllNodesResult(*this);
    }
  };

  enum NoRoutingReason
  {
    // TODO As the calculator supports more reasons, add more elements to this enum
    // Generic reason, when not possible to specify more
    NO_ROUTING_FOUND,
    // No accessible node at origin
    NO_ACCESS_AT_ORIGIN,
    // No accessible node at destination
    NO_ACCESS_AT_DESTINATION,
    // There is no service from origin with the query parameters
    NO_SERVICE_FROM_ORIGIN,
    // There is no service to destination with the query parameters
    NO_SERVICE_TO_DESTINATION,
    // No accessible node at both origin and destination
    NO_ACCESS_AT_ORIGIN_AND_DESTINATION
  };

  /**
   * @brief Exception class thrown when no routing is found
   * 
   */
  class NoRoutingFoundException : public std::exception
  {
    public:
      NoRoutingFoundException(NoRoutingReason reason_) : std::exception(), reason(reason_) {};
      NoRoutingReason getReason() const { return reason; };

    private:
      NoRoutingReason reason;
  };

}

#endif // TR_ROUTING_RESULT
