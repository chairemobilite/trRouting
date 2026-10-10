#include "gtest/gtest.h"
#include "connection_set_test.hpp"
#include "compact_connection_set.hpp"
#include "connection_set.hpp"
#include "connection.hpp"
#include "node.hpp"
#include "scenario.hpp"
#include "line.hpp"
#include "trip.hpp"

class CompactConnectionSetTestSuite : public ConnectionSetFixtureTests,
                                      public testing::WithParamInterface<boost::uuids::uuid> {
};

INSTANTIATE_TEST_SUITE_P(
    ScenarioValues, CompactConnectionSetTestSuite,
    testing::Values(TestDataFetcher::scenarioUuid, TestDataFetcher::scenario2Uuid)
);

// Each compact entry must match the forward connection at the same index
TEST_P(CompactConnectionSetTestSuite, TestMatchesForwardConnections)
{
    auto connectionSet = transitData.getConnectionsForScenario(transitData.getScenarios().at(GetParam()));
    TrRouting::CompactConnectionSet compact(*connectionSet);

    const auto &connections = connectionSet->getForwardConnections();
    const auto &trips = connectionSet->getTrips();
    ASSERT_FALSE(connections.empty());
    ASSERT_EQ(connections.size(), compact.departureTime.size());
    ASSERT_EQ(trips.size(), compact.tripLineUid.size());
    for (size_t i = 0; i < connections.size(); i++) {
        const TrRouting::Connection &connection = connections[i];
        SCOPED_TRACE("connection " + std::to_string(i));
        EXPECT_EQ(connection.getDepartureNode().uid, compact.departureNodeUid[i]);
        EXPECT_EQ(connection.getArrivalNode().uid, compact.arrivalNodeUid[i]);
        EXPECT_EQ(connection.getDepartureTime(), compact.departureTime[i]);
        EXPECT_EQ(connection.getArrivalTime(), compact.arrivalTime[i]);
        EXPECT_EQ(connection.getMinWaitingTime(), compact.minWaitingTime[i]);
        EXPECT_EQ(connection.canBoard(), (compact.flags[i] & TrRouting::CompactConnectionSet::CAN_BOARD) != 0);
        EXPECT_EQ(connection.canUnboard(), (compact.flags[i] & TrRouting::CompactConnectionSet::CAN_UNBOARD) != 0);
        const int32_t tripIndex = compact.tripIndex[i];
        ASSERT_GE(tripIndex, 0);
        ASSERT_LT(tripIndex, (int32_t)trips.size());
        EXPECT_EQ(connection.getTrip().uid, trips[tripIndex].get().uid);
        EXPECT_EQ(connection.getTrip().line.uid, compact.tripLineUid[tripIndex]);
    }
}
