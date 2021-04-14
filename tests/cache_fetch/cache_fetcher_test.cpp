#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "parameters.hpp"
#include "cache_fetcher.hpp"

class CacheFetcherFixtureTests : public ::testing::Test {
protected:
    TrRouting::Parameters params;
    std::string cacheFile = "cacheFile.capnp";
    std::string customPath = "test";
    std::string relativeCacheDir = "cache";
    std::string projectName = "test";
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
