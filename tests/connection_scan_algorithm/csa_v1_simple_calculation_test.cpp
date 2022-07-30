#include <errno.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "gtest/gtest.h"
#include "calculator.hpp"
#include "csa_test_base.hpp"
#include "csa_v1_simple_calculation_test.hpp"
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

// TODO:
// Test transferable mode, it has separate code path
// Test optimization cases

/**
 * This file covers use cases  of tests without alternatives or transfer,
 * testing the values of the various parameters
 * */

// Test from south node to the opposite direction
TEST_F(RouteCalculationFixtureTests, NoRoutingBecauseNoPath)
{
    std::vector<std::string> parametersWithValues = initializeParameters();
    parametersWithValues.push_back("origin=45.5269,-73.58912");
    parametersWithValues.push_back("destination=45.52184,-73.57817");
    parametersWithValues.push_back("departure_time_seconds=" + std::to_string(getTimeInSeconds(9, 50)));

    try {
        calculateOd(parametersWithValues);
        FAIL() << "Expected TrRouting::NoRoutingFoundException, no exception thrown";
    } catch (TrRouting::NoRoutingFoundException const & e) {
        assertNoRouting(e, TrRouting::NoRoutingReason::NO_ROUTING_FOUND);
    } catch(...) {
        FAIL() << "Expected TrRouting::NoRoutingFoundException, another type was thrown";
    }
}

// Test origin far from network
TEST_F(RouteCalculationFixtureTests, NoRoutingBecauseNoNodeAtOrigin)
{
    std::vector<std::string> parametersWithValues = initializeParameters();
    parametersWithValues.push_back("origin=45.5349,-73.55478");
    parametersWithValues.push_back("destination=45.54,-73.6146");
    parametersWithValues.push_back("departure_time_seconds=" + std::to_string(getTimeInSeconds(9, 50)));

    try {
        calculateOd(parametersWithValues);
        FAIL() << "Expected TrRouting::NoRoutingFoundException, no exception thrown";
    } catch (TrRouting::NoRoutingFoundException const & e) {
        assertNoRouting(e, TrRouting::NoRoutingReason::NO_ACCESS_AT_ORIGIN);
    } catch(...) {
        FAIL() << "Expected TrRouting::NoRoutingFoundException, another type was thrown";
    }
}

// Test destination far from network
TEST_F(RouteCalculationFixtureTests, NoRoutingBecauseNoNodeAtDestination)
{
    std::vector<std::string> parametersWithValues = initializeParameters();
    parametersWithValues.push_back("origin=45.5242,-73.5817");
    parametersWithValues.push_back("destination=45.5155,-73.56797");
    parametersWithValues.push_back("departure_time_seconds=" + std::to_string(getTimeInSeconds(9, 50)));

    try {
        calculateOd(parametersWithValues);
        FAIL() << "Expected TrRouting::NoRoutingFoundException, no exception thrown";
    } catch (TrRouting::NoRoutingFoundException const & e) {
        assertNoRouting(e, TrRouting::NoRoutingReason::NO_ACCESS_AT_DESTINATION);
    } catch(...) {
        FAIL() << "Expected TrRouting::NoRoutingFoundException, another type was thrown";
    }
}

// Test from first to second node of SN path, but before the time of the trip (6:50)
TEST_F(RouteCalculationFixtureTests, NoRoutingBecauseTooEarly)
{
    std::vector<std::string> parametersWithValues = initializeParameters();
    parametersWithValues.push_back("origin=45.5269,-73.58912");
    parametersWithValues.push_back("destination=45.53258,-73.60196");
    parametersWithValues.push_back("departure_time_seconds=" + std::to_string(getTimeInSeconds(6, 50)));

    try {
        calculateOd(parametersWithValues);
        FAIL() << "Expected TrRouting::NoRoutingFoundException, no exception thrown";
    } catch (TrRouting::NoRoutingFoundException const & e) {
        assertNoRouting(e, TrRouting::NoRoutingReason::NO_SERVICE_FROM_ORIGIN);
    } catch(...) {
        FAIL() << "Expected TrRouting::NoRoutingFoundException, another type was thrown";
    }
}

