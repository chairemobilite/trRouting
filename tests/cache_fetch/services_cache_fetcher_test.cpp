#include <errno.h>
#include <filesystem>

#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "cache_fetcher.hpp"
#include "cache_fetcher_test.hpp"
#include "service.hpp"
#include "capnp/serviceCollection.capnp.h"

namespace fs = std::filesystem;

class ServiceCacheFetcherFixtureTests : public BaseCacheFetcherFixtureTests
{
protected:
    std::map<boost::uuids::uuid, TrRouting::Service> services;

public:
    void SetUp( ) override
    {
        BaseCacheFetcherFixtureTests::SetUp();
        // Copy the invalid file
        fs::copy_file(BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/genericInvalid.capnpbin", BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/services.capnpbin");
    }

    void TearDown( ) override
    {
        BaseCacheFetcherFixtureTests::TearDown();
        // Remove the invalid file
        fs::remove(BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/services.capnpbin");
    }
};

TEST_F(ServiceCacheFetcherFixtureTests, TestGetServicesInvalid)
{
    int retVal = cacheFetcher.getServices(services, INVALID_CUSTOM_PATH);
    ASSERT_EQ(-EBADMSG, retVal);
    ASSERT_EQ(0u, services.size());
}

TEST_F(ServiceCacheFetcherFixtureTests, TestGetServicesValid)
{
    int retVal = cacheFetcher.getServices(services, VALID_CUSTOM_PATH);
    ASSERT_EQ(0, retVal);
    ASSERT_EQ(2u, services.size());
}

TEST_F(ServiceCacheFetcherFixtureTests, TestGetServicesFileNotExists)
{
    int retVal = cacheFetcher.getServices(services, BASE_CUSTOM_PATH);
    ASSERT_EQ(-ENOENT, retVal);
    ASSERT_EQ(0u, services.size());
}

