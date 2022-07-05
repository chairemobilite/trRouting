#include <errno.h>
#include <filesystem>

#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "cache_fetcher.hpp"
#include "cache_fetcher_test.hpp"
#include "person.hpp"
#include "capnp/person.capnp.h"

namespace fs = std::filesystem;

class PersonCacheFetcherFixtureTests : public BaseCacheFetcherFixtureTests
{
protected:
    std::vector<std::unique_ptr<TrRouting::Person>> objects;
    std::map<boost::uuids::uuid, int> objectIndexesByUuid;
    std::map<boost::uuids::uuid, int> dataSourceIndexesByUuid;
    std::map<boost::uuids::uuid, int> householdIndexesByUuid;
    std::map<boost::uuids::uuid, int> nodeIndexesByUuid;
};

TEST_F(PersonCacheFetcherFixtureTests, TestGetPersonsValid)
{
    int retVal = cacheFetcher.getPersons(objects, objectIndexesByUuid, dataSourceIndexesByUuid, householdIndexesByUuid, nodeIndexesByUuid, VALID_CUSTOM_PATH);
    ASSERT_EQ(0, retVal);
    ASSERT_EQ(0, objects.size());
}

// TODO Test with actual data (and invalid ones)
