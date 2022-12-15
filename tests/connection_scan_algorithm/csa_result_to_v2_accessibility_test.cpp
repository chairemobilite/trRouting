#include <errno.h>

#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "csa_result_to_response_test.hpp"
#include "routing_result.hpp"
#include "json.hpp"
#include "constants.hpp"
#include "node.hpp"
#include "result_to_v2_accessibility.hpp"

class ResultToV2AccessFixtureTest : public ResultToResponseFixtureTest
{
protected:
    std::unique_ptr<TrRouting::AccessibilityParameters> accessParameters;

    void assertQueryConversion(nlohmann::json jsonResponse, bool isForward = true);

public:
    void SetUp( ) override
    {
        ResultToResponseFixtureTest::SetUp();

        // Create the accessibility parameters
        accessParameters = std::make_unique<TrRouting::AccessibilityParameters>(
            std::make_unique<TrRouting::Point>(45.5269, -73.58912),
            *scenario,
            DEFAULT_TIME,
            DEFAULT_MIN_WAITING_TIME,
            DEFAULT_MAX_TOTAL_TIME,
            DEFAULT_MAX_ACCESS_TRAVEL_TIME,
            DEFAULT_MAX_EGRESS_TRAVEL_TIME,
            DEFAULT_MAX_TRANSFER_TRAVEL_TIME,
            DEFAULT_FIRST_WAITING_TIME,
            true
        );
    }
};

TEST_F(ResultToV2AccessFixtureTest, TestNoRoutingFoundResultDefaultV2Access)
{
    nlohmann::json jsonResponse = TrRouting::ResultToV2AccessibilityResponse::noRoutingFoundResponse(*accessParameters.get(), TrRouting::NoRoutingReason::NO_ROUTING_FOUND);

    ASSERT_EQ(STATUS_NO_ROUTING_FOUND, jsonResponse["status"]);
    assertQueryConversion(jsonResponse);
    ASSERT_EQ("NO_ROUTING_FOUND", jsonResponse["reason"]);
}

TEST_F(ResultToV2AccessFixtureTest, TestNoRoutingFoundResultWithReasonV2Access)
{
    nlohmann::json jsonResponse = TrRouting::ResultToV2AccessibilityResponse::noRoutingFoundResponse(*accessParameters.get(), TrRouting::NoRoutingReason::NO_ACCESS_AT_ORIGIN);

    ASSERT_EQ(STATUS_NO_ROUTING_FOUND, jsonResponse["status"]);
    assertQueryConversion(jsonResponse);
    ASSERT_EQ("NO_ACCESS_AT_PLACE", jsonResponse["reason"]);
}

TEST_F(ResultToV2AccessFixtureTest, TestAllNodesForwardV2Access)
{
    // Prepare a result object
    int node1TravelTime = 900;
    int node1Transfers = 1;
    int node2TravelTime = 1500;
    int node2Transfers = 0;
    TrRouting::AllNodesResult result = TrRouting::AllNodesResult();
    result.totalNodeCount = 50;
    result.numberOfReachableNodes = 2;

    result.nodes.push_back(TrRouting::AccessibleNodes(*boardingNode, 
        DEFAULT_TIME + node1TravelTime,
        node1TravelTime,
        node1Transfers)
    );
    result.nodes.push_back(TrRouting::AccessibleNodes(*unboardingNode, 
        DEFAULT_TIME + node2TravelTime,
        node2TravelTime,
        node2Transfers)
    );

    // Validate response
    nlohmann::json jsonResponse = TrRouting::ResultToV2AccessibilityResponse::resultToJsonString(result, *accessParameters.get());

    ASSERT_EQ(STATUS_SUCCESS, jsonResponse["status"]);
    assertQueryConversion(jsonResponse);

    // Validate result
    ASSERT_EQ(result.totalNodeCount, jsonResponse["result"]["totalNodeCount"]);
    ASSERT_EQ(2u, jsonResponse["result"]["nodes"].size());

    // Validate first node
    ASSERT_EQ(boardingNode->name, jsonResponse["result"]["nodes"][0]["nodeName"]);
    ASSERT_EQ(boardingNode->code, jsonResponse["result"]["nodes"][0]["nodeCode"]);
    ASSERT_EQ(boardingNodeUuid, jsonResponse["result"]["nodes"][0]["nodeUuid"]);
    ASSERT_EQ(boardingNode->point.get()->longitude, jsonResponse["result"]["nodes"][0]["nodeCoordinates"][0]);
    ASSERT_EQ(boardingNode->point.get()->latitude, jsonResponse["result"]["nodes"][0]["nodeCoordinates"][1]);
    ASSERT_EQ(DEFAULT_TIME + node1TravelTime, jsonResponse["result"]["nodes"][0]["nodeTime"]);
    ASSERT_EQ(node1TravelTime, jsonResponse["result"]["nodes"][0]["totalTravelTime"]);
    ASSERT_EQ(node1Transfers, jsonResponse["result"]["nodes"][0]["numberOfTransfers"]);

    // Validate second node
    ASSERT_EQ(unboardingNode->name, jsonResponse["result"]["nodes"][1]["nodeName"]);
    ASSERT_EQ(unboardingNode->code, jsonResponse["result"]["nodes"][1]["nodeCode"]);
    ASSERT_EQ(unboardingNodeUuid, jsonResponse["result"]["nodes"][1]["nodeUuid"]);
    ASSERT_EQ(unboardingNode->point.get()->longitude, jsonResponse["result"]["nodes"][1]["nodeCoordinates"][0]);
    ASSERT_EQ(unboardingNode->point.get()->latitude, jsonResponse["result"]["nodes"][1]["nodeCoordinates"][1]);
    ASSERT_EQ(DEFAULT_TIME + node2TravelTime, jsonResponse["result"]["nodes"][1]["nodeTime"]);
    ASSERT_EQ(node2TravelTime, jsonResponse["result"]["nodes"][1]["totalTravelTime"]);
    ASSERT_EQ(node2Transfers, jsonResponse["result"]["nodes"][1]["numberOfTransfers"]);

}

