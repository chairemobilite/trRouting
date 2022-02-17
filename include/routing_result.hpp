#ifndef TR_ROUTING_RESULT
#define TR_ROUTING_RESULT

#include <vector>
#include <string>
#include <boost/uuid/uuid.hpp>
#include <stdlib.h>
#include "point.hpp"
#include "parameters.hpp"

namespace TrRouting
{
  enum result_type { SINGLE_CALCULATION, NO_ROUTING_FOUND, ALTERNATIVES, ALL_NODES };
  enum result_step_type { WALKING, BOARDING, UNBOARDING };
  enum walking_step_type { ACCESS, EGRESS, TRANSFER };

  class SingleCalculationResult;
  class AlternativesResult;
  class AllNodesResult;
  class NoRoutingFoundResult;
  class StepBoardingRoutingResult;
  class StepUnboardingRoutingResult;
  class StepWalkingRoutingResult;

  class ResultVisitorBase {
  public:
    virtual void visitSingleCalculationResult(const SingleCalculationResult& result) = 0;
    virtual void visitAlternativesResult(const AlternativesResult& result) = 0;
    virtual void visitAllNodesResult(const AllNodesResult& result) = 0;
    virtual void visitNoRoutingFoundResult(const NoRoutingFoundResult& result) = 0;
  };

  template <class T>
  class ResultVisitor : public ResultVisitorBase{
  public:
    virtual T getResult() = 0;
  };

  class StepVisitorBase {
  public:
    virtual void visitBoardingStepResult(const StepBoardingRoutingResult& step) = 0;
    virtual void visitUnboardingStepResult(const StepUnboardingRoutingResult& step) = 0;
    virtual void visitWalkingStepResult(const StepWalkingRoutingResult& step) = 0;
  };

  template <class T>
  class StepVisitor : public StepVisitorBase {
  public:
    virtual T getResult() = 0;
  };

  // TODO Will be renamed to RoutingResult in later patch, when the struct is removed and this class is used instead
  class RoutingResultNew {
  public:
    enum result_type resType;
    RoutingResultNew(result_type _resType): resType(_resType) {}
    template<class T>
    T accept(ResultVisitor<T> &visitor) {
        do_accept(visitor);
        return visitor.getResult();
    }
    virtual void do_accept(ResultVisitorBase &visitor) const = 0;
  };

  class StepRoutingResult {
  public:
    enum result_step_type action;
    StepRoutingResult(
      result_step_type _action
    ): action(_action) {}
    template<class T>
    T accept(StepVisitor<T> &visitor) {
        do_accept(visitor);
        return visitor.getResult();
    }
    virtual void do_accept(StepVisitorBase &visitor) const = 0;
  };

  class StepTransitRoutingResult : public StepRoutingResult {
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
    StepTransitRoutingResult(
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
    ): StepRoutingResult(_action),
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

  class StepBoardingRoutingResult : public StepTransitRoutingResult {
  public:
    int departureTime;
    int waitingTime;
    StepBoardingRoutingResult(
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
    ): StepTransitRoutingResult(result_step_type::BOARDING, agencyUuid, _agencyAcronym, _agencyName,
      _lineUuid, _lineShortname, _lineLongname, _pathUuid,
      _modeName, _mode, _tripUuid, _legSequenceInTrip,
      _stopSequenceInTrip, _nodeUuid, _nodeCode, _nodeName,
      _nodeCoordinates), departureTime(_departureTime), waitingTime(_waitingTime)
    {}
    void do_accept(StepVisitorBase &visitor) const override {
      return visitor.visitBoardingStepResult(*this);
    }
  };

  class StepUnboardingRoutingResult : public StepTransitRoutingResult {
  public:
    int arrivalTime;
    int inVehicleTime;
    int inVehicleDistanceMeters;
    StepUnboardingRoutingResult(
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
    ): StepTransitRoutingResult(result_step_type::UNBOARDING, _agencyUuid, _agencyAcronym, _agencyName,
      _lineUuid, _lineShortname, _lineLongname, _pathUuid,
      _modeName, _mode, _tripUuid, _legSequenceInTrip,
      _stopSequenceInTrip, _nodeUuid, _nodeCode, _nodeName,
      _nodeCoordinates), arrivalTime(_arrivalTime), inVehicleTime(_inVehicleTime),
      inVehicleDistanceMeters(_inVehicleDistanceMeters)
    {}
    void do_accept(StepVisitorBase &visitor) const override {
      return visitor.visitUnboardingStepResult(*this);
    }
  };

