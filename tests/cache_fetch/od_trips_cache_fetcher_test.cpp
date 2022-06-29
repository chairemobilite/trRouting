#include <errno.h>
#include <filesystem>

#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "parameters.hpp"
#include "cache_fetcher.hpp"
#include "cache_fetcher_test.hpp"
#include "od_trip.hpp"

namespace fs = std::filesystem;

class OdTripCacheFetcherFixtureTests : public BaseCacheFetcherFixtureTests
{
protected:
    std::vector<std::unique_ptr<TrRouting::OdTrip>> objects;
    std::map<boost::uuids::uuid, int> objectIndexesByUuid;
    std::map<boost::uuids::uuid, int> dataSourceIndexesByUuid;
    std::map<boost::uuids::uuid, int> householdIndexesByUuid;
    std::map<boost::uuids::uuid, int> personIndexesByUuid;
    std::map<boost::uuids::uuid, int> nodeIndexesByUuid;
};

TEST_F(OdTripCacheFetcherFixtureTests, TestGetOdTripsValid)
{
    int retVal = cacheFetcher.getOdTrips(objects, objectIndexesByUuid, dataSourceIndexesByUuid, householdIndexesByUuid, personIndexesByUuid, nodeIndexesByUuid, params, VALID_CUSTOM_PATH);
    ASSERT_EQ(0, retVal);
    ASSERT_EQ(0, objects.size());
}

// TODO Test with actual data (and invalid ones)