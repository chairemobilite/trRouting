#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <unordered_map>
#include <boost/uuid/string_generator.hpp>

#include "gtest/gtest.h"
#include "batched_forward_calculation.hpp"
#include "cache_fetcher.hpp"
#include "calculator.hpp"
#include "compact_connection_set.hpp"
#include "compact_transfers.hpp"
#include "connection_set.hpp"
#include "euclideangeofilter.hpp"
#include "parameters.hpp"
#include "scenario.hpp"
#include "transit_data.hpp"

/**
 * Compare the batched forward calculation with Calculator::reset followed by
 * Calculator::forwardCalculation on random queries, different at each run,
 * and time both. Uses the same cache as benchmark_CSA_test.cpp by default.
 * Environment variables:
 * - TR_BENCHMARK_CACHE and TR_BENCHMARK_SCENARIO: other cache directory and scenario uuid
 * - TR_TEST_SEED: replay the random queries of a previous run
 * */
namespace {

  using Clock = std::chrono::steady_clock;

  const int QUERIES_COUNT = 2000;

  double elapsedMs(Clock::time_point start) {
    return std::chrono::duration<double, std::milli>(Clock::now() - start).count();
  }

  std::string getEnv(const char *name, const std::string &defaultValue) {
    const char *value = std::getenv(name);
    return value != nullptr ? value : defaultValue;
  }

  // Data shared by all the batch sizes: the queries and their single calculation results
  struct BenchmarkData {
    std::unique_ptr<TrRouting::TransitData> transitData;
    TrRouting::EuclideanGeoFilter geoFilter;
    std::unique_ptr<TrRouting::RouteParameters> sharedParameters;
    std::unique_ptr<TrRouting::CompactConnectionSet> connections;
    std::unique_ptr<TrRouting::CompactTransfers> transfers;
    std::vector<TrRouting::BatchedForwardQuery> queries;
    std::vector<TrRouting::BatchedForwardResult> expected;
  };
  std::unique_ptr<BenchmarkData> data;

}

class BenchmarkBatchedForwardTests : public testing::TestWithParam<size_t>
{
public:
  static void SetUpTestSuite()
  {
    data = std::make_unique<BenchmarkData>();
    TrRouting::CacheFetcher cacheFetcher(getEnv("TR_BENCHMARK_CACHE", "cache/demo_transition"));
    data->transitData = std::make_unique<TrRouting::TransitData>(cacheFetcher);
    ASSERT_EQ(TrRouting::DataStatus::READY, data->transitData->getDataStatus());
    const TrRouting::TransitData &transitData = *data->transitData;
    const TrRouting::Scenario &scenario = transitData.getScenarios().at(
      boost::uuids::string_generator()(getEnv("TR_BENCHMARK_SCENARIO", "ed42d920-0349-4f64-8590-4698056c2734")));
    auto makeParameters = [&](const TrRouting::Point &origin, const TrRouting::Point &destination, int departureTime) {
      return std::make_unique<TrRouting::RouteParameters>(std::make_unique<TrRouting::Point>(origin), std::make_unique<TrRouting::Point>(destination),
        scenario, departureTime, 3 * 60, 180 * 60, 20 * 60, 20 * 60, 20 * 60, 15 * 60, false, true);
    };

    auto start = Clock::now();
    data->connections = std::make_unique<TrRouting::CompactConnectionSet>(*transitData.getConnectionsForScenario(scenario));
    data->transfers = std::make_unique<TrRouting::CompactTransfers>(transitData.getNodes());
    std::cout << "Compact connections and transfers: " << elapsedMs(start) << " ms" << std::endl;

    // Random points around random nodes, at random departure times during the day
    const char *seedValue = std::getenv("TR_TEST_SEED");
    const unsigned int seed = seedValue != nullptr ? std::stoul(seedValue) : std::random_device{}();
    std::cout << "TR_TEST_SEED=" << seed << std::endl;
    std::mt19937 random(seed);
    std::vector<const TrRouting::Point *> nodePoints;
    for (const auto &[uuid, node] : transitData.getNodes()) {
      nodePoints.push_back(node.point.get());
    }
    std::uniform_real_distribution<double> offset(-0.003, 0.003); // About 300 m
    std::uniform_int_distribution<int> departureTime(6 * 3600, 20 * 3600);
    auto randomPoint = [&]() {
      const TrRouting::Point &node = *nodePoints[random() % nodePoints.size()];
      return TrRouting::Point(node.latitude + offset(random), node.longitude + offset(random));
    };

    TrRouting::Calculator calculator(transitData, data->geoFilter);
    double footpathsMs = 0;
    double singleMs = 0;
    for (int i = 0; i < QUERIES_COUNT; i++) {
      auto parameters = makeParameters(randomPoint(), randomPoint(), departureTime(random));

      start = Clock::now();
      TrRouting::BatchedForwardQuery query {
        parameters->getTimeOfTrip(),
        data->geoFilter.getAccessibleNodesFootpathsFromPoint(*parameters->getOrigin(), transitData.getNodes(), parameters->getMaxAccessWalkingTravelTimeSeconds(), parameters->getWalkingSpeedMetersPerSecond()),
        data->geoFilter.getAccessibleNodesFootpathsFromPoint(*parameters->getDestination(), transitData.getNodes(), parameters->getMaxEgressWalkingTravelTimeSeconds(), parameters->getWalkingSpeedMetersPerSecond()),
        {}
      };
      footpathsMs += elapsedMs(start);
      data->queries.push_back(std::move(query));

      // Same steps as Calculator::calculateSingle until the forward calculation
      start = Clock::now();
      TrRouting::BatchedForwardResult result;
      try {
        calculator.reset(*parameters, *parameters->getOrigin(), *parameters->getDestination());
        std::unordered_map<TrRouting::Node::uid_t, TrRouting::JourneyStep> forwardEgressJourneysSteps;
        auto forward = calculator.forwardCalculation(*parameters, forwardEgressJourneysSteps);
        if (forward.has_value()) {
          result.arrivalTimeSeconds = std::get<0>(*forward);
          result.egressNodeUid = std::get<1>(*forward).get().uid;
        } else {
          result.noRoutingReason = TrRouting::NoRoutingReason::NO_ROUTING_FOUND;
        }
      } catch (const TrRouting::NoRoutingFoundException &e) {
        result.noRoutingReason = e.getReason();
      }
      singleMs += elapsedMs(start);
      data->expected.push_back(result);
      data->sharedParameters = std::move(parameters);
    }
    int arrivals = 0;
    for (const auto &result : data->expected) {
      arrivals += result.noRoutingReason.has_value() ? 0 : 1;
    }
    std::cout << QUERIES_COUNT << " queries, " << arrivals << " with an arrival" << std::endl;
    std::cout << "Access and egress footpaths: " << footpathsMs / QUERIES_COUNT << " ms per query" << std::endl;
    std::cout << "Single forward calculation, including the footpaths: " << singleMs / QUERIES_COUNT << " ms per query" << std::endl;
  }

