#ifndef _BENCHMARK_CSA_TEST_H
#define _BENCHMARK_CSA_TEST_H

#include "gtest/gtest.h"
#include "parameters.hpp"
#include "calculator.hpp"

using namespace TrRouting;

class ConstantBenchmarkCSATests : public ::testing::Test
{
protected:
    TrRouting::ServerParameters setupAlgorithmParams();
    bool updateCalculatorFromCache(TrRouting::Calculator *calculator);
    int assertCacheOk(TrRouting::Calculator *calculator);
    
    std::vector<std::string> createCalculationQuery();
    
    bool updateCalculatorParams(Calculator *calculator, std::vector<std::string> *parametersWithValues);
    
    void benchmarkCurrentParams(Calculator *calculator);
};

#endif