#include <errno.h>
#include <experimental/filesystem>

#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "parameters.hpp"
#include "cache_fetcher.hpp"
#include "cache_fetcher_test.hpp"
#include "service.hpp"
#include "capnp/serviceCollection.capnp.h"

namespace fs = std::experimental::filesystem;

class ServiceCacheFetcherFixtureTests : public BaseCacheFetcherFixtureTests
{
protected:
    std::vector<std::unique_ptr<TrRouting::Service>>     objects;
    std::map<boost::uuids::uuid, int>        objectIndexesByUuid;

public:
    void SetUp( ) override
    {
        BaseCacheFetcherFixtureTests::SetUp();
        // Copy the invalid file
        fs::copy_file(PROJECT_NAME + "/" + INVALID_CUSTOM_PATH + "/genericInvalid.capnpbin", PROJECT_NAME + "/" + INVALID_CUSTOM_PATH + "/services.capnpbin");
    }

    void TearDown( ) override
    {
        BaseCacheFetcherFixtureTests::TearDown();
        // Remove the invalid file
        fs::remove(PROJECT_NAME + "/" + INVALID_CUSTOM_PATH + "/services.capnpbin");
    }
};

TEST_F(ServiceCacheFetcherFixtureTests, TestGetServicesInvalid)
{
    int retVal = cacheFetcher.getServices(objects, objectIndexesByUuid, params, INVALID_CUSTOM_PATH);
    ASSERT_EQ(-EBADMSG, retVal);
    ASSERT_EQ(0, objects.size());
}

TEST_F(ServiceCacheFetcherFixtureTests, TestGetServicesValid)
{
    int retVal = cacheFetcher.getServices(objects, objectIndexesByUuid, params, VALID_CUSTOM_PATH);
    ASSERT_EQ(0, retVal);
    ASSERT_EQ(2, objects.size());
}

TEST_F(ServiceCacheFetcherFixtureTests, TestGetServicesFileNotExists)
{
    int retVal = cacheFetcher.getServices(objects, objectIndexesByUuid, params, BASE_CUSTOM_PATH);
    ASSERT_EQ(-ENOENT, retVal);
    ASSERT_EQ(0, objects.size());
}

