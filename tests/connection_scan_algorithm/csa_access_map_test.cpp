#include "gtest/gtest.h"
#include "spdlog/spdlog.h"

#include "calculator.hpp"
#include "parameters.hpp"
#include "csa_test_base.hpp"
#include "scenario.hpp"
#include "toolbox.hpp" //MAX_INT

// This fixture tests the calculations for the accessibility map from/to a point
class AccessMapFixtureTests : public BaseCsaFixtureTests
{
protected:
    // Default value for various test parameters
    static const int DEFAULT_MIN_WAITING_TIME = 3 * 60;
    static const int DEFAULT_MAX_TOTAL_TIME = TrRouting::MAX_INT;
    static const int DEFAULT_MAX_ACCESS_TRAVEL_TIME = 20 * 60;
    static const int DEFAULT_MAX_EGRESS_TRAVEL_TIME = 20 * 60;
    static const int DEFAULT_MAX_TRANSFER_TRAVEL_TIME = 20 * 60;
    static const int DEFAULT_FIRST_WAITING_TIME = 30 * 60;

public:
    // Helper method to set parameters and calculate OD trip result. Test cases need only provide parameters and validate the result
    std::unique_ptr<TrRouting::AllNodesResult> calculateOd(TrRouting::AccessibilityParameters& parameters);
    void SetUp();
};

void AccessMapFixtureTests::SetUp()
{
    BaseCsaFixtureTests::SetUp();
}

std::unique_ptr<TrRouting::AllNodesResult> AccessMapFixtureTests::calculateOd(TrRouting::AccessibilityParameters& parameters)
{
    // TODO Shouldn't need to do this, but we do for now, benchmark needs to be started
    calculator.algorithmCalculationTime.start();
    calculator.benchmarking.clear();
    return calculator.calculateAllNodes(parameters);
}

// Test from an place far from the network (origin offsetted)
TEST_F(AccessMapFixtureTests, AllNodesQueryNoNodeAtPlace)
{
    int departureTime = getTimeInSeconds(9, 45);
    int totalTravelTime = 1800;

    TrRouting::AccessibilityParameters testParameters = TrRouting::AccessibilityParameters(
        std::make_unique<TrRouting::Point>(44.5242,-73.5817),
        transitData.getScenarios().at(TestDataFetcher::scenarioUuid),
        departureTime,
        DEFAULT_MIN_WAITING_TIME,
        totalTravelTime,
        DEFAULT_MAX_ACCESS_TRAVEL_TIME,
        DEFAULT_MAX_EGRESS_TRAVEL_TIME,
        DEFAULT_MAX_TRANSFER_TRAVEL_TIME,
        DEFAULT_FIRST_WAITING_TIME,
        true
    );

    try {
        calculateOd(testParameters);
        FAIL() << "Expected TrRouting::NoRoutingFoundException, no exception thrown";
    } catch (TrRouting::NoRoutingFoundException const & e) {
        assertNoRouting(e, TrRouting::NoRoutingReason::NO_ACCESS_AT_ORIGIN);
    } catch(...) {
        FAIL() << "Expected TrRouting::NoRoutingFoundException, another type was thrown";
    }

}

// Test at a time where there is no service
TEST_F(AccessMapFixtureTests, AllNodesQueryNoServiceAtPlace)
{
    int departureTime = getTimeInSeconds(15, 45);
    int totalTravelTime = 1800;

    TrRouting::AccessibilityParameters testParameters = TrRouting::AccessibilityParameters(
        std::make_unique<TrRouting::Point>(45.54, -73.6146),
        transitData.getScenarios().at(TestDataFetcher::scenarioUuid),
        departureTime,
        DEFAULT_MIN_WAITING_TIME,
        totalTravelTime,
        DEFAULT_MAX_ACCESS_TRAVEL_TIME,
        DEFAULT_MAX_EGRESS_TRAVEL_TIME,
        DEFAULT_MAX_TRANSFER_TRAVEL_TIME,
        DEFAULT_FIRST_WAITING_TIME,
        false
    );

    try {
        calculateOd(testParameters);
        FAIL() << "Expected TrRouting::NoRoutingFoundException, no exception thrown";
    } catch (TrRouting::NoRoutingFoundException const & e) {
        assertNoRouting(e, TrRouting::NoRoutingReason::NO_SERVICE_TO_DESTINATION);
    } catch(...) {
        FAIL() << "Expected TrRouting::NoRoutingFoundException, another type was thrown";
    }

}

