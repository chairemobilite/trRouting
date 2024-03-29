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
    std::map<boost::uuids::uuid, TrRouting::OdTrip> objects;
    std::map<boost::uuids::uuid, TrRouting::DataSource> dataSources;
    std::map<boost::uuids::uuid, TrRouting::Person> persons;
    std::map<boost::uuids::uuid, TrRouting::Node> nodes;
};

TEST_F(OdTripCacheFetcherFixtureTests, TestGetOdTripsValid)
{
    int retVal = cacheFetcher.getOdTrips(objects, dataSources, persons, nodes, VALID_CUSTOM_PATH);
    ASSERT_EQ(0, retVal);
    ASSERT_EQ(0u, objects.size());
}

// TODO Test with actual data (and invalid ones)
