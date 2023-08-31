#include <errno.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "gtest/gtest.h"
#include "calculator.hpp"
#include "csa_test_base.hpp"
#include "csa_v1_simple_calculation_test.hpp"
#include "constants.hpp"
#include "mode.hpp"
#include "data_source.hpp"
#include "household.hpp"
#include "person.hpp"
#include "place.hpp"
#include "agency.hpp"
#include "service.hpp"
#include "line.hpp"
#include "path.hpp"
#include "trip.hpp"
#include "od_trip.hpp"
#include "node.hpp"

/**
 * This file covers od trips use cases, where the value of od_trips is set to 1
 * */

// TODO:
// Do more than simple single use case, but at least we make sure the query path always works

class RouteOdTripsFixtureTests : public RouteCalculationFixtureTests
{
private:

public:
    // Return a parameters vector with the scenario and minimum waiting time, which is repetitive and mandatory
    std::vector<std::string> initializeParameters();
    // Helper method to set parameters and calculate OD trip result. Test cases need only provide parameters and validate the result
    nlohmann::json calculateOdTrips(std::vector<std::string> parameters);

};

// Simple test to make sure this code path still works
TEST_F(RouteOdTripsFixtureTests, NoOdTripsQuery)
{
    std::vector<std::string> parametersWithValues = initializeParameters();

    nlohmann::json result = calculateOdTrips(parametersWithValues);
    ASSERT_EQ(1u, result["odTrips"].size());
    ASSERT_EQ(STATUS_SUCCESS, result["odTrips"][0]["status"]);
}

std::vector<std::string> RouteOdTripsFixtureTests::initializeParameters()
{
    std::vector<std::string> parametersWithValues = RouteCalculationFixtureTests::initializeParameters();
    parametersWithValues.push_back("od_trips=1");
    return parametersWithValues;
}

nlohmann::json RouteOdTripsFixtureTests::calculateOdTrips(std::vector<std::string> parameters)
{
    calculator.params.setDefaultValues();
    TrRouting::RouteParameters routeParams = calculator.params.update(parameters,
                                                                      transitData.getScenarios(),
                                                                      transitData.getOdTrips(),
                                                                      transitData.getNodes(),
                                                                      transitData.getDataSources());

    // TODO Shouldn't need to do this, but we do for now, benchmark needs to be started
    calculator.algorithmCalculationTime.start();
    calculator.benchmarking.clear();

    std::string result =  calculator.odTripsRouting(routeParams);
    nlohmann::json json;
    nlohmann::json jsonResult = json.parse(result);
    return jsonResult;
}

//TODO Add a test that validate the shuffle of the odTrips list
