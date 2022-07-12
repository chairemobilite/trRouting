#include <errno.h>
#include <filesystem>

#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "cache_fetcher.hpp"
#include "cache_fetcher_test.hpp"
#include "trip.hpp"
#include "line.hpp"
#include "node.hpp"
#include "block.hpp"
#include "capnp/line.capnp.h"

namespace fs = std::filesystem;

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
    std::vector<std::shared_ptr<std::tuple<int,int,int,int,int,short,short,int,int,int,short,short>>> connections;

public:
    void SetUp( ) override
    {
        BaseCacheFetcherFixtureTests::SetUp();
        // Read valid data for agencies, lines and paths
        std::vector<std::unique_ptr<TrRouting::Agency>>     agencies;
        cacheFetcher.getAgencies(agencies, agencyIndexesByUuid, VALID_CUSTOM_PATH);
        std::vector<TrRouting::Mode>                        modes;
        std::tie(modes, modeIndexesByShortname) = cacheFetcher.getModes();
        std::vector<std::unique_ptr<TrRouting::Service>> services;
        cacheFetcher.getServices(services, serviceIndexesByUuid, VALID_CUSTOM_PATH);

        cacheFetcher.getNodes(nodes, nodeIndexesByUuid, stationIndexesByUuid, VALID_CUSTOM_PATH);
        cacheFetcher.getLines(lines, lineIndexesByUuid, agencyIndexesByUuid, modeIndexesByShortname, VALID_CUSTOM_PATH);
        cacheFetcher.getPaths(paths, pathIndexesByUuid, lineIndexesByUuid, nodeIndexesByUuid, VALID_CUSTOM_PATH);
        // Create the invalid lines directory
        fs::create_directory(BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/lines");
    }

    void TearDown( ) override
    {
        BaseCacheFetcherFixtureTests::TearDown();
        // Remove the invalid lines directory
        fs::remove_all(BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/lines");
    }
};

TEST_F(ScheduleCacheFetcherFixtureTests, TestGetSchedulesInvalidLineFile)
{
    // Copy the invalid file for the first line 
    std::string node0Uuid = boost::uuids::to_string(lines[0].get()->uuid);
    fs::copy_file(BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/genericInvalid.capnpbin", BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/lines/line_ " + node0Uuid.c_str() + ".capnpbin");
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
      connections,
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
      connections,
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
      connections,
      VALID_CUSTOM_PATH
    );
    ASSERT_EQ(0, retVal);
    ASSERT_EQ(50, trips.size());
}
