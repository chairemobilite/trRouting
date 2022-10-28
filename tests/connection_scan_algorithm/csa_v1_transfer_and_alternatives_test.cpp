#include <errno.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "gtest/gtest.h"
#include "calculator.hpp"
#include "csa_test_base.hpp"
#include "csa_v1_simple_calculation_test.hpp"
#include "constants.hpp"
#include "node.hpp"
#include "scenario.hpp"
#include "agency.hpp"
#include "line.hpp"
#include "service.hpp"
#include "path.hpp"
#include "point.hpp"
#include "mode.hpp"
#include "trip.hpp"
#include "data_source.hpp"
#include "household.hpp"
#include "person.hpp"
#include "place.hpp"
#include "od_trip.hpp"
#include "osrm_fetcher.hpp"

/**
 * This file covers tests with transfers and requests for alternatives
 * */

class TAndACalculationFixtureTests : public RouteCalculationFixtureTests
{

public:
    // Helper method to set parameters and calculate OD with alternatives. Test cases need only provide parameters and validate the result
    TrRouting::AlternativesResult calculateWithAlternatives(std::vector<std::string> parameters);
};

// Test from OD which includes a transfer to the same node, origin is further
// south of South2, in the line axis, destination is very close to west2
// Use default parameters for non-mandatory ones
TEST_F(TAndACalculationFixtureTests, TripWithTransfer)
{
    int departureTime = getTimeInSeconds(9, 45);
    int minWaitingTime = 90;
    int travelTimeInVehicle = 720;
    // This is where mocking would be interesting. Those were taken from the first run of the test
    int accessTime = 469;
    int egressTime = 76;
    int expectedTransitDepartureTime = getTimeInSeconds(10);
    int expectedTransferWaitingTime = 120;

    std::vector<std::string> parametersWithValues = initializeParameters();
    parametersWithValues.push_back("origin=45.5242,-73.5817");
    parametersWithValues.push_back("destination=45.5295,-73.624");
    parametersWithValues.push_back("departure_time_seconds=" + std::to_string(departureTime));
    parametersWithValues.push_back("min_waiting_time_seconds=" + std::to_string(minWaitingTime));

    std::unique_ptr<TrRouting::RoutingResult> result = calculateOd(parametersWithValues);
    assertSuccessResults(*result.get(),
        departureTime,
        expectedTransitDepartureTime,
        travelTimeInVehicle,
        accessTime,
        egressTime,
        1,
        minWaitingTime,
        minWaitingTime + expectedTransferWaitingTime,
        expectedTransferWaitingTime,
        0);
}

// Same as TripWithTransfer but with a minimal transfer time higher than
// available, the result is a walk from midpoint instead of another bus trip
TEST_F(TAndACalculationFixtureTests, NoTransferMinTransferTime)
{
    int departureTime = getTimeInSeconds(9, 45);
    int minWaitingTime = 240;
    int travelTimeInVehicle = 420;
    int accessTime = 469;
    int egressTime = 884;
    int expectedTransitDepartureTime = getTimeInSeconds(10);

    std::vector<std::string> parametersWithValues = initializeParameters();
    parametersWithValues.push_back("origin=45.5242,-73.5817");
    parametersWithValues.push_back("destination=45.5295,-73.624");
    parametersWithValues.push_back("departure_time_seconds=" + std::to_string(departureTime));
    parametersWithValues.push_back("min_waiting_time_seconds=" + std::to_string(minWaitingTime));

    std::unique_ptr<TrRouting::RoutingResult> result = calculateOd(parametersWithValues);
    assertSuccessResults(*result.get(),
        departureTime,
        expectedTransitDepartureTime,
        travelTimeInVehicle,
        accessTime,
        egressTime,
        0,
        minWaitingTime,
        minWaitingTime);
}

