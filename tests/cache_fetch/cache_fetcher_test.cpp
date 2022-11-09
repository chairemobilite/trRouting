#include "gtest/gtest.h" // we will add the path to C preprocessor later
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
    TrRouting::CacheFetcher aCacheFetcher("");
    std::string filePath = aCacheFetcher.getFilePath(CACHE_FILE, "");
    EXPECT_STREQ(filePath.c_str(), CACHE_FILE.c_str());

    // Test with default params and custom path
    filePath = aCacheFetcher.getFilePath(CACHE_FILE, VALID_CUSTOM_PATH);
    EXPECT_STREQ(filePath.c_str(), (VALID_CUSTOM_PATH + '/' + CACHE_FILE).c_str());

    // Omit the project name in params
    TrRouting::CacheFetcher relCacheFetcher(relativeCacheDir);
    filePath = relCacheFetcher.getFilePath(CACHE_FILE, "");
    EXPECT_STREQ(filePath.c_str(), (relativeCacheDir + '/' + CACHE_FILE).c_str());

}

TEST_F(CacheFetcherFixtureTests, TestFileExists)
{
    // Test a file that exists
    std::string testFile = "testCache/validCacheFiles/agencies.capnpbin";
    ASSERT_TRUE(TrRouting::CacheFetcher::capnpCacheFileExists(testFile));

    // Test a file that does not exist
    testFile = "testCache/notexists.capnpbin";
    ASSERT_FALSE(TrRouting::CacheFetcher::capnpCacheFileExists(testFile));
}

TEST_F(CacheFetcherFixtureTests, TestGetFileCount)
{
    // Test count with a count file that does not exist
    std::string testFile = "testCache/notexists.capnpbin";
    ASSERT_EQ(1, TrRouting::CacheFetcher::getCacheFilesCount(testFile));

    // Test a file that contains a count
    testFile = "testCache/someCache.capnpbin.count";
    ASSERT_EQ(10, TrRouting::CacheFetcher::getCacheFilesCount(testFile));

    // Read a value from an invalid file.
    // TODO It returns 0, is this the right expected value?
    testFile = "testCache/invalidCacheFiles/genericInvalid.capnpbin";
    ASSERT_EQ(0, TrRouting::CacheFetcher::getCacheFilesCount(testFile));
}

