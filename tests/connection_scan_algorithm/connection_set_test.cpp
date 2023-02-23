#include <errno.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "gtest/gtest.h"
#include "calculator.hpp"
#include "connection_set_test.hpp"
#include "connection_set.hpp"
#include "spdlog/spdlog.h"
#include "scenario.hpp"


class ForwardConnectionIteratorTestSuite : public ConnectionSetFixtureTests,
                              public testing::WithParamInterface<std::tuple<int, int>> {
};

class ReverseConnectionIteratorTestSuite : public ConnectionSetFixtureTests,
                              public testing::WithParamInterface<std::tuple<int, int>> {
};

// Create a connection set object with all connections
std::shared_ptr<ConnectionSet> getConnectionSet(TransitData transitData) {
    std::vector<std::reference_wrapper<const Connection>> forwardConnections;
    std::vector<std::reference_wrapper<const Connection>> reverseConnections;

    auto forwardLastConnection = getForwardConnections().end(); // cache last connection for loop
    for(auto connection = getForwardConnections().begin(); connection != forwardLastConnection; ++connection)
    {
        forwardConnections.push_back((**connection));
    }

    auto reverseLastConnection = getReverseConnections().end(); // cache last connection for loop
    for(auto connection = getReverseConnections().begin(); connection != reverseLastConnection; ++connection)
    {
        reverseConnections.push_back((**connection));
    }

    return std::make_shared<ConnectionSet>(trips, forwardConnections, reverseConnections);
}

INSTANTIATE_TEST_SUITE_P(
    ForwardConnectionIteratorValues, ForwardConnectionIteratorTestSuite,
    testing::Values(
        // First trip, should include all connections
        std::make_tuple(9, 17),
        // Before all connections, should include all
        std::make_tuple(0, 17),
        // Should not include 9am trip
        std::make_tuple(10, 13),
        // Should only include the 11am trip
        std::make_tuple(11, 4),
        // The following should not include any connections
        std::make_tuple(15, 0),
        std::make_tuple(31, 0),
        // out of range, but still no connection
        std::make_tuple(100, 0)
    )
);

// Test the forward connection iterator by hour
TEST_P(ForwardConnectionIteratorTestSuite, TestForwardIterator)
{
    std::tuple<int, int> param = GetParam();
    int startHour = std::get<0>(param);
    int expected = std::get<1>(param);

    std::shared_ptr<TrRouting::ConnectionSet> cache = getConnectionSet();

    // cache last connection
    auto lastConnection = cache.get()->getForwardConnections().end(); // cache last connection for loop

    int connectionCount = 0;
    for(auto connection = cache.get()->getForwardConnectionsBeginAtDepartureHour(startHour); connection != lastConnection; ++connection)
    {
        connectionCount++;
    }
    ASSERT_EQ(expected, connectionCount);
}

INSTANTIATE_TEST_SUITE_P(
    ReverseConnectionIteratorValues, ReverseConnectionIteratorTestSuite,
    testing::Values(
        // After all trips, should include all
        std::make_tuple(12, 17),
        // Before all connections, should include all
        std::make_tuple(15, 17),
        // Test from 11, which should not include the 11 am trip
        std::make_tuple(11, 13),
        // Test from 10, which should include the first east west trip
        std::make_tuple(10, 4),
        // Test from 8, which should not include any connection
        std::make_tuple(8, 0),
        std::make_tuple(0, 0),
        // out of range, but should include all connections
        std::make_tuple(100, 17)
    )
);

// Test the forward connection iterator by hour
TEST_P(ReverseConnectionIteratorTestSuite, TestReverseIterator)
{
    std::tuple<int, int> param = GetParam();
    int startHour = std::get<0>(param);
    int expected = std::get<1>(param);

    std::shared_ptr<TrRouting::ConnectionSet> cache = getConnectionSet();

    // cache last connection
    auto lastConnection = cache.get()->getReverseConnections().end(); // cache last connection for loop

    int connectionCount = 0;
    for(auto connection = cache.get()->getReverseConnectionsBeginAtArrivalHour(startHour); connection != lastConnection; ++connection)
    {
        connectionCount++;
    }
    ASSERT_EQ(expected, connectionCount);
}


