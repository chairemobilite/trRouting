#ifndef _BENCHMARK_CSA_TEST_H
#define _BENCHMARK_CSA_TEST_H

#include "gtest/gtest.h"
#include "parameters.hpp"
#include "calculator.hpp"

using namespace TrRouting;
using BenchmarkDataTuple = std::tuple<std::string, double, double, double, double, int, bool>;

class ConstantBenchmarkCSATests : public ::testing::TestWithParam<BenchmarkDataTuple>
{
protected:
    enum parameterIndexes : short { TEST_DESCRIPTION = 0, LON_ORIG, LAT_ORIG, LON_DEST, LAT_DEST, TIME, EXPECT_RESULTS };

public:

};

#endif