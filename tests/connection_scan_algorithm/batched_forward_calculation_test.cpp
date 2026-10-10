#include <cstdlib>
#include <random>
#include <unordered_map>

#include "gtest/gtest.h"
#include "csa_test_base.hpp"
#include "batched_forward_calculation.hpp"
#include "compact_connection_set.hpp"
#include "compact_transfers.hpp"
#include "alternative_filter.hpp"
#include "calculator.hpp"
#include "connection_set.hpp"
#include "parameters.hpp"
#include "scenario.hpp"
#include "toolbox.hpp"

// Excluded lines, max total travel time, max inner time of trip buffer, max transfer walking time
using BatchedForwardParameters = std::tuple<std::vector<boost::uuids::uuid>, int, int, int>;

class BatchedForwardCalculationTestSuite : public BaseCsaFixtureTests,
                                           public testing::WithParamInterface<BatchedForwardParameters> {
};

INSTANTIATE_TEST_SUITE_P(
    BatchedForwardValues, BatchedForwardCalculationTestSuite,
    testing::Combine(
        testing::Values(
            std::vector<boost::uuids::uuid>{},
            std::vector<boost::uuids::uuid>{TestDataFetcher::lineSNUuid},
            std::vector<boost::uuids::uuid>{TestDataFetcher::lineEWUuid},
            std::vector<boost::uuids::uuid>{TestDataFetcher::lineExtraUuid},
            std::vector<boost::uuids::uuid>{TestDataFetcher::lineSNUuid, TestDataFetcher::lineEWUuid}
        ),
        testing::Values(TrRouting::MAX_INT, 40 * 60),
        testing::Values(0, 30 * 60),
        testing::Values(0, 20 * 60)
    )
);

// Result of Calculator::reset followed by Calculator::forwardCalculation, the reference of the batch
TrRouting::BatchedForwardResult calculateForward(TrRouting::Calculator &calculator, const TrRouting::RouteParameters &parameters, TrRouting::AlternativeLineFilter &lineFilter)
{
    TrRouting::BatchedForwardResult result;
    try {
        calculator.reset(parameters, *parameters.getOrigin(), *parameters.getDestination(), true, &lineFilter);
        std::unordered_map<TrRouting::Node::uid_t, TrRouting::JourneyStep> forwardEgressJourneysSteps;
        auto forward = calculator.forwardCalculation(parameters, forwardEgressJourneysSteps);
        if (forward.has_value()) {
            result.arrivalTimeSeconds = std::get<0>(*forward);
            result.egressNodeUid = std::get<1>(*forward).get().uid;
        } else {
            result.noRoutingReason = TrRouting::NoRoutingReason::NO_ROUTING_FOUND;
        }
    } catch (const TrRouting::NoRoutingFoundException &e) {
        result.noRoutingReason = e.getReason();
    }
    return result;
}

// Seed of the random points and departure times, different at each run. Set TR_TEST_SEED to replay a failure.
unsigned int getTestSeed()
{
    const char *seed = std::getenv("TR_TEST_SEED");
    return seed != nullptr ? std::stoul(seed) : std::random_device{}();
}

