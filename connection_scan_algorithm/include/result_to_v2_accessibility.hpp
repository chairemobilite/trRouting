#ifndef TR_RESULT_TO_V2_ACCESSIBILITY_RESPONSE
#define TR_RESULT_TO_V2_ACCESSIBILITY_RESPONSE

#include "json.hpp"
#include "routing_result.hpp"

namespace TrRouting
{

  class AccessibilityParameters;

  /**
   * @brief Convert a result object to a json object for the version 2 trRouting accessibility API
   */
  class ResultToV2AccessibilityResponse {
  public:
    static nlohmann::json resultToJsonString(AllNodesResult& result, AccessibilityParameters& params);
    static nlohmann::json noRoutingFoundResponse(AccessibilityParameters& params, NoRoutingReason noRoutingReason);
  };

}


#endif // TR_RESULT_TO_V2_ACCESSIBILITY_RESPONSE
