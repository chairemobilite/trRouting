#ifndef _CACHE_FETCHER_TEST_H
#define _CACHE_FETCHER_TEST_H

#include "gtest/gtest.h"
#include "parameters.hpp"
#include "cache_fetcher.hpp"

class ConstantCacheFetcherFixtureTests : public ::testing::Test 
{
protected:
    TrRouting::ServerParameters params;
    std::string cacheFile = "cacheFile.capnpbin";
    const std::string INVALID_CUSTOM_PATH = "invalidCacheFiles";
    const std::string VALID_CUSTOM_PATH = "validCacheFiles";
    const std::string BASE_CUSTOM_PATH = "";
    const std::string PROJECT_NAME = "testCache";
};

class BaseCacheFetcherFixtureTests : public ConstantCacheFetcherFixtureTests 
{
protected:
    TrRouting::CacheFetcher cacheFetcher = TrRouting::CacheFetcher();

public:
    void SetUp() 
    {
        params.projectShortname = PROJECT_NAME; 
    }
};

#endif // _CACHE_FETCHER_TEST_H