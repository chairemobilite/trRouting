#include "json.hpp"
#include "constants.hpp"
#include "result_constants.hpp"
#include "result_to_v2_accessibility.hpp"
#include "parameters.hpp"
#include "routing_result.hpp"
#include "node.hpp"

namespace TrRouting
{

  std::array<double, 2> pointToAccessibilityJson(const Point& point)
  {
    return { point.longitude, point.latitude };
  }

  nlohmann::json parametersToAccessibilityQueryResponse(AccessibilityParameters& params)
  {
    // Query parameters for response
    nlohmann::json queryJson;
    queryJson["place"] = pointToAccessibilityJson(*params.getPlace());
    queryJson["timeOfTrip"] = params.getTimeOfTrip();
    queryJson["timeType"] = params.isForwardCalculation() ? 0 : 1;
    return queryJson;
  }

  nlohmann::json ResultToV2AccessibilityResponse::noRoutingFoundResponse(AccessibilityParameters& params, NoRoutingReason noRoutingReason)
  {
    nlohmann::json json;
    json["status"] = STATUS_NO_ROUTING_FOUND;
    json["query"] = parametersToAccessibilityQueryResponse(params);

    std::string reason;
    switch(noRoutingReason) {
      case NoRoutingReason::NO_ROUTING_FOUND:
        reason = NO_ROUTING_REASON_DEFAULT;
        break;
      case NoRoutingReason::NO_ACCESS_AT_ORIGIN: // fall through
      case NoRoutingReason::NO_ACCESS_AT_DESTINATION:
        reason = NO_ROUTING_REASON_NO_ACCESS_AT_PLACE;
        break;
      case NoRoutingReason::NO_SERVICE_FROM_ORIGIN: // fall through
      case NoRoutingReason::NO_SERVICE_TO_DESTINATION:
        reason = NO_ROUTING_REASON_NO_SERVICE_AT_PLACE;
        break;
      default:
        reason = NO_ROUTING_REASON_DEFAULT;
        break;
    }
    json["reason"] = reason;
    return json;
  }

  nlohmann::json nodeToJson(const AccessibleNodes & node, const bool isForward)
  {
    nlohmann::json nodeResult;
    nodeResult["nodeName"] = node.node.name;
    nodeResult["nodeCode"] = node.node.code;
    nodeResult["nodeUuid"] = boost::uuids::to_string(node.node.uuid);
    nodeResult["nodeTime"] = isForward ? node.arrivalTime : node.arrivalTime - node.totalTravelTime;
    nodeResult["nodeCoordinates"] = { node.node.point->longitude, node.node.point->latitude };
    nodeResult["totalTravelTime"] = node.totalTravelTime;
    nodeResult["numberOfTransfers"] = node.numberOfTransfers;
    return nodeResult;
  }

  nlohmann::json ResultToV2AccessibilityResponse::resultToJsonString(AllNodesResult& result, AccessibilityParameters& params)
  {
    // Initialize response
    nlohmann::json json;
    json["status"] = STATUS_SUCCESS;
    json["query"] = parametersToAccessibilityQueryResponse(params);

    // Result response
    nlohmann::json resultJson;
    resultJson["totalNodeCount"] = result.totalNodeCount;
    resultJson["nodes"] = nlohmann::json::array();
    for (auto &node : result.nodes) {
      resultJson["nodes"].push_back(nodeToJson(node, params.isForwardCalculation()));
    }
    json["result"] = resultJson;

    return json;
    
  }
}
