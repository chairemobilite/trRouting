#ifndef _CACHE_FETCHER_TEST_H
#define _CACHE_FETCHER_TEST_H

#include "gtest/gtest.h"
#include "cache_fetcher.hpp"

class ConstantCacheFetcherFixtureTests : public ::testing::Test 
{
protected:
    std::string cacheFile = "cacheFile.capnpbin";
    const std::string INVALID_CUSTOM_PATH = "invalidCacheFiles";
    const std::string VALID_CUSTOM_PATH = "validCacheFiles";
    const std::string BASE_CUSTOM_PATH = "";
    const std::string BASE_CACHE_DIRECTORY_NAME = "testCache";
};

class BaseCacheFetcherFixtureTests : public ConstantCacheFetcherFixtureTests 
{
protected:
    TrRouting::CacheFetcher cacheFetcher = TrRouting::CacheFetcher(BASE_CACHE_DIRECTORY_NAME);

public:
    void SetUp() 
    {

    }
};

#endif // _CACHE_FETCHER_TEST_H