// Test forward calculation from a place near the south2 node
TEST_F(AccessMapFixtureTests, SimpleAllNodesQuery)
{
    int departureTime = getTimeInSeconds(9, 45);
    int totalTravelTime = 45 * 60;

    TrRouting::AccessibilityParameters testParameters = TrRouting::AccessibilityParameters(
        std::make_unique<TrRouting::Point>(45.5242,-73.5817),
         transitData.getScenarios().at(TestDataFetcher::scenarioUuid),
        departureTime,
        DEFAULT_MIN_WAITING_TIME,
        totalTravelTime,
        DEFAULT_MAX_ACCESS_TRAVEL_TIME,
        DEFAULT_MAX_EGRESS_TRAVEL_TIME,
        DEFAULT_MAX_TRANSFER_TRAVEL_TIME,
        DEFAULT_FIRST_WAITING_TIME,
        true
    );

    // FIXME: It should transfer to the extra line, 558 seconds walk to node, then transit. Why not?
    std::unique_ptr<TrRouting::AllNodesResult> result = calculateOd(testParameters);
    ASSERT_EQ(5, result->numberOfReachableNodes);
    ASSERT_EQ(5u, result->nodes.size());
    ASSERT_EQ(10, result->totalNodeCount);

    // Validate each node
    for(auto accessibleNode: result->nodes)
    {
        spdlog::info("accessible node {} ...", accessibleNode.node.name);
        if (boost::uuids::to_string(accessibleNode.node.uuid) == boost::uuids::to_string(TestDataFetcher::nodeSouth1Uuid)) {
            int arrivalTime = getTimeInSeconds(10, 3, 30);
            ASSERT_EQ(arrivalTime, accessibleNode.arrivalTime);
            ASSERT_EQ(arrivalTime - departureTime, accessibleNode.totalTravelTime);
            ASSERT_EQ(0, accessibleNode.numberOfTransfers);
        } else if (boost::uuids::to_string(accessibleNode.node.uuid) == boost::uuids::to_string(TestDataFetcher::nodeMidNodeUuid)) {
            int arrivalTime = getTimeInSeconds(10, 7);
            ASSERT_EQ(arrivalTime, accessibleNode.arrivalTime);
            ASSERT_EQ(arrivalTime - departureTime, accessibleNode.totalTravelTime);
            ASSERT_EQ(0, accessibleNode.numberOfTransfers);
        } else if (boost::uuids::to_string(accessibleNode.node.uuid) == boost::uuids::to_string(TestDataFetcher::nodeNorth1Uuid)) {
            int arrivalTime = getTimeInSeconds(10, 13);
            ASSERT_EQ(arrivalTime, accessibleNode.arrivalTime);
            ASSERT_EQ(arrivalTime - departureTime, accessibleNode.totalTravelTime);
            ASSERT_EQ(0, accessibleNode.numberOfTransfers);
        } else if (boost::uuids::to_string(accessibleNode.node.uuid) == boost::uuids::to_string(TestDataFetcher::nodeNorth2Uuid)) {
            int arrivalTime = getTimeInSeconds(10, 18);
            ASSERT_EQ(arrivalTime, accessibleNode.arrivalTime);
            ASSERT_EQ(arrivalTime - departureTime, accessibleNode.totalTravelTime);
            ASSERT_EQ(0, accessibleNode.numberOfTransfers);
        } else if (boost::uuids::to_string(accessibleNode.node.uuid) == boost::uuids::to_string(TestDataFetcher::nodeExtra1Uuid)) {
            int arrivalTime = getTimeInSeconds(10, 25);
            ASSERT_EQ(arrivalTime, accessibleNode.arrivalTime);
            ASSERT_EQ(arrivalTime - departureTime, accessibleNode.totalTravelTime);
            ASSERT_EQ(1, accessibleNode.numberOfTransfers);
        } else {
            FAIL() << "Unexpected node: " << accessibleNode.node.name;
        }
    }   

}