// Run all origin/destination pairs at fixed and random departure times in one batch and compare each query with the single calculation
TEST_P(BatchedForwardCalculationTestSuite, TestSameResultsAsForwardCalculation)
{
    const unsigned int seed = getTestSeed();
    SCOPED_TRACE("TR_TEST_SEED=" + std::to_string(seed));
    std::mt19937 random(seed);
    const auto &[excludedLineUuids, maxTotalTravelTime, maxInnerTimeOfTripBuffer, maxTransferTravelTime] = GetParam();
    const TrRouting::Scenario &scenario = transitData.getScenarios().at(TestDataFetcher::scenarioUuid);
    std::vector<std::reference_wrapper<const TrRouting::Line>> excludedLines;
    std::vector<TrRouting::Line::uid_t> excludedLineUids;
    for (const auto &lineUuid : excludedLineUuids) {
        excludedLines.push_back(transitData.getLines().at(lineUuid));
        excludedLineUids.push_back(transitData.getLines().at(lineUuid).uid);
    }
    TrRouting::AlternativeLineFilter lineFilter(excludedLines);

    // All nodes, random points around them and a point far from the network to cover the no access reasons
    std::vector<TrRouting::Point> points{TrRouting::Point(45.5349, -73.55478)};
    for (const auto &[uuid, node] : transitData.getNodes()) {
        points.push_back(*node.point);
    }
    std::uniform_real_distribution<double> offset(-0.005, 0.005); // About 500 m
    for (int i = 0; i < 5; i++) {
        const TrRouting::Point &node = points[1 + random() % (points.size() - 1)];
        points.push_back(TrRouting::Point(node.latitude + offset(random), node.longitude + offset(random)));
    }
    auto makeParameters = [&](const TrRouting::Point &origin, const TrRouting::Point &destination, int departureTime) {
        return TrRouting::RouteParameters(std::make_unique<TrRouting::Point>(origin), std::make_unique<TrRouting::Point>(destination),
            scenario, departureTime, MIN_WAITING_TIME, maxTotalTravelTime, 20 * 60, 20 * 60, maxTransferTravelTime, maxInnerTimeOfTripBuffer, false, true);
    };

    TrRouting::Calculator calculator(transitData, geoFilter);
    std::vector<TrRouting::BatchedForwardQuery> queries;
    std::vector<TrRouting::BatchedForwardResult> expected;
    // Fixed times, which have arrivals for every test parameter, and random times
    std::vector<int> departureTimes{7 * 3600, 9 * 3600, 9 * 3600 + 50 * 60, 10 * 3600 + 30 * 60, 11 * 3600};
    std::uniform_int_distribution<int> randomDepartureTime(8 * 3600, 11 * 3600 + 30 * 60); // Around the trips of the test data
    for (int i = 0; i < 5; i++) {
        departureTimes.push_back(randomDepartureTime(random));
    }
    for (int departureTime : departureTimes) {
        for (const TrRouting::Point &origin : points) {
            for (const TrRouting::Point &destination : points) {
                TrRouting::RouteParameters parameters = makeParameters(origin, destination, departureTime);
                expected.push_back(calculateForward(calculator, parameters, lineFilter));
                queries.push_back({
                    departureTime,
                    geoFilter.getAccessibleNodesFootpathsFromPoint(origin, transitData.getNodes(), parameters.getMaxAccessWalkingTravelTimeSeconds(), parameters.getWalkingSpeedMetersPerSecond()),
                    geoFilter.getAccessibleNodesFootpathsFromPoint(destination, transitData.getNodes(), parameters.getMaxEgressWalkingTravelTimeSeconds(), parameters.getWalkingSpeedMetersPerSecond()),
                    excludedLineUids
                });
            }
        }
    }

    TrRouting::CompactConnectionSet connections(*transitData.getConnectionsForScenario(scenario));
    TrRouting::CompactTransfers transfers(transitData.getNodes());
    const auto results = TrRouting::batchedForwardCalculation(connections, transfers, makeParameters(points[0], points[0], 0), queries);

    ASSERT_EQ(expected.size(), results.size());
    int arrivalsCount = 0;
    for (size_t i = 0; i < results.size(); i++) {
        SCOPED_TRACE("query " + std::to_string(i));
        EXPECT_EQ(expected[i].noRoutingReason, results[i].noRoutingReason);
        EXPECT_EQ(expected[i].arrivalTimeSeconds, results[i].arrivalTimeSeconds);
        EXPECT_EQ(expected[i].egressNodeUid, results[i].egressNodeUid);
        arrivalsCount += expected[i].noRoutingReason.has_value() ? 0 : 1;
    }
    // Make sure the comparison is not only on queries without routing
    EXPECT_GT(arrivalsCount, 0);
}
