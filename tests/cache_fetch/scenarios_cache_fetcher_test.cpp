#include <errno.h>
#include <filesystem>

#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "cache_fetcher.hpp"
#include "cache_fetcher_test.hpp"
#include "scenario.hpp"
#include "capnp/scenarioCollection.capnp.h"

namespace fs = std::filesystem;

class ScenarioCacheFetcherFixtureTests : public BaseCacheFetcherFixtureTests
{
protected:
    std::vector<std::unique_ptr<TrRouting::Scenario>> objects;
    std::map<boost::uuids::uuid, int> objectIndexesByUuid;
    std::map<boost::uuids::uuid, TrRouting::Service> services;
    std::map<boost::uuids::uuid, TrRouting::Line> lines;
    std::map<boost::uuids::uuid, TrRouting::Agency> agencies;
    std::map<boost::uuids::uuid, TrRouting::Node> nodes;
    std::map<std::string, TrRouting::Mode> modes;

public:
    void SetUp( ) override
    {
        BaseCacheFetcherFixtureTests::SetUp();
        // Copy the invalid file
        fs::copy_file(BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/genericInvalid.capnpbin", BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/scenarios.capnpbin");

        // Load valid data 
        cacheFetcher.getAgencies(agencies, VALID_CUSTOM_PATH);

        modes = cacheFetcher.getModes();
        cacheFetcher.getLines(lines, agencies, modes, VALID_CUSTOM_PATH);

        cacheFetcher.getNodes(nodes, VALID_CUSTOM_PATH);

        cacheFetcher.getServices(services, VALID_CUSTOM_PATH);

    }

    void TearDown( ) override
    {
        BaseCacheFetcherFixtureTests::TearDown();
        // Remove the invalid file
        fs::remove(BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/scenarios.capnpbin");
    }
};

TEST_F(ScenarioCacheFetcherFixtureTests, TestGetScenariosInvalid)
{
    int retVal = cacheFetcher.getScenarios(objects, objectIndexesByUuid, services, lines, agencies, nodes, modes, INVALID_CUSTOM_PATH);
    ASSERT_EQ(-EBADMSG, retVal);
    ASSERT_EQ(0, objects.size());
}

// TODO Add tests for various services, lines, agencies that don't exist. But first, we should be able to create cache files with mock test data
TEST_F(ScenarioCacheFetcherFixtureTests, TestGetScenariosValid)
{
    int retVal = cacheFetcher.getScenarios(objects, objectIndexesByUuid, services, lines, agencies, nodes, modes, VALID_CUSTOM_PATH);
    ASSERT_EQ(0, retVal);
    ASSERT_EQ(2, objects.size());
}

TEST_F(ScenarioCacheFetcherFixtureTests, TestGetScenariosFileNotExists)
{
    int retVal = cacheFetcher.getScenarios(objects, objectIndexesByUuid, services, lines, agencies, nodes, modes, BASE_CUSTOM_PATH);
    ASSERT_EQ(-ENOENT, retVal);
    ASSERT_EQ(0, objects.size());
}