/*
FIXME: This test does not completely pass. There is a 1 second offset of travel time that needs to be investigated
FIXME2: See issue #230. Time should include the waiting time
// Test backward calculation from a place near the north2 node
TEST_F(AccessMapFixtureTests, SimpleAllNodesQueryBackward)
{
    int arrivalTime = getTimeInSeconds(10, 30);
    int totalTravelTime = 45 * 60;

    TrRouting::AccessibilityParameters testParameters = TrRouting::AccessibilityParameters(
        std::make_unique<TrRouting::Point>(45.5485, -73.6416),
         transitData.getScenarios().at(TestDataFetcher::scenarioUuid),
        arrivalTime,
        DEFAULT_MIN_WAITING_TIME,
        totalTravelTime,
        DEFAULT_MAX_ACCESS_TRAVEL_TIME,
        DEFAULT_MAX_EGRESS_TRAVEL_TIME,
        DEFAULT_MAX_TRANSFER_TRAVEL_TIME,
        DEFAULT_FIRST_WAITING_TIME,
        false
    );

    std::unique_ptr<TrRouting::AllNodesResult> result = calculateOd(testParameters);
    ASSERT_EQ(6, result->numberOfReachableNodes);
    ASSERT_EQ(6u, result->nodes.size());
    ASSERT_EQ(10, result->totalNodeCount);

    // Validate each node
    for(auto accessibleNode: result->nodes)
    {
        spdlog::info("accessible node {} ...", accessibleNode.node.name);
        if (boost::uuids::to_string(accessibleNode.node.uuid) == boost::uuids::to_string(TestDataFetcher::nodeSouth2Uuid)) {
            int departureTime = getTimeInSeconds(10, 0);
            ASSERT_EQ(arrivalTime, accessibleNode.arrivalTime);
            ASSERT_EQ(arrivalTime - departureTime, accessibleNode.totalTravelTime);
            ASSERT_EQ(0, accessibleNode.numberOfTransfers);
        } else if (boost::uuids::to_string(accessibleNode.node.uuid) == boost::uuids::to_string(TestDataFetcher::nodeSouth1Uuid)) {
            int departureTime = getTimeInSeconds(10, 3, 50);
            ASSERT_EQ(arrivalTime, accessibleNode.arrivalTime);
            ASSERT_EQ(arrivalTime - departureTime, accessibleNode.totalTravelTime);
            ASSERT_EQ(0, accessibleNode.numberOfTransfers);
        } else if (boost::uuids::to_string(accessibleNode.node.uuid) == boost::uuids::to_string(TestDataFetcher::nodeMidNodeUuid)) {
            int departureTime = getTimeInSeconds(10, 10);
            ASSERT_EQ(arrivalTime, accessibleNode.arrivalTime);
            ASSERT_EQ(arrivalTime - departureTime, accessibleNode.totalTravelTime);
            ASSERT_EQ(0, accessibleNode.numberOfTransfers);
        } else if (boost::uuids::to_string(accessibleNode.node.uuid) == boost::uuids::to_string(TestDataFetcher::nodeNorth1Uuid)) {
            int departureTime = getTimeInSeconds(10, 13, 30);
            ASSERT_EQ(arrivalTime, accessibleNode.arrivalTime);
            ASSERT_EQ(arrivalTime - departureTime, accessibleNode.totalTravelTime);
            ASSERT_EQ(0, accessibleNode.numberOfTransfers);
        } else if (boost::uuids::to_string(accessibleNode.node.uuid) == boost::uuids::to_string(TestDataFetcher::nodeEast1Uuid)) {
            int departureTime = getTimeInSeconds(10, 5, 10);
            ASSERT_EQ(arrivalTime, accessibleNode.arrivalTime);
            ASSERT_EQ(arrivalTime - departureTime, accessibleNode.totalTravelTime);
            ASSERT_EQ(1, accessibleNode.numberOfTransfers);
        } else if (boost::uuids::to_string(accessibleNode.node.uuid) == boost::uuids::to_string(TestDataFetcher::nodeEast2Uuid)) {
            int departureTime = getTimeInSeconds(10, 2);
            ASSERT_EQ(arrivalTime, accessibleNode.arrivalTime);
            ASSERT_EQ(arrivalTime - departureTime, accessibleNode.totalTravelTime);
            ASSERT_EQ(1, accessibleNode.numberOfTransfers);
        } else {
            FAIL() << "Unexpected node: " << accessibleNode.node.name;
        }
    }   

}*/
