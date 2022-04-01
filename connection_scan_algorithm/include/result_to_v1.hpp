#ifndef TR_RESULT_TO_V1_RESPONSE
#define TR_RESULT_TO_V1_RESPONSE

#include "json.hpp"
#include "routing_result.hpp"

namespace TrRouting
{

  class RouteParameters;

  /**
   * @brief Convert a result object to a json object for the version 1 trRouting API, as described in docs/API.yml
   */
  class ResultToV1Response {
  public:
    static nlohmann::json resultToJsonString(RoutingResult& result, RouteParameters& params);
    static nlohmann::json noRoutingFoundResponse(RouteParameters& params, NoRoutingReason noRoutingReason);
  };

}

#endif // TR_RESULT_TO_V1_RESPONSE
