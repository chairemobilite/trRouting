#include "json.hpp"
#include "constants.hpp"
#include "result_to_v2.hpp"
#include "toolbox.hpp"
#include "parameters.hpp"
#include "routing_result.hpp"


namespace TrRouting
{
  const std::string NO_ROUTING_REASON_DEFAULT = "NO_ROUTING_FOUND";
  const std::string NO_ROUTING_REASON_NO_ACCESS_AT_ORIGIN = "NO_ACCESS_AT_ORIGIN";
  const std::string NO_ROUTING_REASON_NO_ACCESS_AT_DESTINATION = "NO_ACCESS_AT_DESTINATION";
  const std::string NO_ROUTING_REASON_NO_SERVICE_FROM_ORIGIN = "NO_SERVICE_FROM_ORIGIN";
  const std::string NO_ROUTING_REASON_NO_SERVICE_TO_DESTINATION = "NO_SERVICE_TO_DESTINATION";

  /**
   * @brief Visitor for the result's steps
   */
  class StepToV2Visitor: public StepVisitor<nlohmann::json> {
  private:
    nlohmann::json response;
  public:
    nlohmann::json getResult() { return response; }
    void visitBoardingStep(const BoardingStep& step) override;
    void visitUnboardingStep(const UnboardingStep& step) override;
    void visitWalkingStep(const WalkingStep& step) override;
  };

  /**
   * @brief Visitor for the result object
   */
  class ResultToV2Visitor: public ResultVisitor<nlohmann::json> {
  private:
    nlohmann::json response;
    RouteParameters& params;
  public:
    ResultToV2Visitor(RouteParameters& _params): params(_params) {
      // Nothing to initialize
    }
    nlohmann::json getResult() { return response; }
    void visitSingleCalculationResult(const SingleCalculationResult& result) override;
    void visitAlternativesResult(const AlternativesResult& result) override;
    void visitAllNodesResult(const AllNodesResult& result) override;
  };

  void StepToV2Visitor::visitBoardingStep(const BoardingStep& step)
  {
    nlohmann::json stepJson;
    stepJson["action"] = "boarding";
    stepJson["agencyAcronym"] = step.agencyAcronym;
    stepJson["agencyName"] = step.agencyName;
    stepJson["agencyUuid"] = boost::uuids::to_string(step.agencyUuid);
    stepJson["lineShortname"] = step.lineShortname;
    stepJson["lineLongname"] = step.lineLongname;
    stepJson["lineUuid"] = boost::uuids::to_string(step.lineUuid);
    stepJson["pathUuid"] = boost::uuids::to_string(step.pathUuid);
    stepJson["modeName"] = step.modeName;
    stepJson["mode"] = step.mode;
    stepJson["tripUuid"] = boost::uuids::to_string(step.tripUuid);
    stepJson["legSequenceInTrip"] = step.legSequenceInTrip;
    stepJson["stopSequenceInTrip"] = step.stopSequenceInTrip;
    stepJson["nodeName"] = step.nodeName;
    stepJson["nodeCode"] = step.nodeCode;
    stepJson["nodeUuid"] = boost::uuids::to_string(step.nodeUuid);
    stepJson["nodeCoordinates"] = {step.nodeCoordinates.longitude, step.nodeCoordinates.latitude};
    stepJson["departureTime"] = step.departureTime;
    stepJson["waitingTime"] = step.waitingTime;
    response = stepJson;
  }

  void StepToV2Visitor::visitUnboardingStep(const UnboardingStep& step)
  {
    nlohmann::json stepJson;
    stepJson["action"] = "unboarding";
    stepJson["agencyAcronym"] = step.agencyAcronym;
    stepJson["agencyName"] = step.agencyName;
    stepJson["agencyUuid"] = boost::uuids::to_string(step.agencyUuid);
    stepJson["lineShortname"] = step.lineShortname;
    stepJson["lineLongname"] = step.lineLongname;
    stepJson["lineUuid"] = boost::uuids::to_string(step.lineUuid);
    stepJson["pathUuid"] = boost::uuids::to_string(step.pathUuid);
    stepJson["modeName"] = step.modeName;
    stepJson["mode"] = step.mode;
    stepJson["tripUuid"] = boost::uuids::to_string(step.tripUuid);
    stepJson["legSequenceInTrip"] = step.legSequenceInTrip;
    stepJson["stopSequenceInTrip"] = step.stopSequenceInTrip;
    stepJson["nodeName"] = step.nodeName;
    stepJson["nodeCode"] = step.nodeCode;
    stepJson["nodeUuid"] = boost::uuids::to_string(step.nodeUuid);
    stepJson["nodeCoordinates"] = {step.nodeCoordinates.longitude, step.nodeCoordinates.latitude};
    stepJson["arrivalTime"] = step.arrivalTime;
    stepJson["inVehicleTime"] = step.inVehicleTime;
    stepJson["inVehicleDistanceMeters"] = step.inVehicleDistanceMeters;
    response = stepJson;
  }

