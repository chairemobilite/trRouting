#ifndef TR_RESULT_TO_V2_RESPONSE
#define TR_RESULT_TO_V2_RESPONSE

#include "json.hpp"
#include "routing_result.hpp"

namespace TrRouting
{

  class RouteParameters;

  /**
   * @brief Convert a result object to a json object for the version 2 trRouting API, as described in docs/APIv2/API.yml
   */
  class ResultToV2Response {
  public:
    static nlohmann::json resultToJsonString(RoutingResult& result, RouteParameters& params);
    static nlohmann::json noRoutingFoundResponse(RouteParameters& params, NoRoutingReason noRoutingReason);
  };

}


#endif // TR_RESULT_TO_V2_RESPONSE
