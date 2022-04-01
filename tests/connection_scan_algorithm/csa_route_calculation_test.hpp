#ifndef _CSA_ROUTE_CALC_H
#define _CSA_ROUTE_CALC_H

#include "gtest/gtest.h"
#include "calculator.hpp"
#include "parameters.hpp"
#include "csa_test_base.hpp"
#include "scenario.hpp"
#include "toolbox.hpp" //MAX_INT

// This fixture tests the parameters from the single route parameters values
class SingleRouteCalculationFixtureTests : public BaseCsaFixtureTests
{
protected:
    // Default value for various test parameters
    static const int DEFAULT_MIN_WAITING_TIME = 3 * 60;
    static const int DEFAULT_MAX_TOTAL_TIME = TrRouting::MAX_INT;
    static const int DEFAULT_MAX_ACCESS_TRAVEL_TIME = 20 * 60;
    static const int DEFAULT_MAX_EGRESS_TRAVEL_TIME = 20 * 60;
    static const int DEFAULT_MAX_TRANSFER_TRAVEL_TIME = 20 * 60;
    static const int DEFAULT_FIRST_WAITING_TIME = 30 * 60;

    TrRouting::Scenario *scenario;

public:
    // Helper method to set parameters and calculate OD trip result. Test cases need only provide parameters and validate the result
    std::unique_ptr<TrRouting::RoutingResult> calculateOd(TrRouting::RouteParameters& parameters);
    void SetUp();
};

#endif // _CSA_ROUTE_CALC_H
