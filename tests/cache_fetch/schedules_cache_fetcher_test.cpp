#include <errno.h>
#include <experimental/filesystem>

#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "parameters.hpp"
#include "cache_fetcher.hpp"
#include "cache_fetcher_test.hpp"
#include "trip.hpp"
#include "line.hpp"
#include "node.hpp"
#include "block.hpp"
#include "capnp/line.capnp.h"

namespace fs = std::experimental::filesystem;

class ScheduleCacheFetcherFixtureTests : public BaseCacheFetcherFixtureTests
{
protected:
    std::vector<std::unique_ptr<TrRouting::Trip>> trips;
    std::vector<std::unique_ptr<TrRouting::Line>> lines;
    std::vector<std::unique_ptr<TrRouting::Path>> paths;
    std::vector<std::unique_ptr<TrRouting::Node>> nodes;
    std::map<boost::uuids::uuid, int> tripIndexesByUuid;
    std::map<boost::uuids::uuid, int> serviceIndexesByUuid;
    std::map<boost::uuids::uuid, int> lineIndexesByUuid;
    std::map<boost::uuids::uuid, int> pathIndexesByUuid;
    std::map<boost::uuids::uuid, int> agencyIndexesByUuid;
    std::map<boost::uuids::uuid, int> nodeIndexesByUuid;
    std::map<boost::uuids::uuid, int> stationIndexesByUuid;
    std::map<std::string, int> modeIndexesByShortname;
    std::vector<std::vector<std::unique_ptr<int>>> tripConnectionDepartureTimes;
    std::vector<std::vector<std::unique_ptr<float>>> tripConnectionDemands;
    std::vector<std::shared_ptr<std::tuple<int,int,int,int,int,short,short,int,int,int,short,short>>> forwardConnections;
    std::vector<std::shared_ptr<std::tuple<int,int,int,int,int,short,short,int,int,int,short,short>>> reverseConnections;

public:
    void SetUp( ) override
    {
        BaseCacheFetcherFixtureTests::SetUp();
        // Read valid data for agencies, lines and paths
        cacheFetcher.getNodes(nodes, nodeIndexesByUuid, stationIndexesByUuid, params, VALID_CUSTOM_PATH);
        cacheFetcher.getLines(lines, lineIndexesByUuid, agencyIndexesByUuid, modeIndexesByShortname, params, VALID_CUSTOM_PATH);
        cacheFetcher.getPaths(paths, pathIndexesByUuid, lineIndexesByUuid, nodeIndexesByUuid, params, VALID_CUSTOM_PATH);
        // Create the invalid lines directory
        fs::create_directory(PROJECT_NAME + "/" + INVALID_CUSTOM_PATH + "/lines");
    }

    void TearDown( ) override
    {
        BaseCacheFetcherFixtureTests::TearDown();
        // Remove the invalid lines directory
        fs::remove_all(PROJECT_NAME + "/" + INVALID_CUSTOM_PATH + "/lines");
    }
};

TEST_F(ScheduleCacheFetcherFixtureTests, TestGetSchedulesInvalidLineFile)
{
    // Copy the invalid file for the first line 
    std::string node0Uuid = boost::uuids::to_string(lines[0].get()->uuid);
    fs::copy_file(PROJECT_NAME + "/" + INVALID_CUSTOM_PATH + "/genericInvalid.capnpbin", PROJECT_NAME + "/" + INVALID_CUSTOM_PATH + "/lines/line_ " + node0Uuid.c_str() + ".capnpbin");
    int retVal = cacheFetcher.getSchedules(
      trips,
      lines,
      paths,
      tripIndexesByUuid,
      serviceIndexesByUuid,
      lineIndexesByUuid,
      pathIndexesByUuid,
      agencyIndexesByUuid,
      nodeIndexesByUuid,
      modeIndexesByShortname,
      tripConnectionDepartureTimes,
      tripConnectionDemands,
      forwardConnections,
      reverseConnections,
      params,
      INVALID_CUSTOM_PATH
    );
    // TODO: Since a file was invalid, should this return -EBADMSG?
    ASSERT_EQ(0, retVal);
    ASSERT_EQ(0, trips.size());
}

TEST_F(ScheduleCacheFetcherFixtureTests, TestGetUnexistingLineFiles)
{
    int retVal = cacheFetcher.getSchedules(
      trips,
      lines,
      paths,
      tripIndexesByUuid,
      serviceIndexesByUuid,
      lineIndexesByUuid,
      pathIndexesByUuid,
      agencyIndexesByUuid,
      nodeIndexesByUuid,
      modeIndexesByShortname,
      tripConnectionDepartureTimes,
      tripConnectionDemands,
      forwardConnections,
      reverseConnections,
      params,
      INVALID_CUSTOM_PATH
    );
    ASSERT_EQ(0, retVal);
    ASSERT_EQ(0, trips.size());
}

TEST_F(ScheduleCacheFetcherFixtureTests, TestGetSchedulesValid)
{
    int retVal = cacheFetcher.getSchedules(
      trips,
      lines,
      paths,
      tripIndexesByUuid,
      serviceIndexesByUuid,
      lineIndexesByUuid,
      pathIndexesByUuid,
      agencyIndexesByUuid,
      nodeIndexesByUuid,
      modeIndexesByShortname,
      tripConnectionDepartureTimes,
      tripConnectionDemands,
      forwardConnections,
      reverseConnections,
      params,
      VALID_CUSTOM_PATH
    );
    ASSERT_EQ(0, retVal);
    ASSERT_EQ(50, trips.size());
}
