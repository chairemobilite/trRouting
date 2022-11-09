#ifndef _CSA_TEST_H
#define _CSA_TEST_H

#include <boost/uuid/string_generator.hpp>

#include "gtest/gtest.h"
#include "csa_test_data_fetcher.hpp"
#include "transit_data.hpp"
#include "calculator.hpp"
#include "routing_result.hpp"


const int MIN_WAITING_TIME{180};

class BaseCsaFixtureTests : public ::testing::Test
{

protected:
    // A DataFetcher is required to initialize the calculator
    TestDataFetcher dataFetcher;
    TrRouting::TransitData transitData;
    // Calculator is the entry point to run the algorithm, it is part of the test object since (for now) there is nothing specific for its initialization.
    TrRouting::Calculator calculator;

public:
    BaseCsaFixtureTests() : dataFetcher(TestDataFetcher()), transitData(TrRouting::TransitData(dataFetcher)), calculator(TrRouting::Calculator(transitData)) {}
    void SetUp();
    // Assert the result returned an exception and that the reason matches the expected reason
    void assertNoRouting(const TrRouting::NoRoutingFoundException& exception, TrRouting::NoRoutingReason expectedReason);
    // Asserts the successful result fields, given some easy to provide expected test data
    void assertSuccessResults(TrRouting::RoutingResult& result,
        int origDepartureTime,
        int expTransitDepartureTime,
        int expInVehicleTravelTime,
        int expAccessTime = 0,
        int expEgressTime = 0,
        int expNbTransfers = 0,
        int minWaitingTime = MIN_WAITING_TIME,
        int expTotalWaitingTime = MIN_WAITING_TIME,
        int expTransferWaitingTime = 0,
        int expTransferTravelTime = 0);

};

#endif
