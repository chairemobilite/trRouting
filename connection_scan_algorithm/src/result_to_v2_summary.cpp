#include "json.hpp"
#include "constants.hpp"
#include "result_to_v2_summary.hpp"
#include "toolbox.hpp"
#include "parameters.hpp"
#include "routing_result.hpp"


namespace TrRouting
{
  // FIXME See issue #180, this class when it's fixed, we can use the line objects directly
  class LineSummary {
  public:
    std::string agencyUuid;
    std::string agencyAcronym;
    std::string agencyName;
    std::string lineUuid;
    std::string lineShortname;
    std::string lineLongname;
    int count;
    LineSummary(
      std::string _agencyUuid,
      std::string _agencyAcronym,
      std::string _agencyName,
      std::string _lineUuid,
      std::string _lineShortname,
      std::string _lineLongname
    ): agencyUuid(_agencyUuid),
    agencyAcronym(_agencyAcronym),
    agencyName(_agencyName),
    lineUuid(_lineUuid),
    lineShortname(_lineShortname),
    lineLongname(_lineLongname)
    {
        count = 1;
    }

    LineSummary(const LineSummary& obj)
    {
      agencyUuid = obj.agencyUuid;
      agencyAcronym = obj.agencyAcronym;
      agencyName = obj.agencyName;
      lineUuid = obj.lineUuid;
      lineShortname = obj.lineShortname;
      lineLongname = obj.lineLongname;
      count = obj.count;
    }
  };

  /**
   * @brief Visitor for the result's steps
   */
  class StepToV2SummaryVisitor: public StepVisitor<std::optional<LineSummary>> {
  private:
    std::optional<LineSummary> response;
  public:
    std::optional<LineSummary> getResult() { return response; }
    void visitBoardingStep(const BoardingStep& step) override;
    void visitUnboardingStep(const UnboardingStep& step) override;
    void visitWalkingStep(const WalkingStep& step) override;
  };

  /**
   * @brief Visitor for the result object, returns a map of line uuid to LineSummary
   */
  class ResultToV2SummaryVisitor: public ResultVisitor<std::map<std::string, LineSummary>> {
  private:
    std::map<std::string, LineSummary> response;
    RouteParameters& params;
  public:
    ResultToV2SummaryVisitor(RouteParameters& _params): params(_params) {
      // Nothing to initialize
    }
    std::map<std::string, LineSummary> getResult() { return response; }
    void visitSingleCalculationResult(const SingleCalculationResult& result) override;
    void visitAlternativesResult(const AlternativesResult& result) override;
    void visitAllNodesResult(const AllNodesResult& result) override;
  };

  /**
   * @brief Visitor for the result object, returns a map of line uuid to LineSummary
   */
  class CountAlternativesResultVisitor: public ResultVisitor<int> {
  private:
    int response;
  public:
    CountAlternativesResultVisitor(): response(0) {
      // Nothing to initialize
    }
    int getResult() { return response; }
    void visitSingleCalculationResult(const SingleCalculationResult& ) {
      response = 1;
    }
    void visitAlternativesResult(const AlternativesResult& result) {
      response = result.alternatives.size();
    }
    void visitAllNodesResult(const AllNodesResult& ) {
      // Nothing to do
    }
  };

  void StepToV2SummaryVisitor::visitBoardingStep(const BoardingStep& step)
  {
    response.emplace(LineSummary(
        boost::uuids::to_string(step.agencyUuid),
        step.agencyAcronym,
        step.agencyName,
        boost::uuids::to_string(step.lineUuid),
        step.lineShortname,
        step.lineLongname
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

  void ResultToV2SummaryVisitor::visitSingleCalculationResult(const SingleCalculationResult& result)
  {
    // convert the steps
    StepToV2SummaryVisitor stepVisitor = StepToV2SummaryVisitor();
    for (auto &step : result.steps) {
      std::optional<LineSummary> optSummary = step.get()->accept(stepVisitor);
      if (optSummary.has_value()) {
        LineSummary summary = optSummary.value();
        if (response.find(summary.lineUuid) == response.end()) {
          response.emplace(summary.lineUuid, LineSummary(summary));
        } else {
          response.at(summary.lineUuid).count++;
        }
      }
    }
  }

  void ResultToV2SummaryVisitor::visitAlternativesResult(const AlternativesResult& result)
  {
    for (auto &alternative : result.alternatives) {
      alternative.get()->accept(*this);
    }
  }

  void ResultToV2SummaryVisitor::visitAllNodesResult(const AllNodesResult& )
  {
    // TODO This type of result is not defined in v2 yet, we should not be here
  }

  nlohmann::json ResultToV2SummaryResponse::noRoutingFoundResponse(RouteParameters& params, NoRoutingReason )
  {
    nlohmann::json json;
    json["status"] = STATUS_SUCCESS;
    json["origin"] = { params.getOrigin()->longitude, params.getOrigin()->latitude };
    json["destination"] = { params.getDestination()->longitude, params.getDestination()->latitude };
    json["timeOfTrip"] = params.getTimeOfTrip();
    json["timeType"] = params.isForwardCalculation() ? 0 : 1;
    json["nbAlternativesCalculated"] = 0;
    json["lines"] = nlohmann::json::array();
    
    return json;
  }

  nlohmann::json ResultToV2SummaryResponse::resultToJsonString(RoutingResult& result, RouteParameters& params)
  {
    // Initialize response
    nlohmann::json json;
    json["status"] = STATUS_SUCCESS;
    json["origin"] = { params.getOrigin()->longitude, params.getOrigin()->latitude };
    json["destination"] = { params.getDestination()->longitude, params.getDestination()->latitude };
    json["timeOfTrip"] = params.getTimeOfTrip();
    json["timeType"] = params.isForwardCalculation() ? 0 : 1;
    CountAlternativesResultVisitor countVisitor = CountAlternativesResultVisitor();
    json["nbAlternativesCalculated"] = result.accept(countVisitor);
    json["lines"] = nlohmann::json::array();

    ResultToV2SummaryVisitor visitor = ResultToV2SummaryVisitor(params);
    std::map<std::string, LineSummary> summaries = result.accept(visitor);
    for (auto it = summaries.begin(); it != summaries.end(); ++it) {
      nlohmann::json lineJson;
      LineSummary summary = it->second;
      lineJson["lineUuid"] = summary.lineUuid;
      lineJson["lineShortname"] = summary.lineShortname;
      lineJson["lineLongname"] = summary.lineLongname;
      lineJson["agencyUuid"] = summary.agencyUuid;
      lineJson["agencyAcronym"] = summary.agencyAcronym;
      lineJson["agencyName"] = summary.agencyName;
      lineJson["alternativeCount"] = summary.count;
      json["lines"].push_back(lineJson);
    }

    return json;
    
  }
}
