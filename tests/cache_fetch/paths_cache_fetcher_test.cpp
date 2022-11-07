#include <errno.h>
#include <filesystem>

#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "cache_fetcher.hpp"
#include "cache_fetcher_test.hpp"
#include "path.hpp"
#include "capnp/pathCollection.capnp.h"

namespace fs = std::filesystem;

class PathCacheFetcherFixtureTests : public BaseCacheFetcherFixtureTests
{
protected:
    std::map<boost::uuids::uuid, TrRouting::Path> objects;
    std::map<boost::uuids::uuid, TrRouting::Line> lines;
    std::map<boost::uuids::uuid, TrRouting::Node> nodes;

public:
    void SetUp( ) override
    {
        BaseCacheFetcherFixtureTests::SetUp();
        // Copy the invalid file
        fs::copy_file(BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/genericInvalid.capnpbin", BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/paths.capnpbin");

        // Load valid data 
        std::map<boost::uuids::uuid, TrRouting::Agency> agencies;
        cacheFetcher.getAgencies(agencies, VALID_CUSTOM_PATH);

        auto modes = cacheFetcher.getModes();
        cacheFetcher.getLines(lines, agencies, modes, VALID_CUSTOM_PATH);

        cacheFetcher.getNodes(nodes, VALID_CUSTOM_PATH);
    }

    void TearDown( ) override
    {
        BaseCacheFetcherFixtureTests::TearDown();
        // Remove the invalid file
        fs::remove(BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/paths.capnpbin");
    }
};

TEST_F(PathCacheFetcherFixtureTests, TestGetPathsInvalid)
{
    int retVal = cacheFetcher.getPaths(objects, lines, nodes, INVALID_CUSTOM_PATH);
    ASSERT_EQ(-EBADMSG, retVal);
    ASSERT_EQ(0, objects.size());
}

// TODO Add tests for various services, lines, agencies that don't exist. But first, we should be able to create cache files with mock test data
TEST_F(PathCacheFetcherFixtureTests, TestGetPathsValid)
{
    int retVal = cacheFetcher.getPaths(objects, lines, nodes, VALID_CUSTOM_PATH);
    ASSERT_EQ(0, retVal);
    ASSERT_EQ(4, objects.size());
}

TEST_F(PathCacheFetcherFixtureTests, TestGetPathsFileNotExists)
{
    int retVal = cacheFetcher.getPaths(objects, lines, nodes, BASE_CUSTOM_PATH);
    ASSERT_EQ(-ENOENT, retVal);
    ASSERT_EQ(0, objects.size());
}

