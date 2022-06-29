#include <errno.h>
#include <filesystem>

#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "parameters.hpp"
#include "cache_fetcher.hpp"
#include "cache_fetcher_test.hpp"
#include "agency.hpp"
#include "capnp/agencyCollection.capnp.h"

namespace fs = std::filesystem;

class AgencyCacheFetcherFixtureTests : public BaseCacheFetcherFixtureTests
{
protected:
    std::vector<std::unique_ptr<TrRouting::Agency>>     agencies;
    std::map<boost::uuids::uuid, int>        agencyIndexesByUuid;

public:
    void SetUp( ) override
    { 
        BaseCacheFetcherFixtureTests::SetUp();
        // Copy the invalid file
        fs::copy_file(PROJECT_NAME + "/" + INVALID_CUSTOM_PATH + "/genericInvalid.capnpbin", PROJECT_NAME + "/" + INVALID_CUSTOM_PATH + "/agencies.capnpbin");
    }

    void TearDown( ) override
    { 
        BaseCacheFetcherFixtureTests::TearDown();
        // Remove the invalid file
        fs::remove(PROJECT_NAME + "/" + INVALID_CUSTOM_PATH + "/agencies.capnpbin");
    }
};

TEST_F(AgencyCacheFetcherFixtureTests, TestGetAgenciesInvalid)
{
    int retVal = cacheFetcher.getAgencies(agencies, agencyIndexesByUuid, params, INVALID_CUSTOM_PATH);
    ASSERT_EQ(-EBADMSG, retVal);
    ASSERT_EQ(0, agencies.size());
}

TEST_F(AgencyCacheFetcherFixtureTests, TestGetAgenciesValid)
{
    int retVal = cacheFetcher.getAgencies(agencies, agencyIndexesByUuid, params, VALID_CUSTOM_PATH);
    ASSERT_EQ(0, retVal);
    ASSERT_EQ(2, agencies.size());
}

TEST_F(AgencyCacheFetcherFixtureTests, TestGetAgenciesFileNotExists)
{
    int retVal = cacheFetcher.getAgencies(agencies, agencyIndexesByUuid, params, BASE_CUSTOM_PATH);
    ASSERT_EQ(-ENOENT, retVal);
    ASSERT_EQ(0, agencies.size());
}

