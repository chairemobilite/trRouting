#include <errno.h>
#include <filesystem>

#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "cache_fetcher.hpp"
#include "cache_fetcher_test.hpp"
#include "trip.hpp"
#include "line.hpp"
#include "node.hpp"
#include "capnp/line.capnp.h"
#include <boost/uuid/string_generator.hpp>

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
    // Services of the valid test cache: all the trips are on the first one
    const boost::uuids::uuid serviceWithTripsUuid = boost::uuids::string_generator()("cfbd9b3c-1e91-4e69-ba8b-f2bcf98c8e4d");
    const boost::uuids::uuid serviceWithoutTripsUuid = boost::uuids::string_generator()("c02e3197-9c0b-4541-a81f-6539a7a08d31");

    // The uuids of all the services, to load all the schedules
    std::set<boost::uuids::uuid> allServiceUuids()
    {
        std::set<boost::uuids::uuid> uuids;
        for (const auto & serviceIte : services)
        {
            uuids.insert(serviceIte.first);
        }
        return uuids;
    }

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
      allServiceUuids(),
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
      allServiceUuids(),
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
      allServiceUuids(),
      connections,
      VALID_CUSTOM_PATH
    );
    ASSERT_EQ(0, retVal);
    ASSERT_EQ(50u, trips.size());
    ASSERT_EQ(275u, connections.size());
}

TEST_F(ScheduleCacheFetcherFixtureTests, TestGetSchedulesServicesToLoadWithTrips)
{
    // Loading the service that has all the trips loads the same as loading all services
    int retVal = cacheFetcher.getSchedules(
      trips,
      lines,
      paths,
      services,
      std::set<boost::uuids::uuid>{serviceWithTripsUuid},
      connections,
      VALID_CUSTOM_PATH
    );
    ASSERT_EQ(0, retVal);
    ASSERT_EQ(50u, trips.size());
    ASSERT_EQ(275u, connections.size());
}

TEST_F(ScheduleCacheFetcherFixtureTests, TestGetSchedulesServicesToLoadWithoutTrips)
{
    // The schedules of the service with all the trips are skipped
    int retVal = cacheFetcher.getSchedules(
      trips,
      lines,
      paths,
      services,
      std::set<boost::uuids::uuid>{serviceWithoutTripsUuid},
      connections,
      VALID_CUSTOM_PATH
    );
    ASSERT_EQ(0, retVal);
    ASSERT_EQ(0u, trips.size());
    ASSERT_EQ(0u, connections.size());
}

TEST_F(ScheduleCacheFetcherFixtureTests, TestGetSchedulesNoServicesToLoad)
{
    // An empty set of services loads nothing
    int retVal = cacheFetcher.getSchedules(
      trips,
      lines,
      paths,
      services,
      std::set<boost::uuids::uuid>{},
      connections,
      VALID_CUSTOM_PATH
    );
    ASSERT_EQ(0, retVal);
    ASSERT_EQ(0u, trips.size());
    ASSERT_EQ(0u, connections.size());
}