  static void TearDownTestSuite()
  {
    data.reset();
  }
};

INSTANTIATE_TEST_SUITE_P(
  BatchSizes, BenchmarkBatchedForwardTests,
  testing::Values(1, 16, 64, 256, 1024)
);

// Run the queries in batches of the parameter size and compare each result with the single calculation
TEST_P(BenchmarkBatchedForwardTests, BenchmarkBatchedForward)
{
  const size_t batchSize = GetParam();
  // A batch scans the connections from its earliest to its latest query, so group the queries by departure time
  std::vector<size_t> order(data->queries.size());
  std::iota(order.begin(), order.end(), 0);
  std::sort(order.begin(), order.end(), [](size_t a, size_t b) { return data->queries[a].departureTimeSeconds < data->queries[b].departureTimeSeconds; });
  std::vector<std::vector<TrRouting::BatchedForwardQuery>> batches;
  for (size_t first = 0; first < order.size(); first += batchSize) {
    batches.emplace_back();
    for (size_t i = first; i < std::min(first + batchSize, order.size()); i++) {
      batches.back().push_back(data->queries[order[i]]);
    }
  }

  std::vector<TrRouting::BatchedForwardResult> results;
  auto start = Clock::now();
  for (const auto &batch : batches) {
    for (const auto &result : TrRouting::batchedForwardCalculation(*data->connections, *data->transfers, *data->sharedParameters, batch)) {
      results.push_back(result);
    }
  }
  std::cout << "Batches of " << batchSize << ": " << elapsedMs(start) / data->queries.size() << " ms per query" << std::endl;

  ASSERT_EQ(data->expected.size(), results.size());
  for (size_t i = 0; i < results.size(); i++) {
    const TrRouting::BatchedForwardResult &expected = data->expected[order[i]];
    SCOPED_TRACE("query " + std::to_string(order[i]));
    EXPECT_EQ(expected.noRoutingReason, results[i].noRoutingReason);
    EXPECT_EQ(expected.arrivalTimeSeconds, results[i].arrivalTimeSeconds);
    EXPECT_EQ(expected.egressNodeUid, results[i].egressNodeUid);
  }
}
