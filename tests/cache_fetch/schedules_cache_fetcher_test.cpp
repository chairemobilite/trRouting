#include <errno.h>
#include <filesystem>

#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "cache_fetcher.hpp"
#include "cache_fetcher_test.hpp"
#include "trip.hpp"
#include "line.hpp"
#include "node.hpp"
#include "capnp/line.capnp.h"

namespace fs = std::filesystem;

class ScheduleCacheFetcherFixtureTests : public BaseCacheFetcherFixtureTests
{
protected:
    std::map<boost::uuids::uuid, TrRouting::Trip> trips;
    std::map<boost::uuids::uuid, TrRouting::Line> lines;
    std::map<boost::uuids::uuid, TrRouting::Path> paths;
    std::map<boost::uuids::uuid, TrRouting::Node> nodes;
    std::map<boost::uuids::uuid, int> tripIndexesByUuid;
    std::map<boost::uuids::uuid, TrRouting::Service> services;
    std::vector<TrRouting::Connection> connections;

public:
    void SetUp( ) override
    {
        BaseCacheFetcherFixtureTests::SetUp();
        // Read valid data for agencies, lines and paths
        std::map<boost::uuids::uuid, TrRouting::Agency> agencies;
        cacheFetcher.getAgencies(agencies, VALID_CUSTOM_PATH);
        auto modes = cacheFetcher.getModes();
        cacheFetcher.getServices(services, VALID_CUSTOM_PATH);

        cacheFetcher.getNodes(nodes, VALID_CUSTOM_PATH);
        cacheFetcher.getLines(lines, agencies, modes, VALID_CUSTOM_PATH);
        cacheFetcher.getPaths(paths, lines, nodes, VALID_CUSTOM_PATH);
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
    std::string node0Uuid = boost::uuids::to_string(lines.begin()->second.uuid);
    fs::copy_file(BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/genericInvalid.capnpbin", BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/lines/line_ " + node0Uuid.c_str() + ".capnpbin");
    int retVal = cacheFetcher.getSchedules(
      trips,
      lines,
      paths,
      services,
      connections,
      INVALID_CUSTOM_PATH
    );
    // TODO: Since a file was invalid, should this return -EBADMSG?
    ASSERT_EQ(0, retVal);
    ASSERT_EQ(0u, trips.size());
    ASSERT_EQ(0u, connections.size());
}

TEST_F(ScheduleCacheFetcherFixtureTests, TestGetUnexistingLineFiles)
{
    int retVal = cacheFetcher.getSchedules(
      trips,
      lines,
      paths,
      services,
      connections,
      INVALID_CUSTOM_PATH
    );
    ASSERT_EQ(0, retVal);
    ASSERT_EQ(0u, trips.size());
    ASSERT_EQ(0u, connections.size());
}

TEST_F(ScheduleCacheFetcherFixtureTests, TestGetSchedulesValid)
{
    int retVal = cacheFetcher.getSchedules(
      trips,
      lines,
      paths,
      services,
      connections,
      VALID_CUSTOM_PATH
    );
    ASSERT_EQ(0, retVal);
    ASSERT_EQ(50u, trips.size());
    ASSERT_EQ(275u, connections.size());
}

// A schedule of a service that is not in the services, for example a deleted
// one, should be ignored instead of failing the whole data load
TEST_F(ScheduleCacheFetcherFixtureTests, TestGetSchedulesUnknownService)
{
    // Remove the service of a trip of the valid data
    cacheFetcher.getSchedules(trips, lines, paths, services, connections, VALID_CUSTOM_PATH);
    const boost::uuids::uuid unknownServiceUuid = trips.begin()->second.service.uuid;
    connections.clear();
    trips.clear();
    services.erase(unknownServiceUuid);
    int retVal = cacheFetcher.getSchedules(
      trips,
      lines,
      paths,
      services,
      connections,
      VALID_CUSTOM_PATH
    );
    ASSERT_EQ(0, retVal);
    ASSERT_LT(trips.size(), 50u);
    for (const auto & [tripUuid, trip] : trips)
    {
        ASSERT_NE(unknownServiceUuid, trip.service.uuid);
    }
}
