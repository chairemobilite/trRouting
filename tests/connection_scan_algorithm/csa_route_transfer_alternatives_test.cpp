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
    TrRouting::AlternativesResult calculateWithAlternatives(TrRouting::RouteParameters& parameters);
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

    std::unique_ptr<TrRouting::RoutingResult> result = calculateOd(testParameters);
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

    std::unique_ptr<TrRouting::RoutingResult> result = calculateOd(testParameters);
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

    std::unique_ptr<TrRouting::RoutingResult> result = calculateOd(testParameters);
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
TEST_F(SingleTAndACalculationFixtureTests, TripWithAlternatives)
{
    int departureTime = getTimeInSeconds(9, 45);
    // This is where mocking would be interesting. Those were taken from the first run of the test
    int accessTime = 469;
    int expectedTransitDepartureTime = getTimeInSeconds(10);

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

    TrRouting::AlternativesResult routingResult = calculateWithAlternatives(testParameters);
    ASSERT_EQ(2, routingResult.alternatives.size());
    // TODO: Why is it 5? Would it be a sign of an error? To investigate
    ASSERT_EQ(5, routingResult.totalAlternativesCalculated);

    // First trip gets off at midpoint, walks to east 1 and takes extra line and walks from extra node to destination
    int egressTimeAlt1 = 77;
    int expTransferWaitingTime1 = 300;
    int expTransferWalkingTime = 480;
    TrRouting::SingleCalculationResult& alternativeResult1 = dynamic_cast<TrRouting::SingleCalculationResult&>(*routingResult.alternatives[0].get());
    assertSuccessResults(alternativeResult1,
        departureTime,
        expectedTransitDepartureTime,
        12 * 60,
        accessTime,
        egressTimeAlt1,
        1,
        MIN_WAITING_TIME,
        MIN_WAITING_TIME + expTransferWaitingTime1,
        expTransferWaitingTime1,
        expTransferWalkingTime
    );

    // Second trip takes SN line to North1 and walks to destination
    int egressTimeAlt2 = 1080;
    assertSuccessResults(*routingResult.alternatives[1].get(),
        departureTime,
        expectedTransitDepartureTime,
        13 * 60,
        accessTime,
        egressTimeAlt2,
        0,
        MIN_WAITING_TIME,
        MIN_WAITING_TIME,
        0,
        0
    );
}

TrRouting::AlternativesResult SingleTAndACalculationFixtureTests::calculateWithAlternatives(TrRouting::RouteParameters& parameters)
{
    // TODO: This needs to be called to set some default values that are still part of the global parameters
    calculator.params.setDefaultValues();
    calculator.params.birdDistanceAccessibilityEnabled = true;

    // TODO Shouldn't need to do this, but we do for now, benchmark needs to be started
    calculator.algorithmCalculationTime.start();
    calculator.benchmarking.clear();

    return calculator.alternativesRouting(parameters);

}
