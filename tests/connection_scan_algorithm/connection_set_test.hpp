#ifndef _CONNECTION_SET_TEST_H
#define _CONNECTION_SET_TEST_H

#include <boost/uuid/string_generator.hpp>

#include "gtest/gtest.h"
#include "csa_test_data_fetcher.hpp"
#include "transit_data.hpp"


class ConnectionSetFixtureTests : public ::testing::Test
{

protected:
    // A DataFetcher is required to initialize the calculator
    TestDataFetcher dataFetcher;
    TrRouting::TransitData transitData;

public:
    ConnectionSetFixtureTests() : dataFetcher(TestDataFetcher()), transitData(TrRouting::TransitData(dataFetcher)) {}

};

#endif // _CONNECTION_SET_TEST_H