  void StepToV2Visitor::visitWalkingStep(const WalkingStep& step)
  {
    nlohmann::json stepJson;
    stepJson["action"] = "walking";
    stepJson["type"] = step.walkingType == walking_step_type::ACCESS ? "access" : step.walkingType == walking_step_type::EGRESS ? "egress" : "transfer";
    stepJson["travelTime"] = step.travelTime;
    stepJson["distanceMeters"] = step.distanceMeters;
    stepJson["departureTime"] = step.departureTime;
    stepJson["arrivalTime"] = step.arrivalTime;
    if (step.walkingType != walking_step_type::EGRESS) {
      stepJson["readyToBoardAt"] = step.readyToBoardAt;
    }
    response = stepJson;
  }

  void ResultToV2Visitor::visitSingleCalculationResult(const SingleCalculationResult& result)
  {
    nlohmann::json json;
    json["status"] = STATUS_SUCCESS;
    json["origin"] = { params.getOrigin()->longitude, params.getOrigin()->latitude };
    json["destination"] = { params.getDestination()->longitude, params.getDestination()->latitude };
    json["timeOfTrip"] = params.getTimeOfTrip();
    json["timeType"] = params.isForwardCalculation() ? 0 : 1;
    json["departureTime"] = result.departureTime;
    json["arrivalTime"] = result.arrivalTime;
    json["totalTravelTime"] = result.totalTravelTime;
    json["totalDistanceMeters"] = result.totalDistance;
    json["totalInVehicleTime"] = result.totalInVehicleTime;
    json["totalInVehicleDistanceMeters"] = result.totalInVehicleDistance;
    json["totalNonTransitTravelTime"] = result.totalNonTransitTravelTime;
    json["totalNonTransitDistanceMeters"] = result.totalNonTransitDistance;
    json["numberOfBoardings"] = result.numberOfBoardings;
    json["numberOfTransfers"] = result.numberOfTransfers;
    json["transferWalkingTime"] = result.transferWalkingTime;
    json["transferWalkingDistanceMeters"] = result.transferWalkingDistance;
    json["accessTravelTime"] = result.accessTravelTime;
    json["accessDistanceMeters"] = result.accessDistance;
    json["egressTravelTime"] = result.egressTravelTime;
    json["egressDistanceMeters"] = result.egressDistance;
    json["transferWaitingTime"] = result.transferWaitingTime;
    json["firstWaitingTime"] = result.firstWaitingTime;
    json["totalWaitingTime"] = result.totalWaitingTime;
    json["steps"] = nlohmann::json::array();

    // convert the steps
    StepToV2Visitor stepVisitor = StepToV2Visitor();
    for (auto &step : result.steps) {
      json["steps"].push_back(step.get()->accept(stepVisitor));
    }
    response = json;
  }

  void ResultToV2Visitor::visitAlternativesResult(const AlternativesResult& result)
  {
    nlohmann::json json;
    json["status"] = STATUS_SUCCESS;
    json["alternatives"] = nlohmann::json::array();
    for (auto &alternative : result.alternatives) {
      nlohmann::json alternativeJson = alternative.get()->accept(*this);
      json["alternatives"].push_back(alternativeJson);
    }
    json["alternativesTotal"] = result.totalAlternativesCalculated;
    response = json;
  }

  void ResultToV2Visitor::visitAllNodesResult(const AllNodesResult& result)
  {
    // TODO This type of result is not defined in v2 yet, we should not be here
    nlohmann::json json;
    json["status"] = "not_implemented_yet";
    response = json;
  }

  nlohmann::json ResultToV2Response::noRoutingFoundResponse(RouteParameters& params, NoRoutingReason noRoutingReason)
  {
    nlohmann::json json;
    json["status"] = STATUS_NO_ROUTING_FOUND;
    json["origin"] = { params.getOrigin()->longitude, params.getOrigin()->latitude };
    json["destination"] = { params.getDestination()->longitude, params.getDestination()->latitude };
    json["timeOfTrip"] = params.getTimeOfTrip();
    json["timeType"] = params.isForwardCalculation() ? 0 : 1;
    std::string reason;
    switch(noRoutingReason) {
      case NoRoutingReason::NO_ROUTING_FOUND:
        reason = NO_ROUTING_REASON_DEFAULT;
        break;
      case NoRoutingReason::NO_ACCESS_AT_ORIGIN:
        reason = NO_ROUTING_REASON_NO_ACCESS_AT_ORIGIN;
        break;
      case NoRoutingReason::NO_ACCESS_AT_DESTINATION:
        reason = NO_ROUTING_REASON_NO_ACCESS_AT_DESTINATION;
        break;
      case NoRoutingReason::NO_SERVICE_FROM_ORIGIN:
        reason = NO_ROUTING_REASON_NO_SERVICE_FROM_ORIGIN;
        break;
      case NoRoutingReason::NO_SERVICE_TO_DESTINATION:
        reason = NO_ROUTING_REASON_NO_SERVICE_TO_DESTINATION;
        break;
      default:
        reason = NO_ROUTING_REASON_DEFAULT;
        break;
    }
    json["reason"] = reason;
    return json;
  }

  nlohmann::json ResultToV2Response::resultToJsonString(RoutingResult& result, RouteParameters& params)
  {
    ResultToV2Visitor visitor = ResultToV2Visitor(params);
    return result.accept(visitor);
  }
}