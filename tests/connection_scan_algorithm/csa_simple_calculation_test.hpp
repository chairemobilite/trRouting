#ifndef _CSA_SIMPLE_CALC_H
#define _CSA_SIMPLE_CALC_H

#include "gtest/gtest.h"
#include "calculator.hpp"
#include "parameters.hpp"
#include "csa_test_base.hpp"

const int MIN_WAITING_TIME{180};

class RouteCalculationFixtureTests : public BaseCsaFixtureTests
{

public:
    // Helper method to set parameters and calculate OD trip result. Test cases need only provide parameters and validate the result
    TrRouting::RoutingResult calculateOd(std::vector<std::string> parameters);
    // Asserts the result status is no routing. If we add reasons, this method can eventually be updated and all test cases will need to be updated.
    void assertNoRouting(TrRouting::RoutingResult result);
    // Asserts the successful result fields, given some easy to provide expected test data
    void assertSuccessResults(TrRouting::RoutingResult result,
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
    // Return a parameters vector with the scenario and minimum waiting time, which is repetitive and mandatory
    std::vector<std::string> initializeParameters();
};

#endif // _CSA_SIMPLE_CALC_H