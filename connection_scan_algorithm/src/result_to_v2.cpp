#include "json.hpp"
#include "constants.hpp"
#include "result_to_v2.hpp"
#include "toolbox.hpp"
#include "parameters.hpp"
#include "routing_result.hpp"
#include "trip.hpp"
#include "agency.hpp"
#include "line.hpp"
#include "mode.hpp"
#include "node.hpp"
#include "path.hpp"

namespace TrRouting
{
  const std::string NO_ROUTING_REASON_DEFAULT = "NO_ROUTING_FOUND";
  const std::string NO_ROUTING_REASON_NO_ACCESS_AT_ORIGIN = "NO_ACCESS_AT_ORIGIN";
  const std::string NO_ROUTING_REASON_NO_ACCESS_AT_DESTINATION = "NO_ACCESS_AT_DESTINATION";
  const std::string NO_ROUTING_REASON_NO_SERVICE_FROM_ORIGIN = "NO_SERVICE_FROM_ORIGIN";
  const std::string NO_ROUTING_REASON_NO_SERVICE_TO_DESTINATION = "NO_SERVICE_TO_DESTINATION";
  const std::string NO_ROUTING_REASON_NO_ACCESS_AT_ORIGIN_AND_DESTINATION = "NO_ACCESS_AT_ORIGIN_AND_DESTINATION";

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

  void StepToV2Visitor::visitBoardingStep(const BoardingStep& step)
  {
    nlohmann::json stepJson;
    stepJson["action"] = "boarding";
    stepJson["agencyAcronym"] = step.trip.agency.acronym;
    stepJson["agencyName"] = step.trip.agency.name;
    stepJson["agencyUuid"] = boost::uuids::to_string(step.trip.agency.uuid);
    stepJson["lineShortname"] = step.trip.line.shortname;
    stepJson["lineLongname"] = step.trip.line.longname;
    stepJson["lineUuid"] = boost::uuids::to_string(step.trip.line.uuid);
    stepJson["pathUuid"] = boost::uuids::to_string(step.trip.path.uuid);
    stepJson["modeName"] = step.trip.line.mode.name;
    stepJson["mode"] = step.trip.line.mode.shortname;
    stepJson["tripUuid"] = boost::uuids::to_string(step.trip.uuid);
    stepJson["legSequenceInTrip"] = step.legSequenceInTrip;
    stepJson["stopSequenceInTrip"] = step.stopSequenceInTrip;
    stepJson["nodeName"] = step.node.name;
    stepJson["nodeCode"] = step.node.code;
    stepJson["nodeUuid"] = boost::uuids::to_string(step.node.uuid);
    stepJson["nodeCoordinates"] = {step.node.point->longitude, step.node.point->latitude};
    stepJson["departureTime"] = step.departureTime;
    stepJson["waitingTime"] = step.waitingTime;
    response = stepJson;
  }

  void StepToV2Visitor::visitUnboardingStep(const UnboardingStep& step)
  {
    nlohmann::json stepJson;
    stepJson["action"] = "unboarding";
    stepJson["agencyAcronym"] = step.trip.agency.acronym;
    stepJson["agencyName"] = step.trip.agency.name;
    stepJson["agencyUuid"] = boost::uuids::to_string(step.trip.agency.uuid);
    stepJson["lineShortname"] = step.trip.line.shortname;
    stepJson["lineLongname"] = step.trip.line.longname;
    stepJson["lineUuid"] = boost::uuids::to_string(step.trip.line.uuid);
    stepJson["pathUuid"] = boost::uuids::to_string(step.trip.path.uuid);
    stepJson["modeName"] = step.trip.line.mode.name;
    stepJson["mode"] = step.trip.line.mode.shortname;
    stepJson["tripUuid"] = boost::uuids::to_string(step.trip.uuid);
    stepJson["legSequenceInTrip"] = step.legSequenceInTrip;
    stepJson["stopSequenceInTrip"] = step.stopSequenceInTrip;
    stepJson["nodeName"] = step.node.name;
    stepJson["nodeCode"] = step.node.code;
    stepJson["nodeUuid"] = boost::uuids::to_string(step.node.uuid);
    stepJson["nodeCoordinates"] = {step.node.point->longitude, step.node.point->latitude};
    stepJson["arrivalTime"] = step.arrivalTime;
    stepJson["inVehicleTime"] = step.inVehicleTime;
    stepJson["inVehicleDistance"] = step.inVehicleDistanceMeters;
    response = stepJson;
  }

