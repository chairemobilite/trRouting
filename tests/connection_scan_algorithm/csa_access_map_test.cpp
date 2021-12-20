#include <errno.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "gtest/gtest.h"
#include "calculator.hpp"
#include "csa_test_base.hpp"
#include "csa_simple_calculation_test.hpp"
#include "constants.hpp"

/**
 * This file covers accessibility map use cases, where the value of all_nodes is
 * set to 1
 * */

// TODO:
// Do more than simple single use case, but at least we make sure the query path always works

class RouteAccessMapFixtureTests : public RouteCalculationFixtureTests
{

public:
    // Asserts the successful result fields, given some easy to provide expected test data
    void assertResults(TrRouting::RoutingResult result,
        int nbReachableNodes);
    // Return a parameters vector with the scenario and minimum waiting time, which is repetitive and mandatory
    std::vector<std::string> initializeParameters();
};

// Test from an origin near the south2 node
TEST_F(RouteAccessMapFixtureTests, SimpleAllNodesQuery)
{
    int departureTime = getTimeInSeconds(9, 45);
    int travelTimeInVehicle = 420;
    // This is where mocking would be interesting. Those were taken from the first run of the test
    int accessTime = 469;
    int egressTime = 138;
    int expectedTransitDepartureTime = getTimeInSeconds(10);

    std::vector<std::string> parametersWithValues = initializeParameters();
    parametersWithValues.push_back("origin=45.5242,-73.5817");
    parametersWithValues.push_back("departure_time_seconds=" + std::to_string(departureTime));

    TrRouting::RoutingResult result = calculateOd(parametersWithValues);
    assertResults(result, 5);

}

void RouteAccessMapFixtureTests::assertResults(TrRouting::RoutingResult result,
    int nbReachableNodes)
{
    ASSERT_EQ(STATUS_SUCCESS, result.json["status"]);
    ASSERT_EQ(nbReachableNodes, result.json["numberOfReachableNodes"]);
}

std::vector<std::string> RouteAccessMapFixtureTests::initializeParameters() 
{
    std::vector<std::string> parametersWithValues = RouteCalculationFixtureTests::initializeParameters();
    parametersWithValues.push_back("all_nodes=1");
    return parametersWithValues;
}
