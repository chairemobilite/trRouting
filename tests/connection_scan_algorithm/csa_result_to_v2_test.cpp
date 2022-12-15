#include <errno.h>

#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "csa_result_to_response_test.hpp"
#include "routing_result.hpp"
#include "json.hpp"
#include "constants.hpp"
#include "result_to_v2.hpp"

class ResultToV2FixtureTest : public ResultToResponseFixtureTest
{
protected:
    void assertResultConversion(nlohmann::json jsonResponse, TrRouting::SingleCalculationResult &result, TrRouting::RouteParameters &params);

public:

};

TEST_F(ResultToV2FixtureTest, TestNoRoutingFoundResultDefaultReasonV2)
{
    nlohmann::json jsonResponse = TrRouting::ResultToV2Response::noRoutingFoundResponse(*testParameters.get(), TrRouting::NoRoutingReason::NO_ROUTING_FOUND);

    TrRouting::Point* origin = testParameters.get()->getOrigin();
    TrRouting::Point* destination = testParameters.get()->getDestination();

    ASSERT_EQ(STATUS_NO_ROUTING_FOUND, jsonResponse["status"]);
    ASSERT_EQ(origin->latitude, jsonResponse["origin"][1]);
    ASSERT_EQ(origin->longitude, jsonResponse["origin"][0]);
    ASSERT_EQ(destination->latitude, jsonResponse["destination"][1]);
    ASSERT_EQ(destination->longitude, jsonResponse["destination"][0]);
    ASSERT_EQ(testParameters.get()->getTimeOfTrip(), jsonResponse["timeOfTrip"]);
    ASSERT_EQ(0, jsonResponse["timeType"]);
    ASSERT_EQ("NO_ROUTING_FOUND", jsonResponse["reason"]);
}

TEST_F(ResultToV2FixtureTest, TestNoRoutingFoundResultWithReasonV2)
{
    nlohmann::json jsonResponse = TrRouting::ResultToV2Response::noRoutingFoundResponse(*testParameters.get(), TrRouting::NoRoutingReason::NO_ACCESS_AT_ORIGIN);

    TrRouting::Point* origin = testParameters.get()->getOrigin();
    TrRouting::Point* destination = testParameters.get()->getDestination();

    ASSERT_EQ(STATUS_NO_ROUTING_FOUND, jsonResponse["status"]);
    ASSERT_EQ(origin->latitude, jsonResponse["origin"][1]);
    ASSERT_EQ(origin->longitude, jsonResponse["origin"][0]);
    ASSERT_EQ(destination->latitude, jsonResponse["destination"][1]);
    ASSERT_EQ(destination->longitude, jsonResponse["destination"][0]);
    ASSERT_EQ(testParameters.get()->getTimeOfTrip(), jsonResponse["timeOfTrip"]);
    ASSERT_EQ(0, jsonResponse["timeType"]);
    ASSERT_EQ("NO_ACCESS_AT_ORIGIN", jsonResponse["reason"]);
}

TEST_F(ResultToV2FixtureTest, TestSingleCalculationResultV2)
{
    std::unique_ptr<TrRouting::SingleCalculationResult> resultPtr = getSingleResult();
    TrRouting::SingleCalculationResult &result = *resultPtr.get();

    nlohmann::json jsonResponse = TrRouting::ResultToV2Response::resultToJsonString(result, *testParameters.get());

    assertResultConversion(jsonResponse, result, *testParameters.get());
}

TEST_F(ResultToV2FixtureTest, TestAlternativesResultV2)
{
    TrRouting::AlternativesResult result = TrRouting::AlternativesResult();
    result.totalAlternativesCalculated = 4;

    // Add results, both are the same, it's not important
    std::unique_ptr<TrRouting::SingleCalculationResult> resultPtr1 = getSingleResult();
    std::unique_ptr<TrRouting::SingleCalculationResult> resultPtr2 = getSingleResult();
    result.alternatives.push_back(std::move(resultPtr1));
    result.alternatives.push_back(std::move(resultPtr2));

    nlohmann::json jsonResponse =  TrRouting::ResultToV2Response::resultToJsonString(result, *testParameters.get());

    ASSERT_EQ(STATUS_SUCCESS, jsonResponse["status"]);
    ASSERT_EQ(result.totalAlternativesCalculated, jsonResponse["alternativesTotal"]);
    ASSERT_EQ(2u, jsonResponse["alternatives"].size());

    TrRouting::SingleCalculationResult& alternative1 = dynamic_cast<TrRouting::SingleCalculationResult&>(*result.alternatives[0].get());
    assertResultConversion(jsonResponse["alternatives"][0], alternative1, *testParameters.get());
    TrRouting::SingleCalculationResult& alternative2 = dynamic_cast<TrRouting::SingleCalculationResult&>(*result.alternatives[1].get());
    assertResultConversion(jsonResponse["alternatives"][1], alternative2, *testParameters.get());
}