// Test from OD which includes a walking transfer to another node, using the
// extra line, origin is further south of South2, in the line axis, destination
// is close to extra1
TEST_F(TAndACalculationFixtureTests, TripWithWalkingTransfer)
{
    int departureTime = getTimeInSeconds(9, 45);
    int travelTimeInVehicle = 720;
    // This is where mocking would be interesting. Those were taken from the first run of the test
    int accessTime = 469;
    int egressTime = 166;
    int expectedTransitDepartureTime = getTimeInSeconds(10);
    int expectedTransferWaitingTime = 300;
    int expectedTransferWalkingTime = 480;

    std::vector<std::string> parametersWithValues = initializeParameters();
    parametersWithValues.push_back("origin=45.5242,-73.5817");
    parametersWithValues.push_back("destination=45.5549,-73.6173");
    parametersWithValues.push_back("departure_time_seconds=" + std::to_string(departureTime));

    std::unique_ptr<TrRouting::RoutingResult> result = calculateOd(parametersWithValues);
    assertSuccessResults(*result.get(),
        departureTime,
        expectedTransitDepartureTime,
        travelTimeInVehicle,
        accessTime,
        egressTime,
        1,
        MIN_WAITING_TIME,
        MIN_WAITING_TIME + expectedTransferWaitingTime,
        expectedTransferWaitingTime,
        expectedTransferWalkingTime);
}

// Test a trip with alternatives, one taking SN path and walking, the other
// taking the extra path. Origin is further south of South2, in the line axis,
// destination near extra1, but still reachable by walk from north1
TEST_F(TAndACalculationFixtureTests, TripWithAlternatives)
{
    int departureTime = getTimeInSeconds(9, 45);
    int travelTimeInVehicle = 720;
    // This is where mocking would be interesting. Those were taken from the first run of the test
    int accessTime = 469;
    int egressTime = 166;
    int expectedTransitDepartureTime = getTimeInSeconds(10);
    int expectedTransferWaitingTime = 300;

    std::vector<std::string> parametersWithValues = initializeParameters();
    parametersWithValues.push_back("origin=45.5242,-73.5817");
    parametersWithValues.push_back("destination=45.5541,-73.6186");
    parametersWithValues.push_back("departure_time_seconds=" + std::to_string(departureTime));
    parametersWithValues.push_back("alternatives=1");

    TrRouting::AlternativesResult result = calculateWithAlternatives(parametersWithValues);
    ASSERT_EQ(2, result.alternatives.size());
}

// Test a query with alternatives, for a trip with no routing found
TEST_F(TAndACalculationFixtureTests, TripWithNoRoutingAlternatives)
{
    int departureTime = getTimeInSeconds(9, 45);
    int travelTimeInVehicle = 720;
    // This is where mocking would be interesting. Those were taken from the first run of the test
    int accessTime = 469;
    int egressTime = 166;
    int expectedTransitDepartureTime = getTimeInSeconds(10);
    int expectedTransferWaitingTime = 300;

    std::vector<std::string> parametersWithValues = initializeParameters();
    parametersWithValues.push_back("origin=45.7242,-73.7817");
    parametersWithValues.push_back("destination=45.7541,-73.8186");
    parametersWithValues.push_back("departure_time_seconds=" + std::to_string(departureTime));
    parametersWithValues.push_back("alternatives=1");

    try {
        calculateWithAlternatives(parametersWithValues);
        FAIL() << "Expected TrRouting::NoRoutingFoundException, no exception thrown";
    } catch (TrRouting::NoRoutingFoundException const & e) {
        assertNoRouting(e, TrRouting::NoRoutingReason::NO_ACCESS_AT_ORIGIN_AND_DESTINATION);
    } catch(...) {
        FAIL() << "Expected TrRouting::NoRoutingFoundException, another type was thrown";
    }
}

TrRouting::AlternativesResult TAndACalculationFixtureTests::calculateWithAlternatives(std::vector<std::string> parameters)
{
    calculator.params.setDefaultValues();
    TrRouting::RouteParameters routeParams = calculator.params.update(parameters,
        calculator.scenarios,
        calculator.odTripIndexesByUuid,
        calculator.odTrips,
        calculator.nodes,
        calculator.dataSources);
    TrRouting::OsrmFetcher::birdDistanceAccessibilityEnabled = true;

    // TODO Shouldn't need to do this, but we do for now, benchmark needs to be started
    calculator.algorithmCalculationTime.start();
    calculator.benchmarking.clear();

    return calculator.alternativesRouting(routeParams);
}