TEST_F(ResultToV2AccessFixtureTest, TestAllNodesBackwardV2Access)
{
    // Create a backward accessibility parameters
    TrRouting::AccessibilityParameters backwardParams = TrRouting::AccessibilityParameters(
        std::make_unique<TrRouting::Point>(45.5269, -73.58912),
        *scenario,
        DEFAULT_TIME,
        DEFAULT_MIN_WAITING_TIME,
        DEFAULT_MAX_TOTAL_TIME,
        DEFAULT_MAX_ACCESS_TRAVEL_TIME,
        DEFAULT_MAX_EGRESS_TRAVEL_TIME,
        DEFAULT_MAX_TRANSFER_TRAVEL_TIME,
        DEFAULT_FIRST_WAITING_TIME,
        false
    );

    // Prepare a result object
    int node1TravelTime = 900;
    int node1Transfers = 1;
    int node2TravelTime = 1500;
    int node2Transfers = 0;
    TrRouting::AllNodesResult result = TrRouting::AllNodesResult();
    result.totalNodeCount = 50;
    result.numberOfReachableNodes = 2;

    // Backward nodes all have the time as arrival time and the travel time is calculated to this hour, even if it arrives earlier
    result.nodes.push_back(TrRouting::AccessibleNodes(*boardingNode, 
        DEFAULT_TIME,
        node1TravelTime,
        node1Transfers)
    );
    result.nodes.push_back(TrRouting::AccessibleNodes(*unboardingNode, 
        DEFAULT_TIME,
        node2TravelTime,
        node2Transfers)
    );

    // Validate response
    nlohmann::json jsonResponse = TrRouting::ResultToV2AccessibilityResponse::resultToJsonString(result, backwardParams);

    ASSERT_EQ(STATUS_SUCCESS, jsonResponse["status"]);
    assertQueryConversion(jsonResponse, false);

    // Validate result
    ASSERT_EQ(result.totalNodeCount, jsonResponse["result"]["totalNodeCount"]);
    ASSERT_EQ(2u, jsonResponse["result"]["nodes"].size());

    // Validate first node
    ASSERT_EQ(boardingNode->name, jsonResponse["result"]["nodes"][0]["nodeName"]);
    ASSERT_EQ(boardingNode->code, jsonResponse["result"]["nodes"][0]["nodeCode"]);
    ASSERT_EQ(boardingNodeUuid, jsonResponse["result"]["nodes"][0]["nodeUuid"]);
    ASSERT_EQ(boardingNode->point.get()->longitude, jsonResponse["result"]["nodes"][0]["nodeCoordinates"][0]);
    ASSERT_EQ(boardingNode->point.get()->latitude, jsonResponse["result"]["nodes"][0]["nodeCoordinates"][1]);
    ASSERT_EQ(DEFAULT_TIME - node1TravelTime, jsonResponse["result"]["nodes"][0]["nodeTime"]);
    ASSERT_EQ(node1TravelTime, jsonResponse["result"]["nodes"][0]["totalTravelTime"]);
    ASSERT_EQ(node1Transfers, jsonResponse["result"]["nodes"][0]["numberOfTransfers"]);

    // Validate second node
    ASSERT_EQ(unboardingNode->name, jsonResponse["result"]["nodes"][1]["nodeName"]);
    ASSERT_EQ(unboardingNode->code, jsonResponse["result"]["nodes"][1]["nodeCode"]);
    ASSERT_EQ(unboardingNodeUuid, jsonResponse["result"]["nodes"][1]["nodeUuid"]);
    ASSERT_EQ(unboardingNode->point.get()->longitude, jsonResponse["result"]["nodes"][1]["nodeCoordinates"][0]);
    ASSERT_EQ(unboardingNode->point.get()->latitude, jsonResponse["result"]["nodes"][1]["nodeCoordinates"][1]);
    ASSERT_EQ(DEFAULT_TIME - node2TravelTime, jsonResponse["result"]["nodes"][1]["nodeTime"]);
    ASSERT_EQ(node2TravelTime, jsonResponse["result"]["nodes"][1]["totalTravelTime"]);
    ASSERT_EQ(node2Transfers, jsonResponse["result"]["nodes"][1]["numberOfTransfers"]);

}

void ResultToV2AccessFixtureTest::assertQueryConversion(nlohmann::json jsonResponse, bool isForward) {
    TrRouting::Point* place = accessParameters.get()->getPlace();

    ASSERT_EQ(place->latitude, jsonResponse["query"]["place"][1]);
    ASSERT_EQ(place->longitude, jsonResponse["query"]["place"][0]);
    ASSERT_EQ(accessParameters.get()->getTimeOfTrip(), jsonResponse["query"]["timeOfTrip"]);
    ASSERT_EQ(isForward ? 0 : 1, jsonResponse["query"]["timeType"]);
}
