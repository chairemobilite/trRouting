#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "parameters.hpp"
#include "cache_fetcher.hpp"
#include "cache_fetcher_test.hpp"

class CacheFetcherFixtureTests : public ConstantCacheFetcherFixtureTests {
protected:
    const std::string CACHE_FILE = "cacheFile.capnpbin";
    const std::string relativeCacheDir = "cache";
};

TEST_F(CacheFetcherFixtureTests, TestGetFilePath)
{
    // Test with default params values
    std::string filePath = TrRouting::CacheFetcher::getFilePath(CACHE_FILE, params, "");
    EXPECT_STREQ(filePath.c_str(), CACHE_FILE.c_str());

    // Test with default params and custom path
    filePath = TrRouting::CacheFetcher::getFilePath(CACHE_FILE, params, VALID_CUSTOM_PATH);
    EXPECT_STREQ(filePath.c_str(), (VALID_CUSTOM_PATH + '/' + CACHE_FILE).c_str());

    // Put all parameters
    params.projectShortname = PROJECT_NAME;
    filePath = TrRouting::CacheFetcher::getFilePath(CACHE_FILE, params, "");
    EXPECT_STREQ(filePath.c_str(), (PROJECT_NAME + '/' + CACHE_FILE).c_str());

    // Omit the project name in params
    params.cacheDirectoryPath = relativeCacheDir;
    filePath = TrRouting::CacheFetcher::getFilePath(CACHE_FILE, params, "");
    EXPECT_STREQ(filePath.c_str(), (relativeCacheDir + '/' + CACHE_FILE).c_str());

    // Omit the cache directory path
    params.cacheDirectoryPath = "";
    filePath = TrRouting::CacheFetcher::getFilePath(CACHE_FILE, params, "");
    EXPECT_STREQ(filePath.c_str(), (PROJECT_NAME + '/' + CACHE_FILE).c_str());
}

TEST_F(CacheFetcherFixtureTests, TestFileExists)
{
    // Test a file that exists
    std::string cacheFile = "testCache/validCacheFiles/agencies.capnpbin";
    ASSERT_TRUE(TrRouting::CacheFetcher::capnpCacheFileExists(cacheFile));

    // Test a file that does not exist
    cacheFile = "testCache/notexists.capnpbin";
    ASSERT_FALSE(TrRouting::CacheFetcher::capnpCacheFileExists(cacheFile));
}

TEST_F(CacheFetcherFixtureTests, TestGetFileCount)
{
    // Test count with a count file that does not exist
    std::string cacheFile = "testCache/notexists.capnpbin";
    ASSERT_EQ(1, TrRouting::CacheFetcher::getCacheFilesCount(cacheFile));

    // Test a file that contains a count
    cacheFile = "testCache/someCache.capnpbin.count";
    ASSERT_EQ(10, TrRouting::CacheFetcher::getCacheFilesCount(cacheFile));

    // Read a value from an invalid file.
    // TODO It returns 0, is this the right expected value?
    cacheFile = "testCache/invalidCacheFiles/genericInvalid.capnpbin";
    ASSERT_EQ(0, TrRouting::CacheFetcher::getCacheFilesCount(cacheFile));
}

