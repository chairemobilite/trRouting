#include <errno.h>
#include <filesystem>

#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "parameters.hpp"
#include "cache_fetcher.hpp"
#include "cache_fetcher_test.hpp"
#include "household.hpp"
#include "capnp/household.capnp.h"

namespace fs = std::filesystem;

class HouseholdCacheFetcherFixtureTests : public BaseCacheFetcherFixtureTests
{
protected:
    std::vector<std::unique_ptr<TrRouting::Household>> objects;
    std::map<boost::uuids::uuid, int> objectIndexesByUuid;
    std::map<boost::uuids::uuid, int> dataSourceIndexesByUuid;
    std::map<boost::uuids::uuid, int> nodeIndexesByUuid;
};

TEST_F(HouseholdCacheFetcherFixtureTests, TestGetHouseholdsValid)
{
    int retVal = cacheFetcher.getHouseholds(objects, objectIndexesByUuid, dataSourceIndexesByUuid, nodeIndexesByUuid, params, VALID_CUSTOM_PATH);
    ASSERT_EQ(0, retVal);
    ASSERT_EQ(0, objects.size());
}

// TODO Test with actual data (and invalid ones)