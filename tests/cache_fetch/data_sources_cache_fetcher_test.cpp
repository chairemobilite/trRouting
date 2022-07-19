#include <errno.h>
#include <filesystem>

#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "cache_fetcher.hpp"
#include "cache_fetcher_test.hpp"
#include "data_source.hpp"
#include "capnp/dataSourceCollection.capnp.h"

namespace fs = std::filesystem;

class DataSourceCacheFetcherFixtureTests : public BaseCacheFetcherFixtureTests
{
protected:
    std::map<boost::uuids::uuid, TrRouting::DataSource> objects;

public:
    void SetUp( ) override
    {
        BaseCacheFetcherFixtureTests::SetUp();
        // Copy the invalid file
        fs::copy_file(BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/genericInvalid.capnpbin", BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/dataSources.capnpbin");
    }

    void TearDown( ) override
    {
        BaseCacheFetcherFixtureTests::TearDown();
        // Remove the invalid file
        fs::remove(BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/dataSources.capnpbin");
    }
};

TEST_F(DataSourceCacheFetcherFixtureTests, TestGetDataSourcesInvalid)
{
    int retVal = cacheFetcher.getDataSources(objects, INVALID_CUSTOM_PATH);
    ASSERT_EQ(-EBADMSG, retVal);
    ASSERT_EQ(0, objects.size());
}

TEST_F(DataSourceCacheFetcherFixtureTests, TestGetDataSourcesValid)
{
    int retVal = cacheFetcher.getDataSources(objects, VALID_CUSTOM_PATH);
    ASSERT_EQ(0, retVal);
    // TODO Test with actual data
    ASSERT_EQ(0, objects.size());
}

TEST_F(DataSourceCacheFetcherFixtureTests, TestGetDataSourcesFileNotExists)
{
    int retVal = cacheFetcher.getDataSources(objects, BASE_CUSTOM_PATH);
    ASSERT_EQ(-ENOENT, retVal);
    ASSERT_EQ(0, objects.size());
}

