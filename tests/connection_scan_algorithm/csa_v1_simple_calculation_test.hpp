#ifndef _CSA_SIMPLE_CALC_H
#define _CSA_SIMPLE_CALC_H

#include "gtest/gtest.h"
#include "calculator.hpp"
#include "parameters.hpp"
#include "csa_test_base.hpp"
#include "csa_route_calculation_test.hpp"

// This fixture tests the parameters from the legacy parameters update method
class RouteCalculationFixtureTests : public BaseCsaFixtureTests
{

public:
    // Helper method to set parameters and calculate OD trip result. Test cases need only provide parameters and validate the result
    std::unique_ptr<TrRouting::RoutingResult> calculateOd(std::vector<std::string> parameters);
    // Return a parameters vector with the scenario and minimum waiting time, which is repetitive and mandatory
    std::vector<std::string> initializeParameters();
};

#endif // _CSA_SIMPLE_CALC_H