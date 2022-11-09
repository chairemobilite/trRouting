#include <errno.h>
#include <filesystem>

#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "cache_fetcher.hpp"
#include "cache_fetcher_test.hpp"
#include "line.hpp"
#include "capnp/lineCollection.capnp.h"

namespace fs = std::filesystem;

class LineCacheFetcherFixtureTests : public BaseCacheFetcherFixtureTests
{
protected:
    std::map<boost::uuids::uuid, TrRouting::Line> lines;
    std::map<boost::uuids::uuid, TrRouting::Agency> agencies;
    std::map<std::string, TrRouting::Mode> modes;

public:
    void SetUp( ) override
    {
        BaseCacheFetcherFixtureTests::SetUp();
        // Copy the invalid file
        fs::copy_file(BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/genericInvalid.capnpbin", BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/lines.capnpbin");

        // Load valid data 
        cacheFetcher.getAgencies(agencies, VALID_CUSTOM_PATH);

        modes = cacheFetcher.getModes();
        
    }

    void TearDown( ) override
    {
        BaseCacheFetcherFixtureTests::TearDown();
        // Remove the invalid file
        fs::remove(BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/lines.capnpbin");
    }
};

TEST_F(LineCacheFetcherFixtureTests, TestGetLinesInvalid)
{
    int retVal = cacheFetcher.getLines(lines, agencies, modes, INVALID_CUSTOM_PATH);
    ASSERT_EQ(-EBADMSG, retVal);
    ASSERT_EQ(0u, lines.size());
}

TEST_F(LineCacheFetcherFixtureTests, TestGetLinesValid)
{
    int retVal = cacheFetcher.getLines(lines, agencies, modes, VALID_CUSTOM_PATH);
    ASSERT_EQ(0, retVal);
    ASSERT_EQ(2u, lines.size());
}

TEST_F(LineCacheFetcherFixtureTests, TestGetLinesFileNotExists)
{
    int retVal = cacheFetcher.getLines(lines, agencies, modes, BASE_CUSTOM_PATH);
    ASSERT_EQ(-ENOENT, retVal);
    ASSERT_EQ(0u, lines.size());
}