// Matches the single result returned by getSingleResult, with some hard-coded values. If necessary, it will need to be adapted to match any result
void ResultToV2FixtureTest::assertResultConversion(nlohmann::json jsonResponse, TrRouting::SingleCalculationResult &result, TrRouting::RouteParameters &params) {
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
    ASSERT_EQ(result.departureTime, jsonResponse["departureTime"]);
    ASSERT_EQ(result.arrivalTime, jsonResponse["arrivalTime"]);
    ASSERT_EQ(result.totalTravelTime, jsonResponse["totalTravelTime"]);
    ASSERT_EQ(result.totalDistance, jsonResponse["totalDistance"]);
    ASSERT_EQ(result.totalInVehicleTime, jsonResponse["totalInVehicleTime"]);
    ASSERT_EQ(result.totalInVehicleDistance, jsonResponse["totalInVehicleDistance"]);
    ASSERT_EQ(result.totalNonTransitTravelTime, jsonResponse["totalNonTransitTravelTime"]);
    ASSERT_EQ(result.totalNonTransitDistance, jsonResponse["totalNonTransitDistance"]);
    ASSERT_EQ(result.numberOfBoardings, jsonResponse["numberOfBoardings"]);
    ASSERT_EQ(result.numberOfTransfers, jsonResponse["numberOfTransfers"]);
    ASSERT_EQ(result.transferWalkingTime, jsonResponse["transferWalkingTime"]);
    ASSERT_EQ(result.transferWalkingDistance, jsonResponse["transferWalkingDistance"]);
    ASSERT_EQ(result.accessTravelTime, jsonResponse["accessTravelTime"]);
    ASSERT_EQ(result.accessDistance, jsonResponse["accessDistance"]);
    ASSERT_EQ(result.egressTravelTime, jsonResponse["egressTravelTime"]);
    ASSERT_EQ(result.egressDistance, jsonResponse["egressDistance"]);
    ASSERT_EQ(result.transferWaitingTime, jsonResponse["transferWaitingTime"]);
    ASSERT_EQ(result.firstWaitingTime, jsonResponse["firstWaitingTime"]);
    ASSERT_EQ(result.totalWaitingTime, jsonResponse["totalWaitingTime"]);

    // Test the walking step
    TrRouting::WalkingStep& walkingStep = dynamic_cast<TrRouting::WalkingStep&>(*result.steps[0].get());
    ASSERT_EQ("walking", jsonResponse["steps"][0]["action"]);
    ASSERT_EQ("access", jsonResponse["steps"][0]["type"]);
    ASSERT_EQ(walkingStep.travelTime, jsonResponse["steps"][0]["travelTime"]);
    ASSERT_EQ(walkingStep.distanceMeters, jsonResponse["steps"][0]["distance"]);
    ASSERT_EQ(walkingStep.departureTime, jsonResponse["steps"][0]["departureTime"]);
    ASSERT_EQ(walkingStep.arrivalTime, jsonResponse["steps"][0]["arrivalTime"]);
    ASSERT_EQ(walkingStep.readyToBoardAt, jsonResponse["steps"][0]["readyToBoardAt"]);

    // Test the boarding step
    TrRouting::BoardingStep& boardingStep = dynamic_cast<TrRouting::BoardingStep&>(*result.steps[1].get());
    ASSERT_EQ("boarding", jsonResponse["steps"][1]["action"]);
    ASSERT_EQ(agency->acronym, jsonResponse["steps"][1]["agencyAcronym"]);
    ASSERT_EQ(agency->name, jsonResponse["steps"][1]["agencyName"]);
    ASSERT_EQ(agencyUuid, jsonResponse["steps"][1]["agencyUuid"]);
    ASSERT_EQ(line->shortname, jsonResponse["steps"][1]["lineShortname"]);
    ASSERT_EQ(line->longname, jsonResponse["steps"][1]["lineLongname"]);
    ASSERT_EQ(lineUuid, jsonResponse["steps"][1]["lineUuid"]);
    ASSERT_EQ(pathUuid, jsonResponse["steps"][1]["pathUuid"]);
    ASSERT_EQ(mode->name, jsonResponse["steps"][1]["modeName"]);
    ASSERT_EQ(mode->shortname, jsonResponse["steps"][1]["mode"]);
    ASSERT_EQ(tripUuid, jsonResponse["steps"][1]["tripUuid"]);
    ASSERT_EQ(boardingStep.legSequenceInTrip, jsonResponse["steps"][1]["legSequenceInTrip"]);
    ASSERT_EQ(boardingStep.stopSequenceInTrip, jsonResponse["steps"][1]["stopSequenceInTrip"]);
    ASSERT_EQ(boardingNode->name, jsonResponse["steps"][1]["nodeName"]);
    ASSERT_EQ(boardingNode->code, jsonResponse["steps"][1]["nodeCode"]);
    ASSERT_EQ(boardingNodeUuid, jsonResponse["steps"][1]["nodeUuid"]);
    ASSERT_EQ(boardingNodePoint.longitude, jsonResponse["steps"][1]["nodeCoordinates"][0]);
    ASSERT_EQ(boardingNodePoint.latitude, jsonResponse["steps"][1]["nodeCoordinates"][1]);
    ASSERT_EQ(boardingStep.departureTime, jsonResponse["steps"][1]["departureTime"]);
    ASSERT_EQ(boardingStep.waitingTime, jsonResponse["steps"][1]["waitingTime"]);

    // Test the unboarding step
    TrRouting::UnboardingStep& unboardingStep = dynamic_cast<TrRouting::UnboardingStep&>(*result.steps[2].get());
    ASSERT_EQ("unboarding", jsonResponse["steps"][2]["action"]);
    ASSERT_EQ(agency->acronym, jsonResponse["steps"][2]["agencyAcronym"]);
    ASSERT_EQ(agency->name, jsonResponse["steps"][2]["agencyName"]);
    ASSERT_EQ(agencyUuid, jsonResponse["steps"][2]["agencyUuid"]);
    ASSERT_EQ(line->shortname, jsonResponse["steps"][2]["lineShortname"]);
    ASSERT_EQ(line->longname, jsonResponse["steps"][2]["lineLongname"]);
    ASSERT_EQ(lineUuid, jsonResponse["steps"][2]["lineUuid"]);
    ASSERT_EQ(pathUuid, jsonResponse["steps"][2]["pathUuid"]);
    ASSERT_EQ(mode->name, jsonResponse["steps"][2]["modeName"]);
    ASSERT_EQ(mode->shortname, jsonResponse["steps"][2]["mode"]);
    ASSERT_EQ(tripUuid, jsonResponse["steps"][2]["tripUuid"]);
    ASSERT_EQ(unboardingStep.legSequenceInTrip, jsonResponse["steps"][2]["legSequenceInTrip"]);
    ASSERT_EQ(unboardingStep.stopSequenceInTrip, jsonResponse["steps"][2]["stopSequenceInTrip"]);
    ASSERT_EQ(unboardingNode->name, jsonResponse["steps"][2]["nodeName"]);
    ASSERT_EQ(unboardingNode->code, jsonResponse["steps"][2]["nodeCode"]);
    ASSERT_EQ(unboardingNodeUuid, jsonResponse["steps"][2]["nodeUuid"]);
    ASSERT_EQ(unboardingNodePoint.longitude, jsonResponse["steps"][2]["nodeCoordinates"][0]);
    ASSERT_EQ(unboardingNodePoint.latitude, jsonResponse["steps"][2]["nodeCoordinates"][1]);
    ASSERT_EQ(unboardingStep.arrivalTime, jsonResponse["steps"][2]["arrivalTime"]);
    ASSERT_EQ(unboardingStep.inVehicleTime, jsonResponse["steps"][2]["inVehicleTime"]);
    ASSERT_EQ(unboardingStep.inVehicleDistanceMeters, jsonResponse["steps"][2]["inVehicleDistance"]);
}
