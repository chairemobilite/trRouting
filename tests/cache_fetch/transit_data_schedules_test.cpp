#include <set>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/string_generator.hpp>

#include "gtest/gtest.h"
#include "cache_fetcher.hpp"
#include "cache_fetcher_test.hpp"
#include "transit_data.hpp"
#include "scenario.hpp"

namespace
{
  const boost::uuids::string_generator uuidGenerator;
  // Scenarios of the valid test cache, each with its own service. Only the
  // service of the first scenario has trips: 50 trips and 275 connections.
  const boost::uuids::uuid scenarioWithTripsUuid = uuidGenerator("1d569191-d7fe-41ab-a7bc-55bea5a9ae6c");
  const boost::uuids::uuid scenarioWithoutTripsUuid = uuidGenerator("b1489f60-aa44-4a61-b547-6783b4dbdd84");
}

/**
 * Cache fetcher that reads the valid test cache but hides some of its
 * scenarios, to simulate a cache where the services of these scenarios are
 * not used by any scenario, or removes the services of some scenarios.
 */
class HiddenScenariosCacheFetcher : public TrRouting::CacheFetcher
{
public:
  HiddenScenariosCacheFetcher(const std::string &cacheDir) : TrRouting::CacheFetcher(cacheDir) {}

  std::set<boost::uuids::uuid> hiddenScenarios;
  std::set<boost::uuids::uuid> scenariosWithoutServices;

  virtual int getScenarios(
    std::map<boost::uuids::uuid, TrRouting::Scenario>& ts,
    const std::map<boost::uuids::uuid, TrRouting::Service>& services,
    const std::map<boost::uuids::uuid, TrRouting::Line>& lines,
    const std::map<boost::uuids::uuid, TrRouting::Agency>& agencies,
    const std::map<boost::uuids::uuid, TrRouting::Node>& nodes,
    const std::map<std::string, TrRouting::Mode>& modes,
    std::string customPath = ""
  ) override
  {
    int ret = TrRouting::CacheFetcher::getScenarios(ts, services, lines, agencies, nodes, modes, customPath);
    for (const boost::uuids::uuid & scenarioUuid : hiddenScenarios)
    {
      ts.erase(scenarioUuid);
    }
    for (const boost::uuids::uuid & scenarioUuid : scenariosWithoutServices)
    {
      ts.at(scenarioUuid).servicesList.clear();
    }
    return ret;
  }
};

class TransitDataSchedulesFixtureTests : public ConstantCacheFetcherFixtureTests
{
protected:
  HiddenScenariosCacheFetcher cacheFetcher = HiddenScenariosCacheFetcher(BASE_CACHE_DIRECTORY_NAME + "/" + VALID_CUSTOM_PATH);
};

// With all the scenarios, the services of both scenarios are loaded
TEST_F(TransitDataSchedulesFixtureTests, TestLoadServicesOfAllScenarios)
{
  TrRouting::TransitData transitData(cacheFetcher);

  ASSERT_EQ(2u, transitData.getServicesUsedByScenarios().size());
  ASSERT_EQ(50u, transitData.getTrips().size());
  ASSERT_EQ(275u, transitData.getConnectionCount());
}

// The trips of a service that no scenario uses are not loaded
TEST_F(TransitDataSchedulesFixtureTests, TestSkipServicesWithoutScenario)
{
  cacheFetcher.hiddenScenarios = {scenarioWithTripsUuid};
  TrRouting::TransitData transitData(cacheFetcher);

  ASSERT_EQ(0u, transitData.getTrips().size());
  ASSERT_EQ(0u, transitData.getConnectionCount());
  ASSERT_EQ(TrRouting::DataStatus::NO_SCHEDULES, transitData.getDataStatus());
}

// A scenario without services uses all the services, so all the schedules are loaded
TEST_F(TransitDataSchedulesFixtureTests, TestLoadAllServicesForScenarioWithoutServices)
{
  cacheFetcher.hiddenScenarios = {scenarioWithTripsUuid};
  cacheFetcher.scenariosWithoutServices = {scenarioWithoutTripsUuid};
  TrRouting::TransitData transitData(cacheFetcher);

  ASSERT_EQ(2u, transitData.getServicesUsedByScenarios().size());
  ASSERT_EQ(50u, transitData.getTrips().size());
  ASSERT_EQ(275u, transitData.getConnectionCount());
}

// Without scenarios, no query is possible, so no schedules are loaded
TEST_F(TransitDataSchedulesFixtureTests, TestNoSchedulesWithoutScenarios)
{
  cacheFetcher.hiddenScenarios = {scenarioWithTripsUuid, scenarioWithoutTripsUuid};
  TrRouting::TransitData transitData(cacheFetcher);

  ASSERT_TRUE(transitData.getServicesUsedByScenarios().empty());
  ASSERT_EQ(0u, transitData.getTrips().size());
  ASSERT_EQ(0u, transitData.getConnectionCount());
  ASSERT_EQ(TrRouting::DataStatus::NO_SCENARIOS, transitData.getDataStatus());
}