  void StepToV2Visitor::visitWalkingStep(const WalkingStep& step)
  {
    nlohmann::json stepJson;
    stepJson["action"] = "walking";
    stepJson["type"] = step.walkingType == walking_step_type::ACCESS ? "access" : step.walkingType == walking_step_type::EGRESS ? "egress" : "transfer";
    stepJson["travelTime"] = step.travelTime;
    stepJson["distance"] = step.distanceMeters;
    stepJson["departureTime"] = step.departureTime;
    stepJson["arrivalTime"] = step.arrivalTime;
    if (step.walkingType != walking_step_type::EGRESS) {
      stepJson["readyToBoardAt"] = step.readyToBoardAt;
    }
    response = stepJson;
  }

  std::array<double, 2> pointToRouteJson(const Point& point)
  {
    return { point.longitude, point.latitude };
  }

  nlohmann::json parametersToRouteQueryResponse(RouteParameters& params)
  {
    // Query parameters for response
    nlohmann::json queryJson;
    queryJson["origin"] = pointToRouteJson(*params.getOrigin());
    queryJson["destination"] = pointToRouteJson(*params.getDestination());
    queryJson["timeOfTrip"] = params.getTimeOfTrip();
    queryJson["timeType"] = params.isForwardCalculation() ? 0 : 1;
    return queryJson;
  }

  nlohmann::json getSingleResultJsonString(const SingleCalculationResult& result)
  {
    nlohmann::json json;

    // Result response for single route
    json["departureTime"] = result.departureTime;
    json["arrivalTime"] = result.arrivalTime;
    json["totalTravelTime"] = result.totalTravelTime;
    json["totalDistance"] = result.totalDistance;
    json["totalInVehicleTime"] = result.totalInVehicleTime;
    json["totalInVehicleDistance"] = result.totalInVehicleDistance;
    json["totalNonTransitTravelTime"] = result.totalNonTransitTravelTime;
    json["totalNonTransitDistance"] = result.totalNonTransitDistance;
    json["numberOfBoardings"] = result.numberOfBoardings;
    json["numberOfTransfers"] = result.numberOfTransfers;
    json["transferWalkingTime"] = result.transferWalkingTime;
    json["transferWalkingDistance"] = result.transferWalkingDistance;
    json["accessTravelTime"] = result.accessTravelTime;
    json["accessDistance"] = result.accessDistance;
    json["egressTravelTime"] = result.egressTravelTime;
    json["egressDistance"] = result.egressDistance;
    json["transferWaitingTime"] = result.transferWaitingTime;
    json["firstWaitingTime"] = result.firstWaitingTime;
    json["totalWaitingTime"] = result.totalWaitingTime;
    json["steps"] = nlohmann::json::array();

    // convert the steps
    StepToV2Visitor stepVisitor = StepToV2Visitor();
    for (auto &step : result.steps) {
      json["steps"].push_back(step.get()->accept(stepVisitor));
    }
    return json;
  }

  nlohmann::json ResultToV2Response::noRoutingFoundResponse(RouteParameters& params, NoRoutingReason noRoutingReason)
  {
    nlohmann::json json;
    json["status"] = STATUS_NO_ROUTING_FOUND;
    json["query"] = parametersToRouteQueryResponse(params);

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
      case NoRoutingReason::NO_ACCESS_AT_ORIGIN_AND_DESTINATION:
        reason = NO_ROUTING_REASON_NO_ACCESS_AT_ORIGIN_AND_DESTINATION;
        break;
      default:
        reason = NO_ROUTING_REASON_DEFAULT;
        break;
    }
    json["reason"] = reason;
    return json;
  }

  nlohmann::json ResultToV2Response::resultToJsonString(AlternativesResult& result, RouteParameters& params)
  {
    nlohmann::json json;
    json["status"] = STATUS_SUCCESS;
    json["query"] = parametersToRouteQueryResponse(params);

    nlohmann::json resultJson;
    resultJson["routes"] = nlohmann::json::array();
    for (auto &alternative : result.alternatives) {
      resultJson["routes"].push_back(getSingleResultJsonString(*alternative.get()));
    }
    resultJson["totalRoutesCalculated"] = result.totalAlternativesCalculated;
    json["result"] = resultJson;

    return json;
  }

  nlohmann::json ResultToV2Response::resultToJsonString(SingleCalculationResult& result, RouteParameters& params)
  {
    nlohmann::json json;
    json["status"] = STATUS_SUCCESS;
    json["query"] = parametersToRouteQueryResponse(params);

    nlohmann::json resultJson;
    resultJson["totalRoutesCalculated"] = 1;
    resultJson["routes"] = nlohmann::json::array();
    resultJson["routes"].push_back(getSingleResultJsonString(result));
    json["result"] = resultJson;

    return json;
  }
}
