#ifndef _CACHE_FETCHER_TEST_H
#define _CACHE_FETCHER_TEST_H

#include "gtest/gtest.h"
#include "parameters.hpp"

using namespace TrRouting;

class ConstantAlternativeRoutingTests : public ::testing::Test 
{
protected:
    int setup();

    int evaluateRequests();
    int evaluateRequestsWithParallelization();
};

#endif 