// Test from first to second node of SN path, but before the time of the trip (6:50) with reverse journey
TEST_F(RouteCalculationFixtureTests, NoRoutingBecauseTooEarlyArrival)
{
    std::vector<std::string> parametersWithValues = initializeParameters();
    parametersWithValues.push_back("origin=45.5269,-73.58912");
    parametersWithValues.push_back("destination=45.53258,-73.60196");
    parametersWithValues.push_back("arrival_time_seconds=" + std::to_string(getTimeInSeconds(6, 50)));

    try {
        calculateOd(parametersWithValues);
        FAIL() << "Expected TrRouting::NoRoutingFoundException, no exception thrown";
    } catch (TrRouting::NoRoutingFoundException const & e) {
        assertNoRouting(e, TrRouting::NoRoutingReason::NO_SERVICE_TO_DESTINATION);
    } catch(...) {
        FAIL() << "Expected TrRouting::NoRoutingFoundException, another type was thrown";
    }
}

// Test from first to second node of SN path
TEST_F(RouteCalculationFixtureTests, NodeToNodeCalculation)
{
    int departureTime = getTimeInSeconds(9, 50);
    int travelTimeInVehicle = 210;
    int expectedTransitDepartureTime = getTimeInSeconds(10);

    std::vector<std::string> parametersWithValues = initializeParameters();
    parametersWithValues.push_back("origin=45.5269,-73.58912");
    parametersWithValues.push_back("destination=45.53258,-73.60196");
    parametersWithValues.push_back("departure_time_seconds=" + std::to_string(departureTime));

    std::unique_ptr<TrRouting::RoutingResult> result = calculateOd(parametersWithValues);
    assertSuccessResults(*result.get(),
        departureTime,
        expectedTransitDepartureTime,
        travelTimeInVehicle);
}

// Test from OD With access/egress, origin is further south of South2, in the line axis, destination slightly north-west of midpoint
// Use default parameters for non-mandatory ones
TEST_F(RouteCalculationFixtureTests, SimpleODCalculationDepartureTime)
{
    int departureTime = getTimeInSeconds(9, 45);
    int travelTimeInVehicle = 420;
    // This is where mocking would be interesting. Those were taken from the first run of the test
    int accessTime = 469;
    int egressTime = 138;
    int expectedTransitDepartureTime = getTimeInSeconds(10);

    std::vector<std::string> parametersWithValues = initializeParameters();
    parametersWithValues.push_back("origin=45.5242,-73.5817");
    parametersWithValues.push_back("destination=45.54,-73.6146");
    parametersWithValues.push_back("departure_time_seconds=" + std::to_string(departureTime));

    std::unique_ptr<TrRouting::RoutingResult> result = calculateOd(parametersWithValues);
    assertSuccessResults(*result.get(),
        departureTime,
        expectedTransitDepartureTime,
        travelTimeInVehicle,
        accessTime,
        egressTime);
}

// Same as SimpleODCalculationDepartureTime, but with an arrival time
TEST_F(RouteCalculationFixtureTests, SimpleODCalculationArrivalTime)
{
    int arrivalTime = getTimeInSeconds(11, 15);
    int travelTimeInVehicle = 420;
    // This is where mocking would be interesting. Those were taken from the first run of the test
    int accessTime = 469;
    int egressTime = 138;
    int expectedTransitDepartureTime = getTimeInSeconds(11);

    std::vector<std::string> parametersWithValues = initializeParameters();
    parametersWithValues.push_back("origin=45.5242,-73.5817");
    parametersWithValues.push_back("destination=45.54,-73.6146");
    parametersWithValues.push_back("arrival_time_seconds=" + std::to_string(arrivalTime));

    std::unique_ptr<TrRouting::RoutingResult> result = calculateOd(parametersWithValues);
    assertSuccessResults(*result.get(),
        -1,
        expectedTransitDepartureTime,
        travelTimeInVehicle,
        accessTime,
        egressTime);
}

