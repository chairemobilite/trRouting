#include "json.hpp"
#include "constants.hpp"
#include "result_to_v2_summary.hpp"
#include "toolbox.hpp"
#include "parameters.hpp"
#include "routing_result.hpp"
#include "trip.hpp"
#include "agency.hpp"
#include "line.hpp"

namespace TrRouting
{
  class LineSummary {
  public:
    const Trip & trip;
    int count;
    LineSummary(
      const Trip & _trip
    ): trip(_trip)
    {
      count = 1;
    }

    LineSummary(const LineSummary& obj): trip(obj.trip)
    {
      count = obj.count;
    }
    ~LineSummary(){}
  };

  /**
   * @brief Visitor for the result's steps
   */
  class StepToV2SummaryVisitor: public StepVisitor<std::optional<LineSummary>> {
  private:
    std::optional<LineSummary> response;
  public:
    std::optional<LineSummary> getResult() override { return response; }
    void visitBoardingStep(const BoardingStep& step) override;
    void visitUnboardingStep(const UnboardingStep& step) override;
    void visitWalkingStep(const WalkingStep& step) override;
  };

  void StepToV2SummaryVisitor::visitBoardingStep(const BoardingStep& step)
  {
    response.emplace(LineSummary(
        step.trip
    ));
  }

  void StepToV2SummaryVisitor::visitUnboardingStep(const UnboardingStep& )
  {
    response.reset();
  }

  void StepToV2SummaryVisitor::visitWalkingStep(const WalkingStep& )
  {
    response.reset();
  }

  /**
   * @brief Parser for calculation results to add statistics to a line summary map
   */
  class SummaryResultAccumulator {
  private:
    std::map<boost::uuids::uuid, LineSummary> lineSummaries;
  public:
    SummaryResultAccumulator() {
      // Nothing to initialize
    }
    const std::map<boost::uuids::uuid, LineSummary> getLineSummaries() const { return lineSummaries; }
    void processSingleCalculationResult(const SingleCalculationResult& result);
  };

  void SummaryResultAccumulator::processSingleCalculationResult(const SingleCalculationResult& result)
  {
    // convert the steps
    StepToV2SummaryVisitor stepVisitor = StepToV2SummaryVisitor();
    for (auto &step : result.steps) {
      std::optional<LineSummary> optSummary = step.get()->accept(stepVisitor);
      if (optSummary.has_value()) {
        LineSummary summary = optSummary.value();
        if (lineSummaries.find(summary.trip.line.uuid) == lineSummaries.end()) {
          lineSummaries.emplace(summary.trip.line.uuid, LineSummary(summary));
        } else {
          lineSummaries.at(summary.trip.line.uuid).count++;
        }
      }
    }
  }

  std::array<double, 2> pointToJson(const Point& point)
  {
    return { point.longitude, point.latitude };
  }

  nlohmann::json parametersToQueryResponse(RouteParameters& params)
  {
    // Query parameters for response
    nlohmann::json queryJson;
    queryJson["origin"] = pointToJson(*params.getOrigin());
    queryJson["destination"] = pointToJson(*params.getDestination());
    queryJson["timeOfTrip"] = params.getTimeOfTrip();
    queryJson["timeType"] = params.isForwardCalculation() ? 0 : 1;
    return queryJson;
  }

  nlohmann::json ResultToV2SummaryResponse::noRoutingFoundResponse(RouteParameters& params, NoRoutingReason )
  {
    nlohmann::json json;
    // No routing returns a success with 0 routes
    json["status"] = STATUS_SUCCESS;
    json["query"] = parametersToQueryResponse(params);

    // Result response
    nlohmann::json resultJson;
    resultJson["nbRoutes"] = 0;
    resultJson["lines"] = nlohmann::json::array();
    json["result"] = resultJson;
    
    return json;
  }

  nlohmann::json lineSummariesToJson(const SummaryResultAccumulator & parser)
  {
    nlohmann::json lineResult;
    lineResult["lines"] = nlohmann::json::array();
    const std::map<boost::uuids::uuid, LineSummary> summaries = parser.getLineSummaries();
    for (auto it = summaries.begin(); it != summaries.end(); ++it) {
      nlohmann::json lineJson;
      LineSummary summary = it->second;
      lineJson["lineUuid"] = boost::uuids::to_string(summary.trip.line.uuid);
      lineJson["lineShortname"] = summary.trip.line.shortname;
      lineJson["lineLongname"] = summary.trip.line.longname;
      lineJson["agencyUuid"] = boost::uuids::to_string(summary.trip.line.agency.uuid);
      lineJson["agencyAcronym"] = summary.trip.line.agency.acronym;
      lineJson["agencyName"] = summary.trip.line.agency.name;
      lineJson["alternativeCount"] = summary.count;
      lineResult["lines"].push_back(lineJson);
    }
    return lineResult["lines"];
  }

  nlohmann::json ResultToV2SummaryResponse::resultToJsonString(AlternativesResult& result, RouteParameters& params)
  {
    // Initialize response
    nlohmann::json json;
    json["status"] = STATUS_SUCCESS;
    json["query"] = parametersToQueryResponse(params);

    SummaryResultAccumulator resultParser = SummaryResultAccumulator();
    for (auto &alternative : result.alternatives) {
      resultParser.processSingleCalculationResult(*alternative.get());
    }

    // Result response
    nlohmann::json resultJson;
    resultJson["nbRoutes"] = result.alternatives.size();
    resultJson["lines"] = lineSummariesToJson(resultParser);
    json["result"] = resultJson;

    return json;
    
  }

  nlohmann::json ResultToV2SummaryResponse::resultToJsonString(SingleCalculationResult& result, RouteParameters& params)
  {
    // Initialize response
    nlohmann::json json;
    json["status"] = STATUS_SUCCESS;
    json["query"] = parametersToQueryResponse(params);

    SummaryResultAccumulator resultParser = SummaryResultAccumulator();
    resultParser.processSingleCalculationResult(result);

    // Result response
    nlohmann::json resultJson;
    resultJson["nbRoutes"] = 1;
    resultJson["lines"] = lineSummariesToJson(resultParser);
    json["result"] = resultJson;

    return json;
    
  }
}
