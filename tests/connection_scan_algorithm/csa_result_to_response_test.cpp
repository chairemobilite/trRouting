#include <errno.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "gtest/gtest.h"
#include "csa_result_to_response_test.hpp"

void ResultToResponseFixtureTest::SetUp()
{
    scenario = std::make_unique<TrRouting::Scenario>();
    scenario->uuid = uuidGenerator("aaaaaaaa-bbbb-cccc-dddd-eeeeeeffffff");
    scenario->name = "Test arbitrary scenario";

    testParameters = std::make_unique<TrRouting::RouteParameters>(
        std::make_unique<TrRouting::Point>(45.5269, -73.58912),
        std::make_unique<TrRouting::Point>(45.52184, -73.57817),
        *scenario,
        DEFAULT_TIME,
        DEFAULT_MIN_WAITING_TIME,
        DEFAULT_MAX_TOTAL_TIME,
        DEFAULT_MAX_ACCESS_TRAVEL_TIME,
        DEFAULT_MAX_EGRESS_TRAVEL_TIME,
        DEFAULT_MAX_TRANSFER_TRAVEL_TIME,
        DEFAULT_FIRST_WAITING_TIME,
        false,
        true
    );
}

std::unique_ptr<TrRouting::SingleCalculationResult> ResultToResponseFixtureTest::getSingleResult() {
    // Prepare the result, we test the conversion, the result doesn't have to make sense
    std::unique_ptr<TrRouting::SingleCalculationResult> result = std::make_unique<TrRouting::SingleCalculationResult>();
    result.get()->departureTime = 8 * 60 * 60 + 500;
    result.get()->totalTravelTime = 2000;
    result.get()->arrivalTime = result.get()->departureTime + result.get()->totalTravelTime;
    result.get()->totalDistance = 3000;
    result.get()->totalInVehicleTime = 1200;
    result.get()->totalInVehicleDistance = 2000;
    result.get()->totalNonTransitTravelTime = 800;
    result.get()->totalNonTransitDistance = 1000;
    result.get()->numberOfBoardings = 1;
    result.get()->numberOfTransfers = 0;
    result.get()->transferWalkingTime = 100;
    result.get()->transferWalkingDistance = 50;
    result.get()->accessTravelTime = 400;
    result.get()->accessDistance = 700;
    result.get()->egressTravelTime = 200;
    result.get()->egressDistance = 250;
    result.get()->transferWaitingTime = 60;
    result.get()->firstWaitingTime = 40;
    result.get()->totalWaitingTime = 100;

    result.get()->steps.push_back(std::make_unique<TrRouting::WalkingStep>(
        TrRouting::walking_step_type::ACCESS,
        result.get()->accessTravelTime,
        result.get()->accessDistance,
        result.get()->departureTime,
        result.get()->departureTime + result.get()->accessTravelTime,
        result.get()->departureTime + result.get()->accessTravelTime + 180
    ));

    result.get()->steps.push_back(std::make_unique<TrRouting::BoardingStep>(
        uuidGenerator(agencyUuid),
        "AG",
        "AgencyName",
        uuidGenerator(lineUuid),
        "LI",
        "LineName",
        uuidGenerator(pathUuid),
        "bus",
        "autobus",
        uuidGenerator(tripUuid),
        1,
        1,
        uuidGenerator(boardingNodeUuid),
        "NC",
        "NodeName",
        boardingNode,
        result.get()->departureTime + result.get()->accessTravelTime + 180,
        100
    ));

    result.get()->steps.push_back(std::make_unique<TrRouting::UnboardingStep>(
        uuidGenerator(agencyUuid),
        "AG",
        "AgencyName",
        uuidGenerator(lineUuid),
        "LI",
        "LineName",
        uuidGenerator(pathUuid),
        "bus",
        "autobus",
        uuidGenerator(tripUuid),
        3,
        4,
        uuidGenerator(unboardingNodeUuid),
        "NC1",
        "NodeName2",
        unboardingNode,
        result.get()->arrivalTime - result.get()->egressTravelTime,
        result.get()->totalInVehicleTime,
        result.get()->totalInVehicleDistance
    ));
    return std::move(result);
}