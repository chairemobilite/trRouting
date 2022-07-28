#include <errno.h>
#include <filesystem>

#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "cache_fetcher.hpp"
#include "cache_fetcher_test.hpp"
#include "agency.hpp"
#include "capnp/agencyCollection.capnp.h"

namespace fs = std::filesystem;

class AgencyCacheFetcherFixtureTests : public BaseCacheFetcherFixtureTests
{
protected:
    std::map<boost::uuids::uuid, TrRouting::Agency> agencies;

public:
    void SetUp( ) override
    { 
        BaseCacheFetcherFixtureTests::SetUp();
        // Copy the invalid file
        fs::copy_file(BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/genericInvalid.capnpbin", BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/agencies.capnpbin");
    }

    void TearDown( ) override
    { 
        BaseCacheFetcherFixtureTests::TearDown();
        // Remove the invalid file
        fs::remove(BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/agencies.capnpbin");
    }
};

TEST_F(AgencyCacheFetcherFixtureTests, TestGetAgenciesInvalid)
{
    int retVal = cacheFetcher.getAgencies(agencies, INVALID_CUSTOM_PATH);
    ASSERT_EQ(-EBADMSG, retVal);
    ASSERT_EQ(0, agencies.size());
}

TEST_F(AgencyCacheFetcherFixtureTests, TestGetAgenciesValid)
{
    int retVal = cacheFetcher.getAgencies(agencies, VALID_CUSTOM_PATH);
    ASSERT_EQ(0, retVal);
    ASSERT_EQ(2, agencies.size());
}

TEST_F(AgencyCacheFetcherFixtureTests, TestGetAgenciesFileNotExists)
{
    int retVal = cacheFetcher.getAgencies(agencies, BASE_CUSTOM_PATH);
    ASSERT_EQ(-ENOENT, retVal);
    ASSERT_EQ(0, agencies.size());
}

