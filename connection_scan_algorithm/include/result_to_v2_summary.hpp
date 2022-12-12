#ifndef TR_RESULT_TO_V2_SUMMARY_RESPONSE
#define TR_RESULT_TO_V2_SUMMARY_RESPONSE

#include "json.hpp"
#include "routing_result.hpp"

namespace TrRouting
{

  class RouteParameters;

  /**
   * @brief Convert a result object to a json object for the version 2 trRouting summary API
   */
  class ResultToV2SummaryResponse {
  public:
    static nlohmann::json resultToJsonString(AlternativesResult& result, RouteParameters& params);
    static nlohmann::json resultToJsonString(SingleCalculationResult& result, RouteParameters& params);
    static nlohmann::json noRoutingFoundResponse(RouteParameters& params, NoRoutingReason noRoutingReason);
  };

}


#endif // TR_RESULT_TO_V2_SUMMARY_RESPONSE
