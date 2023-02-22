#include <errno.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "gtest/gtest.h"
#include "calculator.hpp"
#include "connection_set_test.hpp"
#include "connection_set.hpp"
#include "connection_cache.hpp"
#include "spdlog/spdlog.h"
#include "scenario.hpp"

// Test that the TransitData gets and create the connection cache correctly for various scenarios
TEST_F(ConnectionSetFixtureTests, TestCacheAssignation)
{
    const TrRouting::Scenario & scenario = transitData.getScenarios().at(TestDataFetcher::scenarioUuid);
    const TrRouting::Scenario & scenario2 = transitData.getScenarios().at(TestDataFetcher::scenario2Uuid);

    // Get the cache a first time for a given scenario
    std::shared_ptr<TrRouting::ConnectionSet> cache = transitData.getConnectionsForScenario(scenario);

    ASSERT_EQ(17, cache->getForwardConnections().size());
    ASSERT_EQ(17, cache->getReverseConnections().size());

    // Get the cache a second time for the same scenario
    cache = transitData.getConnectionsForScenario(scenario);

    ASSERT_EQ(17, cache->getForwardConnections().size());
    ASSERT_EQ(17, cache->getReverseConnections().size());

    // Get the cache for a second scenario
    cache = transitData.getConnectionsForScenario(scenario2);

    ASSERT_EQ(9, cache->getForwardConnections().size());
    ASSERT_EQ(9, cache->getReverseConnections().size());

    // Get the cache for a first scenario again
    cache = transitData.getConnectionsForScenario(scenario);

    ASSERT_EQ(17, cache->getForwardConnections().size());
    ASSERT_EQ(17, cache->getReverseConnections().size());

}
