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
    std::map<boost::uuids::uuid, TrRouting::Person> objects;
    std::map<boost::uuids::uuid, TrRouting::DataSource> dataSources;
};

TEST_F(PersonCacheFetcherFixtureTests, TestGetPersonsValid)
{
    int retVal = cacheFetcher.getPersons(objects, dataSources, VALID_CUSTOM_PATH);
    ASSERT_EQ(0, retVal);
    ASSERT_EQ(0u, objects.size());
}

// TODO Test with actual data (and invalid ones)
