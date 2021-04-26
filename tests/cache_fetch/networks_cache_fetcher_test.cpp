#include <errno.h>
#include <experimental/filesystem>

#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "parameters.hpp"
#include "cache_fetcher.hpp"
#include "cache_fetcher_test.hpp"
#include "network.hpp"
#include "capnp/networkCollection.capnp.h"

namespace fs = std::experimental::filesystem;

class NetworkCacheFetcherFixtureTests : public BaseCacheFetcherFixtureTests
{
protected:
    std::vector<std::unique_ptr<TrRouting::Network>> objects;
    std::map<boost::uuids::uuid, int> objectIndexesByUuid;
    std::map<boost::uuids::uuid, int> agencyIndexesByUuid;
    std::map<boost::uuids::uuid, int> serviceIndexesByUuid;
    std::map<boost::uuids::uuid, int> scenarioIndexesByUuid;

public:
    void SetUp( ) override
    {
        BaseCacheFetcherFixtureTests::SetUp();
        // Copy the invalid file
        fs::copy_file(PROJECT_NAME + "/" + INVALID_CUSTOM_PATH + "/genericInvalid.capnpbin", PROJECT_NAME + "/" + INVALID_CUSTOM_PATH + "/networks.capnpbin");
    }

    void TearDown( ) override
    {
        BaseCacheFetcherFixtureTests::TearDown();
        // Remove the invalid file
        fs::remove(PROJECT_NAME + "/" + INVALID_CUSTOM_PATH + "/networks.capnpbin");
    }
};

TEST_F(NetworkCacheFetcherFixtureTests, TestGetNetworksInvalid)
{
    int retVal = cacheFetcher.getNetworks(objects, objectIndexesByUuid, agencyIndexesByUuid, serviceIndexesByUuid, scenarioIndexesByUuid, params, INVALID_CUSTOM_PATH);
    ASSERT_EQ(-EBADMSG, retVal);
    ASSERT_EQ(0, objects.size());
}

TEST_F(NetworkCacheFetcherFixtureTests, TestGetNetworksValid)
{
    int retVal = cacheFetcher.getNetworks(objects, objectIndexesByUuid, agencyIndexesByUuid, serviceIndexesByUuid, scenarioIndexesByUuid, params, VALID_CUSTOM_PATH);
    ASSERT_EQ(0, retVal);
    // TODO Test with actual data
    ASSERT_EQ(0, objects.size());
}

TEST_F(NetworkCacheFetcherFixtureTests, TestGetNetworksFileNotExists)
{
    int retVal = cacheFetcher.getNetworks(objects, objectIndexesByUuid, agencyIndexesByUuid, serviceIndexesByUuid, scenarioIndexesByUuid, params, BASE_CUSTOM_PATH);
    ASSERT_EQ(-ENOENT, retVal);
    ASSERT_EQ(0, objects.size());
}

