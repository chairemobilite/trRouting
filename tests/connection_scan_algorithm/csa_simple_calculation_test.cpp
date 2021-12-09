#include <errno.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "gtest/gtest.h"
#include "calculator.hpp"
#include "csa_test_base.hpp"
#include "constants.hpp"


class RouteCalculationFixtureTests : public BaseCsaFixtureTests
{

    
};

// One single test for now, but it will be replaced by parameterized tests
TEST_F(RouteCalculationFixtureTests, OneLineCalculation)
{
    std::vector<std::string> parametersWithValues;
    parametersWithValues.push_back("scenario_uuid=" + boost::uuids::to_string(scenarioUuid));
    // Origin and destination are the coordinates of first and second node of SouthNorth path
    parametersWithValues.push_back("origin=45.5269,-73.58912");
    parametersWithValues.push_back("destination=45.53258,-73.60196");
    parametersWithValues.push_back("departure_time_seconds=" + std::to_string(getTimeInSeconds(9, 50)));
    calculator.params.setDefaultValues();
    calculator.params.update(parametersWithValues, calculator.scenarioIndexesByUuid, calculator.scenarios, calculator.nodeIndexesByUuid, calculator.agencyIndexesByUuid, calculator.lineIndexesByUuid, calculator.serviceIndexesByUuid, calculator.modeIndexesByShortname, calculator.dataSourceIndexesByUuid);
    calculator.params.birdDistanceAccessibilityEnabled = true; 

    // TODO Shouldn't need to do this, but we do for now, benchmark needs to be started
    calculator.algorithmCalculationTime.start();
    calculator.benchmarking.clear();

    ASSERT_EQ(9, calculator.nodes.size());
    ASSERT_EQ(2, calculator.lines.size());
    // TODO shouldn't have to do this...
    calculator.origin = &calculator.params.origin;
    calculator.destination = &calculator.params.destination;
    TrRouting::RoutingResult result = calculator.calculate();
    ASSERT_EQ(STATUS_SUCCESS, result.status);
    std::cout << "Running a first test" << std::endl;
}