// Test from OD With access/egress, origin is further south of South2, in the line axis, destination slightly north-west of midpoint
// Specify each parameter with a value that allows the result
TEST_F(RouteCalculationFixtureTests, SimpleODCalculationWithAllParams)
{
    int departureTime = getTimeInSeconds(9, 45);
    int minWaitingTime = 240;
    int travelTimeInVehicle = 420;
    // This is where mocking would be interesting. Those were taken from the first run of the test
    int accessTime = 469;
    int egressTime = 138;
    int expectedTransitDepartureTime = getTimeInSeconds(10);

    std::vector<std::string> parametersWithValues = initializeParameters();
    parametersWithValues.push_back("origin=45.5242,-73.5817");
    parametersWithValues.push_back("destination=45.54,-73.6146");
    parametersWithValues.push_back("departure_time_seconds=" + std::to_string(departureTime));
    parametersWithValues.push_back("min_waiting_time_seconds=" + std::to_string(minWaitingTime));
    parametersWithValues.push_back("max_access_travel_time_seconds=" + std::to_string(accessTime + 5));
    parametersWithValues.push_back("max_egress_travel_time_seconds=" + std::to_string(egressTime + 5));
    parametersWithValues.push_back("max_travel_time_seconds=" + std::to_string(expectedTransitDepartureTime - departureTime + egressTime + travelTimeInVehicle + 5));
    parametersWithValues.push_back("max_first_waiting_time_seconds=" + std::to_string(expectedTransitDepartureTime - departureTime));

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

// Same as SimpleODCalculation, but with max_access_travel_time_seconds lower than access time
TEST_F(RouteCalculationFixtureTests, NoRoutingAccessTimeLimit)
{
    int departureTime = getTimeInSeconds(9, 45);
    // This is where mocking would be interesting. Those were taken from the first run of the test
    int accessTime = 469;

    std::vector<std::string> parametersWithValues = initializeParameters();
    parametersWithValues.push_back("origin=45.5242,-73.5817");
    parametersWithValues.push_back("destination=45.54,-73.6146");
    parametersWithValues.push_back("departure_time_seconds=" + std::to_string(departureTime));
    parametersWithValues.push_back("max_access_travel_time_seconds=" + std::to_string(accessTime - 5));

    try {
        calculateOd(parametersWithValues);
        FAIL() << "Expected TrRouting::NoRoutingFoundException, no exception thrown";
    } catch (TrRouting::NoRoutingFoundException const & e) {
        assertNoRouting(e, TrRouting::NoRoutingReason::NO_ACCESS_AT_ORIGIN);
    } catch(...) {
        FAIL() << "Expected TrRouting::NoRoutingFoundException, another type was thrown";
    }
}

// Same as SimpleODCalculation, but with max_egress_travel_time_seconds lower than egress time
TEST_F(RouteCalculationFixtureTests, NoRoutingEgressTimeLimit)
{
    int departureTime = getTimeInSeconds(9, 45);
    // This is where mocking would be interesting. Those were taken from the first run of the test
    int egressTime = 138;

    std::vector<std::string> parametersWithValues = initializeParameters();
    parametersWithValues.push_back("origin=45.5242,-73.5817");
    parametersWithValues.push_back("destination=45.54,-73.6146");
    parametersWithValues.push_back("departure_time_seconds=" + std::to_string(departureTime));
    parametersWithValues.push_back("max_egress_travel_time_seconds=" + std::to_string(egressTime - 5));

    try {
        calculateOd(parametersWithValues);
        FAIL() << "Expected TrRouting::NoRoutingFoundException, no exception thrown";
    } catch (TrRouting::NoRoutingFoundException const & e) {
        assertNoRouting(e, TrRouting::NoRoutingReason::NO_ACCESS_AT_DESTINATION);
    } catch(...) {
        FAIL() << "Expected TrRouting::NoRoutingFoundException, another type was thrown";
    }
}

// Same as SimpleODCalculation, but with max_first_waiting_time_seconds lower than should be
TEST_F(RouteCalculationFixtureTests, NoRoutingMaxFirstWaitingTime)
{
    int departureTime = getTimeInSeconds(9, 45);
    int travelTimeInVehicle = 420;
    // This is where mocking would be interesting. Those were taken from the first run of the test
    int accessTime = 469;
    int egressTime = 138;
    int expectedTransitDepartureTime = getTimeInSeconds(10);

    std::vector<std::string> parametersWithValues = initializeParameters();
    parametersWithValues.push_back("origin=45.5242,-73.5817");
    parametersWithValues.push_back("destination=45.54,-73.6146");
    parametersWithValues.push_back("departure_time_seconds=" + std::to_string(departureTime));
    parametersWithValues.push_back("max_first_waiting_time_seconds=" + std::to_string(expectedTransitDepartureTime - accessTime - departureTime - 5));

    try {
        calculateOd(parametersWithValues);
        FAIL() << "Expected TrRouting::NoRoutingFoundException, no exception thrown";
    } catch (TrRouting::NoRoutingFoundException const & e) {
        assertNoRouting(e, TrRouting::NoRoutingReason::NO_SERVICE_FROM_ORIGIN);
    } catch(...) {
        FAIL() << "Expected TrRouting::NoRoutingFoundException, another type was thrown";
    }
}

// Same as SimpleODCalculation, but with min_waiting_time_seconds higher than available
TEST_F(RouteCalculationFixtureTests, NoRoutingMinWaitingTime)
{
    int departureTime = getTimeInSeconds(9, 45);
    int travelTimeInVehicle = 420;
    // This is where mocking would be interesting. Those were taken from the first run of the test
    int accessTime = 469;
    int egressTime = 138;
    int expectedTransitDepartureTime = getTimeInSeconds(10);

    std::vector<std::string> parametersWithValues = initializeParameters();
    parametersWithValues.push_back("origin=45.5242,-73.5817");
    parametersWithValues.push_back("destination=45.54,-73.6146");
    parametersWithValues.push_back("departure_time_seconds=" + std::to_string(departureTime));
    parametersWithValues.push_back("min_waiting_time_seconds=" + std::to_string(expectedTransitDepartureTime - departureTime));

    try {
        calculateOd(parametersWithValues);
        FAIL() << "Expected TrRouting::NoRoutingFoundException, no exception thrown";
    } catch (TrRouting::NoRoutingFoundException const & e) {
        assertNoRouting(e, TrRouting::NoRoutingReason::NO_SERVICE_FROM_ORIGIN);
    } catch(...) {
        FAIL() << "Expected TrRouting::NoRoutingFoundException, another type was thrown";
    }
}

// Same as SimpleODCalculation, but with max_travel_time lower than should be, there is no time to reach anywhere with trip, so no service
TEST_F(RouteCalculationFixtureTests, NoRoutingTravelTime)
{
    int departureTime = getTimeInSeconds(9, 45);
    int travelTimeInVehicle = 420;

    std::vector<std::string> parametersWithValues = initializeParameters();
    parametersWithValues.push_back("origin=45.5242,-73.5817");
    parametersWithValues.push_back("destination=45.54,-73.6146");
    parametersWithValues.push_back("departure_time_seconds=" + std::to_string(departureTime));
    parametersWithValues.push_back("max_travel_time_seconds=" + std::to_string(travelTimeInVehicle));

    try {
        calculateOd(parametersWithValues);
        FAIL() << "Expected TrRouting::NoRoutingFoundException, no exception thrown";
    } catch (TrRouting::NoRoutingFoundException const & e) {
        assertNoRouting(e, TrRouting::NoRoutingReason::NO_SERVICE_FROM_ORIGIN);
    } catch(...) {
        FAIL() << "Expected TrRouting::NoRoutingFoundException, another type was thrown";
    }
}

// origin is further south of South2, and destination near North2. max_travel_time is not sufficient to reach destination, but there are trips from origin
TEST_F(RouteCalculationFixtureTests, NoRoutingTravelTimeLongerTrip)
{
    int departureTime = getTimeInSeconds(9, 45);
    int totalTravelTime = 1800;

    std::vector<std::string> parametersWithValues = initializeParameters();
    parametersWithValues.push_back("origin=45.5242,-73.5817");
    parametersWithValues.push_back("destination=45.5466,-73.6405");
    parametersWithValues.push_back("departure_time_seconds=" + std::to_string(departureTime));
    parametersWithValues.push_back("max_travel_time_seconds=" + std::to_string(totalTravelTime));

    try {
        calculateOd(parametersWithValues);
        FAIL() << "Expected TrRouting::NoRoutingFoundException, no exception thrown";
    } catch (TrRouting::NoRoutingFoundException const & e) {
        assertNoRouting(e, TrRouting::NoRoutingReason::NO_ROUTING_FOUND);
    } catch(...) {
        FAIL() << "Expected TrRouting::NoRoutingFoundException, another type was thrown";
    }
}

std::vector<std::string> RouteCalculationFixtureTests::initializeParameters()
{
    std::vector<std::string> parametersWithValues;
    parametersWithValues.push_back("scenario_uuid=" + boost::uuids::to_string(scenarioUuid));
    parametersWithValues.push_back("min_waiting_time_seconds=" + std::to_string(MIN_WAITING_TIME));
    return parametersWithValues;
}

std::unique_ptr<TrRouting::RoutingResult> RouteCalculationFixtureTests::calculateOd(std::vector<std::string> parameters)
{
    calculator.params.setDefaultValues();
    TrRouting::RouteParameters routeParams = calculator.params.update(parameters,
        calculator.scenarioIndexesByUuid,
        calculator.scenarios,
        calculator.odTripIndexesByUuid,
        calculator.odTrips,
        calculator.nodeIndexesByUuid,
        calculator.nodes,
        calculator.dataSources);
    TrRouting::OsrmFetcher::birdDistanceAccessibilityEnabled = true;

    // TODO Shouldn't need to do this, but we do for now, benchmark needs to be started
    calculator.algorithmCalculationTime.start();
    calculator.benchmarking.clear();

    return calculator.calculate(routeParams);
}
