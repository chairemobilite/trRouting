#include <errno.h>

#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "csa_result_to_response_test.hpp"
#include "routing_result.hpp"
#include "json.hpp"
#include "constants.hpp"
#include "result_to_v2_summary.hpp"

class ResultToV2SummaryFixtureTest : public ResultToResponseFixtureTest
{
protected:
    void assertResultConversion(nlohmann::json jsonResponse, TrRouting::SingleCalculationResult &result, TrRouting::RouteParameters &params) override;
    void assertResultConversion(nlohmann::json jsonResponse, TrRouting::BoardingStep& boardingStep, int count, TrRouting::RouteParameters &params);

public:

};

TEST_F(ResultToV2SummaryFixtureTest, TestNoRoutingFoundResultV2Summary)
{
    nlohmann::json jsonResponse = TrRouting::ResultToV2SummaryResponse::noRoutingFoundResponse(*testParameters.get(), TrRouting::NoRoutingReason::NO_ROUTING_FOUND);

    TrRouting::Point* origin = testParameters.get()->getOrigin();
    TrRouting::Point* destination = testParameters.get()->getDestination();

    ASSERT_EQ(STATUS_SUCCESS, jsonResponse["status"]);
    ASSERT_EQ(origin->latitude, jsonResponse["origin"][1]);
    ASSERT_EQ(origin->longitude, jsonResponse["origin"][0]);
    ASSERT_EQ(destination->latitude, jsonResponse["destination"][1]);
    ASSERT_EQ(destination->longitude, jsonResponse["destination"][0]);
    ASSERT_EQ(testParameters.get()->getTimeOfTrip(), jsonResponse["timeOfTrip"]);
    ASSERT_EQ(0, jsonResponse["timeType"]);
    ASSERT_EQ(0u, jsonResponse["lines"].size());
}

TEST_F(ResultToV2SummaryFixtureTest, TestSingleCalculationResultV2Summary)
{
    std::unique_ptr<TrRouting::SingleCalculationResult> resultPtr = getSingleResult();
    TrRouting::SingleCalculationResult &result = *resultPtr.get();
    TrRouting::BoardingStep& boardingStep = dynamic_cast<TrRouting::BoardingStep&>(*result.steps[1].get());

    nlohmann::json jsonResponse = TrRouting::ResultToV2SummaryResponse::resultToJsonString(result, *testParameters.get());

    assertResultConversion(jsonResponse, boardingStep, 1, *testParameters.get());
}

TEST_F(ResultToV2SummaryFixtureTest, TestAlternativesResultV2Summary)
{
    TrRouting::AlternativesResult result = TrRouting::AlternativesResult();
    result.totalAlternativesCalculated = 4;

    // Add results, both are the same, it's not important
    std::unique_ptr<TrRouting::SingleCalculationResult> resultPtr1 = getSingleResult();
    std::unique_ptr<TrRouting::SingleCalculationResult> resultPtr2 = getSingleResult();
    TrRouting::BoardingStep& boardingStep = dynamic_cast<TrRouting::BoardingStep&>(*resultPtr2.get()->steps[1].get());
    result.alternatives.push_back(std::move(resultPtr1));
    result.alternatives.push_back(std::move(resultPtr2));

    nlohmann::json jsonResponse =  TrRouting::ResultToV2SummaryResponse::resultToJsonString(result, *testParameters.get());

    assertResultConversion(jsonResponse, boardingStep, 2, *testParameters.get());
}

// Matches the single result returned by getSingleResult, with some hard-coded values. If necessary, it will need to be adapted to match any result
void ResultToV2SummaryFixtureTest::assertResultConversion(nlohmann::json jsonResponse, TrRouting::BoardingStep& boardingStep, int count, TrRouting::RouteParameters &params) {
    TrRouting::Point* origin = params.getOrigin();
    TrRouting::Point* destination = params.getDestination();

    // Validate main results
    ASSERT_EQ(STATUS_SUCCESS, jsonResponse["status"]);
    ASSERT_EQ(origin->latitude, jsonResponse["origin"][1]);
    ASSERT_EQ(origin->longitude, jsonResponse["origin"][0]);
    ASSERT_EQ(destination->latitude, jsonResponse["destination"][1]);
    ASSERT_EQ(destination->longitude, jsonResponse["destination"][0]);
    ASSERT_EQ(params.getTimeOfTrip(), jsonResponse["timeOfTrip"]);
    ASSERT_EQ(0, jsonResponse["timeType"]);
    ASSERT_EQ(count, jsonResponse["nbAlternativesCalculated"]);
    ASSERT_EQ(1u, jsonResponse["lines"].size());

    // Test the lines
    ASSERT_EQ(boardingStep.agencyAcronym, jsonResponse["lines"][0]["agencyAcronym"]);
    ASSERT_EQ(boardingStep.agencyName, jsonResponse["lines"][0]["agencyName"]);
    ASSERT_EQ(boost::uuids::to_string(boardingStep.agencyUuid), jsonResponse["lines"][0]["agencyUuid"]);
    ASSERT_EQ(boardingStep.lineShortname, jsonResponse["lines"][0]["lineShortname"]);
    ASSERT_EQ(boardingStep.lineLongname, jsonResponse["lines"][0]["lineLongname"]);
    ASSERT_EQ(boost::uuids::to_string(boardingStep.lineUuid), jsonResponse["lines"][0]["lineUuid"]);
   
}

void ResultToV2SummaryFixtureTest::assertResultConversion(nlohmann::json jsonResponse, TrRouting::SingleCalculationResult &result, TrRouting::RouteParameters &params) {
    // Not called by this test
}

