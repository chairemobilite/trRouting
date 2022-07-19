#include <errno.h>
#include <filesystem>

#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "cache_fetcher.hpp"
#include "cache_fetcher_test.hpp"
#include "od_trip.hpp"

namespace fs = std::filesystem;

class OdTripCacheFetcherFixtureTests : public BaseCacheFetcherFixtureTests
{
protected:
    std::vector<std::unique_ptr<TrRouting::OdTrip>> objects;
    std::map<boost::uuids::uuid, int> objectIndexesByUuid;
    std::map<boost::uuids::uuid, TrRouting::DataSource> dataSources;
    std::map<boost::uuids::uuid, int> personIndexesByUuid;
    std::map<boost::uuids::uuid, int> nodeIndexesByUuid;
};

TEST_F(OdTripCacheFetcherFixtureTests, TestGetOdTripsValid)
{
    int retVal = cacheFetcher.getOdTrips(objects, objectIndexesByUuid, dataSources, personIndexesByUuid, nodeIndexesByUuid, VALID_CUSTOM_PATH);
    ASSERT_EQ(0, retVal);
    ASSERT_EQ(0, objects.size());
}

// TODO Test with actual data (and invalid ones)
