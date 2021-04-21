#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "parameters.hpp"
#include "cache_fetcher.hpp"

class CacheFetcherFixtureTests : public ::testing::Test {
protected:
    TrRouting::Parameters params;
    std::string cacheFile = "cacheFile.capnpbin";
    std::string customPath = "test";
    std::string relativeCacheDir = "cache";
    std::string projectName = "testCache";
};

TEST_F(CacheFetcherFixtureTests, TestGetFilePath)
{
    // Test with default params values
    std::string filePath = TrRouting::CacheFetcher::getFilePath(cacheFile, params, "");
    EXPECT_STREQ(filePath.c_str(), cacheFile.c_str());

    // Test with default params and custom path
    filePath = TrRouting::CacheFetcher::getFilePath(cacheFile, params, customPath);
    EXPECT_STREQ(filePath.c_str(), (customPath + '/' + cacheFile).c_str());

    // Omit the project name in params
    params.cacheDirectoryPath = relativeCacheDir;
    filePath = TrRouting::CacheFetcher::getFilePath(cacheFile, params, "");
    EXPECT_STREQ(filePath.c_str(), (relativeCacheDir + '/' + cacheFile).c_str());

    // Put all parameters
    params.projectShortname = projectName;
    filePath = TrRouting::CacheFetcher::getFilePath(cacheFile, params, "");
    EXPECT_STREQ(filePath.c_str(), (relativeCacheDir + '/' + projectName + '/' + cacheFile).c_str());

    // Omit the cache directory path
    params.cacheDirectoryPath = "";
    filePath = TrRouting::CacheFetcher::getFilePath(cacheFile, params, "");
    EXPECT_STREQ(filePath.c_str(), (projectName + '/' + cacheFile).c_str());
}

TEST_F(CacheFetcherFixtureTests, TestFileExists)
{
    // Test a file that exists
    std::string cacheFile = "testCache/invalidCacheFiles/agencies.capnpbin";
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
    cacheFile = "testCache/invalidCacheFiles/agencies.capnpbin";
    ASSERT_EQ(0, TrRouting::CacheFetcher::getCacheFilesCount(cacheFile));
}

