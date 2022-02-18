#ifndef TR_RESULT_TO_RESPONSE
#define TR_RESULT_TO_RESPONSE

#include "json.hpp"
#include "routing_result.hpp"
#include "parameters.hpp"

namespace TrRouting
{
  class ResultToResponse {
  public:
    virtual nlohmann::json resultToJsonString(RoutingResultNew& result) = 0;
  };

  class ResultToV1Response: public ResultToResponse {
  public:
    nlohmann::json resultToJsonString(RoutingResultNew& result) override;
  };

}

#endif // TR_RESULT_TO_RESPONSE
