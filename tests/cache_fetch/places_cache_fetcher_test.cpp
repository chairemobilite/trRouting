#include <errno.h>
#include <filesystem>

#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "cache_fetcher.hpp"
#include "cache_fetcher_test.hpp"
#include "place.hpp"

namespace fs = std::filesystem;

class PlaceCacheFetcherFixtureTests : public BaseCacheFetcherFixtureTests
{
protected:
    std::vector<std::unique_ptr<TrRouting::Place>> objects;
    std::map<boost::uuids::uuid, int> objectIndexesByUuid;
    std::map<boost::uuids::uuid, int> dataSourceIndexesByUuid;
    std::map<boost::uuids::uuid, int> nodeIndexesByUuid;
};

TEST_F(PlaceCacheFetcherFixtureTests, TestGetPlacesValid)
{
    int retVal = cacheFetcher.getPlaces(objects, objectIndexesByUuid, dataSourceIndexesByUuid, nodeIndexesByUuid, VALID_CUSTOM_PATH);
    ASSERT_EQ(0, retVal);
    ASSERT_EQ(0, objects.size());
}

// TODO Test with actual data (and invalid ones)