  class StepWalkingRoutingResult : public StepRoutingResult{
  public:
    enum walking_step_type walkingType;
    int travelTime;
    int distanceMeters;
    int departureTime;
    int arrivalTime;
    int readyToBoardAt;
    StepWalkingRoutingResult(walking_step_type walkType,
      int _travelTime,
      int _distanceMeters,
      int _departureTime,
      int _arrivalTime,
      int _readyToBoardAt = -1
    ): StepRoutingResult(result_step_type::WALKING),
      walkingType(walkType),
      travelTime(_travelTime),
      distanceMeters(_distanceMeters),
      departureTime(_departureTime),
      arrivalTime(_arrivalTime),
      readyToBoardAt(_readyToBoardAt)
    {}
    void do_accept(StepVisitorBase &visitor) const override {
      return visitor.visitWalkingStepResult(*this);
    }
  };

  class SingleCalculationResult : public RoutingResultNew {
  public:
    RouteParameters& params;
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
    std::vector<std::unique_ptr<StepRoutingResult>> steps;
    SingleCalculationResult(RouteParameters& _params):
      RoutingResultNew(result_type::SINGLE_CALCULATION),
      params(_params)
    {}
    void do_accept(ResultVisitorBase &visitor) const override {
      return visitor.visitSingleCalculationResult(*this);
    }
  };

  class NoRoutingFoundResult : public RoutingResultNew {
  public:
    /** No fields required */
    RouteParameters& params;
    NoRoutingFoundResult(RouteParameters& _params):
      RoutingResultNew(result_type::NO_ROUTING_FOUND),
      params(_params)
    {}
    void do_accept(ResultVisitorBase &visitor) const override {
      return visitor.visitNoRoutingFoundResult(*this);
    }
  };

  class AlternativesResult : public RoutingResultNew {
  public:
    // Results should be of SingleCalculationResult, but it's not possible to cast a unique_ptr
    int totalAlternativesCalculated;
    std::vector<std::unique_ptr<RoutingResultNew>> alternatives;
    AlternativesResult(): RoutingResultNew(result_type::ALTERNATIVES) {}
    void do_accept(ResultVisitorBase &visitor) const override {
      return visitor.visitAlternativesResult(*this);
    }
  };

  class AccessibleNodesResult {
  public:
    boost::uuids::uuid nodeUuid;
    int arrivalTime;
    int totalTravelTime;
    int numberOfTransfers;
    AccessibleNodesResult(boost::uuids::uuid _uuid,
      int _arrivalTime,
      int _totalTravelTime,
      int _numberOfTransfers
    ): nodeUuid(_uuid),
      arrivalTime(_arrivalTime),
      totalTravelTime(_totalTravelTime),
      numberOfTransfers(_numberOfTransfers)
    {}
  };

  class AllNodesResult : public RoutingResultNew {
  public:
    std::vector<AccessibleNodesResult> nodes;
    int numberOfReachableNodes;
    float percentOfReachableNodes;
    AllNodesResult(): RoutingResultNew(result_type::ALL_NODES) {}
    void do_accept(ResultVisitorBase &visitor) const override {
      return visitor.visitAllNodesResult(*this);
    }
  };

  // TODO: This type will be removed in later commits
  struct RoutingResult {

    int travelTimeSeconds;
    int arrivalTimeSeconds;
    int departureTimeSeconds;
    int initialDepartureTimeSeconds;
    int initialLostTimeAtDepartureSeconds;
    int numberOfTransfers;
    int inVehicleTravelTimeSeconds;
    int transferTravelTimeSeconds;
    int waitingTimeSeconds;
    int accessTravelTimeSeconds;
    int egressTravelTimeSeconds;
    int transferWaitingTimeSeconds;
    int firstWaitingTimeSeconds;
    int nonTransitTravelTimeSeconds;
    int calculationTimeMilliseconds;
    std::string status;
    nlohmann::json json;
    std::vector<boost::uuids::uuid> lineUuids;
    std::vector<boost::uuids::uuid> tripUuids;
    std::vector<boost::uuids::uuid> boardingNodeUuids;
    std::vector<boost::uuids::uuid> unboardingNodeUuids;
    std::vector<boost::uuids::uuid> agencyUuids;
    std::vector<std::string>        modeShortnames;
    std::vector<int>                tripsIdx;
    std::vector<int>                linesIdx;
    std::vector<int>                inVehicleTravelTimesSeconds;
    std::vector<std::tuple<int, int, int, int, int>> legs; // tuple: tripIdx, lineIdx, pathIdx, start connection index, end connection index

  };

}

#endif // TR_ROUTING_RESULT
