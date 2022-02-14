#include <errno.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "gtest/gtest.h"
#include "calculator.hpp"
#include "csa_test_base.hpp"
#include "csa_route_calculation_test.hpp"
#include "constants.hpp"

/**
 * This file covers tests with transfers and requests for alternatives
 * */

class SingleTAndACalculationFixtureTests : public SingleRouteCalculationFixtureTests
{

public:
    // Helper method to set parameters and calculate OD with alternatives. Test cases need only provide parameters and validate the result
    nlohmann::json calculateWithAlternatives(TrRouting::RouteParameters& parameters);
};

// Test from OD which includes a transfer to the same node, origin is further
// south of South2, in the line axis, destination is very close to west2
// Use default parameters for non-mandatory ones
TEST_F(SingleTAndACalculationFixtureTests, TripWithTransfer)
{
    int departureTime = getTimeInSeconds(9, 45);
    int minWaitingTime = 90;
    int travelTimeInVehicle = 720;
    // This is where mocking would be interesting. Those were taken from the first run of the test
    int accessTime = 469;
    int egressTime = 76;
    int expectedTransitDepartureTime = getTimeInSeconds(10);
    int expectedTransferWaitingTime = 120;

    TrRouting::RouteParameters testParameters = TrRouting::RouteParameters(
        std::make_unique<TrRouting::Point>(45.5242, -73.5817),
        std::make_unique<TrRouting::Point>(45.5295, -73.624),
        *scenario,
        departureTime,
        minWaitingTime,
        DEFAULT_MAX_TOTAL_TIME,
        DEFAULT_MAX_ACCESS_TRAVEL_TIME,
        DEFAULT_MAX_EGRESS_TRAVEL_TIME,
        DEFAULT_MAX_TRANSFER_TRAVEL_TIME,
        DEFAULT_FIRST_WAITING_TIME,
        false,
        true
    );

    TrRouting::RoutingResult result = calculateOd(testParameters);
    assertSuccessResults(result,
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
TEST_F(SingleTAndACalculationFixtureTests, NoTransferMinTransferTime)
{
    int departureTime = getTimeInSeconds(9, 45);
    int minWaitingTime = 240;
    int travelTimeInVehicle = 420;
    int accessTime = 469;
    int egressTime = 884;
    int expectedTransitDepartureTime = getTimeInSeconds(10);

    TrRouting::RouteParameters testParameters = TrRouting::RouteParameters(
        std::make_unique<TrRouting::Point>(45.5242, -73.5817),
        std::make_unique<TrRouting::Point>(45.5295, -73.624),
        *scenario,
        departureTime,
        minWaitingTime,
        DEFAULT_MAX_TOTAL_TIME,
        DEFAULT_MAX_ACCESS_TRAVEL_TIME,
        DEFAULT_MAX_EGRESS_TRAVEL_TIME,
        DEFAULT_MAX_TRANSFER_TRAVEL_TIME,
        DEFAULT_FIRST_WAITING_TIME,
        false,
        true
    );

    TrRouting::RoutingResult result = calculateOd(testParameters);
    assertSuccessResults(result,
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
TEST_F(SingleTAndACalculationFixtureTests, TripWithWalkingTransfer)
{
    int departureTime = getTimeInSeconds(9, 45);
    int travelTimeInVehicle = 720;
    // This is where mocking would be interesting. Those were taken from the first run of the test
    int accessTime = 469;
    int egressTime = 166;
    int expectedTransitDepartureTime = getTimeInSeconds(10);
    int expectedTransferWaitingTime = 300;
    int expectedTransferWalkingTime = 480;

    TrRouting::RouteParameters testParameters = TrRouting::RouteParameters(
        std::make_unique<TrRouting::Point>(45.5242, -73.5817),
        std::make_unique<TrRouting::Point>(45.5549, -73.6173),
        *scenario,
        departureTime,
        DEFAULT_MIN_WAITING_TIME,
        DEFAULT_MAX_TOTAL_TIME,
        DEFAULT_MAX_ACCESS_TRAVEL_TIME,
        DEFAULT_MAX_EGRESS_TRAVEL_TIME,
        DEFAULT_MAX_TRANSFER_TRAVEL_TIME,
        DEFAULT_FIRST_WAITING_TIME,
        false,
        true
    );

    TrRouting::RoutingResult result = calculateOd(testParameters);
    assertSuccessResults(result,
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
TEST_F(SingleTAndACalculationFixtureTests, TripWithAlternatives)
{
    int departureTime = getTimeInSeconds(9, 45);
    int travelTimeInVehicle = 720;
    // This is where mocking would be interesting. Those were taken from the first run of the test
    int accessTime = 469;
    int egressTime = 166;
    int expectedTransitDepartureTime = getTimeInSeconds(10);
    int expectedTransferWaitingTime = 300;

    TrRouting::RouteParameters testParameters = TrRouting::RouteParameters(
        std::make_unique<TrRouting::Point>(45.5242, -73.5817),
        std::make_unique<TrRouting::Point>(45.5541, -73.6186),
        *scenario,
        departureTime,
        DEFAULT_MIN_WAITING_TIME,
        DEFAULT_MAX_TOTAL_TIME,
        DEFAULT_MAX_ACCESS_TRAVEL_TIME,
        DEFAULT_MAX_EGRESS_TRAVEL_TIME,
        DEFAULT_MAX_TRANSFER_TRAVEL_TIME,
        DEFAULT_FIRST_WAITING_TIME,
        true,
        true
    );

    nlohmann::json result = calculateWithAlternatives(testParameters);
    ASSERT_EQ(STATUS_SUCCESS, result["status"]);
    ASSERT_EQ(2, result["alternatives"].size());
}

nlohmann::json SingleTAndACalculationFixtureTests::calculateWithAlternatives(TrRouting::RouteParameters& parameters)
{
    // TODO: This needs to be called to set some default values that are still part of the global parameters
    calculator.params.setDefaultValues();
    calculator.params.birdDistanceAccessibilityEnabled = true;

    // TODO Shouldn't need to do this, but we do for now, benchmark needs to be started
    calculator.algorithmCalculationTime.start();
    calculator.benchmarking.clear();

    std::string result = calculator.alternativesRouting(parameters);
    nlohmann::json json;
    nlohmann::json jsonResult = json.parse(result);
    return jsonResult;

}
