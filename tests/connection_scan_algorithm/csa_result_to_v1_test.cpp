#include <errno.h>
#include <experimental/filesystem>
#include <boost/uuid/string_generator.hpp>

#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "routing_result.hpp"
#include "result_to_v1.hpp"
#include "json.hpp"
#include "constants.hpp"
#include "toolbox.hpp"
#include "point.hpp"
#include "parameters.hpp"
#include "scenario.hpp"

namespace fs = std::experimental::filesystem;

class ResultToV1FixtureTest : public ::testing::Test
{
protected:
    inline static const boost::uuids::string_generator uuidGenerator;
    // test parameters, actual values are not important, it's just for result generation
    inline static const int DEFAULT_MIN_WAITING_TIME = 3 * 60;
    inline static const int DEFAULT_MAX_TOTAL_TIME = TrRouting::MAX_INT;
    inline static const int DEFAULT_MAX_ACCESS_TRAVEL_TIME = 20 * 60;
    inline static const int DEFAULT_MAX_EGRESS_TRAVEL_TIME = 20 * 60;
    inline static const int DEFAULT_MAX_TRANSFER_TRAVEL_TIME = 20 * 60;
    inline static const int DEFAULT_FIRST_WAITING_TIME = 30 * 60;
    inline static const int DEFAULT_TIME = 8 * 60 * 60;
    inline static const std::string agencyUuid = "aaaaaaaa-1111-cccc-dddd-eeeeeeffffff";
    inline static const std::string lineUuid = "aaaaaaaa-2222-cccc-dddd-eeeeeeffffff";
    inline static const std::string pathUuid = "aaaaaaaa-3333-cccc-dddd-eeeeeeffffff";
    inline static const std::string tripUuid = "aaaaaaaa-4444-cccc-dddd-eeeeeeffffff";
    inline static const std::string boardingNodeUuid = "aaaaaaaa-5555-cccc-dddd-eeeeeeffffff";
    inline static const std::string unboardingNodeUuid = "aaaaaaaa-6666-cccc-dddd-eeeeeeffffff";
    inline static TrRouting::Point boardingNode = TrRouting::Point(45.526, -73.59);
    inline static TrRouting::Point unboardingNode = TrRouting::Point(45.53, -73.61);

    std::unique_ptr<TrRouting::Scenario> scenario;
    std::unique_ptr<TrRouting::RouteParameters> testParameters;

    std::unique_ptr<TrRouting::SingleCalculationResult> getSingleResult();
    void assertResultConversion(nlohmann::json jsonResponse, TrRouting::SingleCalculationResult &result, TrRouting::RouteParameters &params);

public:
    void SetUp( ) override
    {
        scenario = std::make_unique<TrRouting::Scenario>();
        scenario->uuid = uuidGenerator("aaaaaaaa-bbbb-cccc-dddd-eeeeeeffffff");
        scenario->name = "Test arbitrary scenario";
        scenario->servicesIdx.push_back(0);

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

    void TearDown( ) override
    {
        // Nothing to tear down
    }
};

TEST_F(ResultToV1FixtureTest, TestNoRoutingFoundResultDefaultReason)
{
    nlohmann::json jsonResponse = TrRouting::ResultToV1Response::noRoutingFoundResponse(*testParameters.get(), TrRouting::NoRoutingReason::NO_ROUTING_FOUND);

    TrRouting::Point* origin = testParameters.get()->getOrigin();
    TrRouting::Point* destination = testParameters.get()->getDestination();

    ASSERT_EQ(STATUS_NO_ROUTING_FOUND, jsonResponse["status"]);
    ASSERT_EQ(origin->latitude, jsonResponse["origin"][0]);
    ASSERT_EQ(origin->longitude, jsonResponse["origin"][1]);
    ASSERT_EQ(destination->latitude, jsonResponse["destination"][0]);
    ASSERT_EQ(destination->longitude, jsonResponse["destination"][1]);
    ASSERT_EQ("NO_ROUTING_FOUND", jsonResponse["reason"]);
}

TEST_F(ResultToV1FixtureTest, TestNoRoutingFoundResultWithReason)
{
    nlohmann::json jsonResponse = TrRouting::ResultToV1Response::noRoutingFoundResponse(*testParameters.get(), TrRouting::NoRoutingReason::NO_ACCESS_AT_ORIGIN);

    TrRouting::Point* origin = testParameters.get()->getOrigin();
    TrRouting::Point* destination = testParameters.get()->getDestination();

    ASSERT_EQ(STATUS_NO_ROUTING_FOUND, jsonResponse["status"]);
    ASSERT_EQ(origin->latitude, jsonResponse["origin"][0]);
    ASSERT_EQ(origin->longitude, jsonResponse["origin"][1]);
    ASSERT_EQ(destination->latitude, jsonResponse["destination"][0]);
    ASSERT_EQ(destination->longitude, jsonResponse["destination"][1]);
    ASSERT_EQ("NO_ACCESS_AT_ORIGIN", jsonResponse["reason"]);
}

TEST_F(ResultToV1FixtureTest, TestSingleCalculationResult)
{
    std::unique_ptr<TrRouting::SingleCalculationResult> resultPtr = getSingleResult();
    TrRouting::SingleCalculationResult &result = *resultPtr.get();

    nlohmann::json jsonResponse = TrRouting::ResultToV1Response::resultToJsonString(result, *testParameters.get());

    assertResultConversion(jsonResponse, result, *testParameters.get());
}

TEST_F(ResultToV1FixtureTest, TestSingleCalculationBackwardResult)
{
    TrRouting::RouteParameters backwardParameters = TrRouting::RouteParameters(
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
        false
    );

    TrRouting::Point* origin = backwardParameters.getOrigin();
    TrRouting::Point* destination = backwardParameters.getDestination();

    std::unique_ptr<TrRouting::SingleCalculationResult> resultPtr = getSingleResult();
    TrRouting::SingleCalculationResult &result = *resultPtr.get();

    nlohmann::json jsonResponse = TrRouting::ResultToV1Response::resultToJsonString(result, backwardParameters);

    // Test a few fields
    ASSERT_EQ(STATUS_SUCCESS, jsonResponse["status"]);
    ASSERT_EQ(origin->latitude, jsonResponse["origin"][0]);
    ASSERT_EQ(origin->longitude, jsonResponse["origin"][1]);
    ASSERT_EQ(destination->latitude, jsonResponse["destination"][0]);
    ASSERT_EQ(destination->longitude, jsonResponse["destination"][1]);
    ASSERT_EQ("08:08", jsonResponse["departureTime"]);
    ASSERT_EQ(result.departureTime, jsonResponse["departureTimeSeconds"]);
    ASSERT_EQ(nlohmann::detail::value_t::null, jsonResponse["initialDepartureTime"]);
    ASSERT_EQ(nlohmann::detail::value_t::null, jsonResponse["initialDepartureTimeSeconds"]);
    ASSERT_EQ(nlohmann::detail::value_t::null, jsonResponse["initialLostTimeAtDepartureSeconds"]);
    ASSERT_EQ(nlohmann::detail::value_t::null, jsonResponse["initialLostTimeAtDepartureMinutes"]);
    ASSERT_EQ(TrRouting::Toolbox::convertSecondsToFormattedTime(result.arrivalTime), jsonResponse["arrivalTime"]);
    ASSERT_EQ(result.arrivalTime, jsonResponse["arrivalTimeSeconds"]);

}

TEST_F(ResultToV1FixtureTest, TestAlternativesResult)
{
    TrRouting::AlternativesResult result = TrRouting::AlternativesResult();
    result.totalAlternativesCalculated = 4;

    // Add results, both are the same, it's not important
    std::unique_ptr<TrRouting::SingleCalculationResult> resultPtr1 = getSingleResult();
    std::unique_ptr<TrRouting::SingleCalculationResult> resultPtr2 = getSingleResult();
    result.alternatives.push_back(std::move(resultPtr1));
    result.alternatives.push_back(std::move(resultPtr2));

    nlohmann::json jsonResponse = TrRouting::ResultToV1Response::resultToJsonString(result, *testParameters.get());

    ASSERT_EQ(STATUS_SUCCESS, jsonResponse["status"]);
    ASSERT_EQ(result.totalAlternativesCalculated, jsonResponse["alternativesTotal"]);
    ASSERT_EQ(2, jsonResponse["alternatives"].size());

    TrRouting::SingleCalculationResult& alternative1 = dynamic_cast<TrRouting::SingleCalculationResult&>(*result.alternatives[0].get());
    assertResultConversion(jsonResponse["alternatives"][0], alternative1, *testParameters.get());
    TrRouting::SingleCalculationResult& alternative2 = dynamic_cast<TrRouting::SingleCalculationResult&>(*result.alternatives[1].get());
    assertResultConversion(jsonResponse["alternatives"][1], alternative2, *testParameters.get());
}

TEST_F(ResultToV1FixtureTest, TestAllNodesResult)
{
    std::string node1Uuid = "aaaaaaaa-5555-cccc-dddd-eeeeeeffffff";
    std::string node2Uuid = "aaaaaaaa-6666-cccc-dddd-eeeeeeffffff";

    // Prepare the result, we test the conversion, the result doesn't have to make sense
    TrRouting::AllNodesResult result = TrRouting::AllNodesResult();
    result.numberOfReachableNodes = 2;
    result.percentOfReachableNodes = 0.3;
    TrRouting::AccessibleNodes node1Result = TrRouting::AccessibleNodes(
        uuidGenerator(node1Uuid),
        9 * 60 * 60,
        2000,
        2
    );
    TrRouting::AccessibleNodes node2Result = TrRouting::AccessibleNodes(
        uuidGenerator(node2Uuid),
        9 * 60 * 60 + 5 * 60,
        2100,
        1
    );

    result.nodes.push_back(node1Result);
    result.nodes.push_back(node2Result);

    nlohmann::json jsonResponse = TrRouting::ResultToV1Response::resultToJsonString(result, *testParameters.get());

    ASSERT_EQ(STATUS_SUCCESS, jsonResponse["status"]);
    ASSERT_EQ(result.numberOfReachableNodes, jsonResponse["numberOfReachableNodes"]);
    ASSERT_EQ(result.percentOfReachableNodes, jsonResponse["percentOfReachableNodes"]);
    ASSERT_EQ(result.numberOfReachableNodes, jsonResponse["nodes"].size());

    // Test the individual nodes
    ASSERT_EQ(node1Uuid, jsonResponse["nodes"][0]["id"]);
    ASSERT_EQ(TrRouting::Toolbox::convertSecondsToFormattedTime(node1Result.arrivalTime), jsonResponse["nodes"][0]["arrivalTime"]);
    ASSERT_EQ(node1Result.arrivalTime, jsonResponse["nodes"][0]["arrivalTimeSeconds"]);
    ASSERT_EQ(node1Result.totalTravelTime, jsonResponse["nodes"][0]["totalTravelTimeSeconds"]);
    ASSERT_EQ(node1Result.numberOfTransfers, jsonResponse["nodes"][0]["numberOfTransfers"]);

    ASSERT_EQ(node2Uuid, jsonResponse["nodes"][1]["id"]);
    ASSERT_EQ(TrRouting::Toolbox::convertSecondsToFormattedTime(node2Result.arrivalTime), jsonResponse["nodes"][1]["arrivalTime"]);
    ASSERT_EQ(node2Result.arrivalTime, jsonResponse["nodes"][1]["arrivalTimeSeconds"]);
    ASSERT_EQ(node2Result.totalTravelTime, jsonResponse["nodes"][1]["totalTravelTimeSeconds"]);
    ASSERT_EQ(node2Result.numberOfTransfers, jsonResponse["nodes"][1]["numberOfTransfers"]);
}

std::unique_ptr<TrRouting::SingleCalculationResult> ResultToV1FixtureTest::getSingleResult() {
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

// Matches the single result returned by getSingleResult, with some hard-coded values. If necessary, it will need to be adapted to match any result
void ResultToV1FixtureTest::assertResultConversion(nlohmann::json jsonResponse, TrRouting::SingleCalculationResult &result, TrRouting::RouteParameters &params) {
    TrRouting::Point* origin = params.getOrigin();
    TrRouting::Point* destination = params.getDestination();

    // Validate main results
    ASSERT_EQ(STATUS_SUCCESS, jsonResponse["status"]);
    ASSERT_EQ(origin->latitude, jsonResponse["origin"][0]);
    ASSERT_EQ(origin->longitude, jsonResponse["origin"][1]);
    ASSERT_EQ(destination->latitude, jsonResponse["destination"][0]);
    ASSERT_EQ(destination->longitude, jsonResponse["destination"][1]);
    ASSERT_EQ("08:08", jsonResponse["departureTime"]);
    ASSERT_EQ(result.departureTime, jsonResponse["departureTimeSeconds"]);
    ASSERT_EQ("08:00", jsonResponse["initialDepartureTime"]);
    ASSERT_EQ(8 * 60 * 60, jsonResponse["initialDepartureTimeSeconds"]);
    ASSERT_EQ(500, jsonResponse["initialLostTimeAtDepartureSeconds"]);
    ASSERT_EQ(8, jsonResponse["initialLostTimeAtDepartureMinutes"]);
    ASSERT_EQ(TrRouting::Toolbox::convertSecondsToFormattedTime(result.arrivalTime), jsonResponse["arrivalTime"]);
    ASSERT_EQ(result.arrivalTime, jsonResponse["arrivalTimeSeconds"]);
    ASSERT_EQ(TrRouting::Toolbox::convertSecondsToMinutes(result.totalTravelTime), jsonResponse["totalTravelTimeMinutes"]);
    ASSERT_EQ(result.totalTravelTime, jsonResponse["totalTravelTimeSeconds"]);
    ASSERT_EQ(result.totalDistance, jsonResponse["totalDistanceMeters"]);
    ASSERT_EQ(TrRouting::Toolbox::convertSecondsToMinutes(result.totalInVehicleTime), jsonResponse["totalInVehicleTimeMinutes"]);
    ASSERT_EQ(result.totalInVehicleTime, jsonResponse["totalInVehicleTimeSeconds"]);
    ASSERT_EQ(result.totalInVehicleDistance, jsonResponse["totalInVehicleDistanceMeters"]);
    ASSERT_EQ(TrRouting::Toolbox::convertSecondsToMinutes(result.totalNonTransitTravelTime), jsonResponse["totalNonTransitTravelTimeMinutes"]);
    ASSERT_EQ(result.totalNonTransitTravelTime, jsonResponse["totalNonTransitTravelTimeSeconds"]);
    ASSERT_EQ(result.totalNonTransitDistance, jsonResponse["totalNonTransitDistanceMeters"]);
    ASSERT_EQ(result.numberOfBoardings, jsonResponse["numberOfBoardings"]);
    ASSERT_EQ(result.numberOfTransfers, jsonResponse["numberOfTransfers"]);
    ASSERT_EQ(TrRouting::Toolbox::convertSecondsToMinutes(result.transferWalkingTime), jsonResponse["transferWalkingTimeMinutes"]);
    ASSERT_EQ(result.transferWalkingTime, jsonResponse["transferWalkingTimeSeconds"]);
    ASSERT_EQ(result.transferWalkingDistance, jsonResponse["transferWalkingDistanceMeters"]);
    ASSERT_EQ(TrRouting::Toolbox::convertSecondsToMinutes(result.accessTravelTime), jsonResponse["accessTravelTimeMinutes"]);
    ASSERT_EQ(result.accessTravelTime, jsonResponse["accessTravelTimeSeconds"]);
    ASSERT_EQ(result.accessDistance, jsonResponse["accessDistanceMeters"]);
    ASSERT_EQ(TrRouting::Toolbox::convertSecondsToMinutes(result.egressTravelTime), jsonResponse["egressTravelTimeMinutes"]);
    ASSERT_EQ(result.egressTravelTime, jsonResponse["egressTravelTimeSeconds"]);
    ASSERT_EQ(result.egressDistance, jsonResponse["egressDistanceMeters"]);
    ASSERT_EQ(TrRouting::Toolbox::convertSecondsToMinutes(result.transferWaitingTime), jsonResponse["transferWaitingTimeMinutes"]);
    ASSERT_EQ(result.transferWaitingTime, jsonResponse["transferWaitingTimeSeconds"]);
    ASSERT_EQ(TrRouting::Toolbox::convertSecondsToMinutes(result.firstWaitingTime), jsonResponse["firstWaitingTimeMinutes"]);
    ASSERT_EQ(result.firstWaitingTime, jsonResponse["firstWaitingTimeSeconds"]);
    ASSERT_EQ(TrRouting::Toolbox::convertSecondsToMinutes(result.totalWaitingTime), jsonResponse["totalWaitingTimeMinutes"]);
    ASSERT_EQ(result.totalWaitingTime, jsonResponse["totalWaitingTimeSeconds"]);

    // Test the walking step
    TrRouting::WalkingStep& walkingStep = dynamic_cast<TrRouting::WalkingStep&>(*result.steps[0].get());
    ASSERT_EQ("walking", jsonResponse["steps"][0]["action"]);
    ASSERT_EQ("access", jsonResponse["steps"][0]["type"]);
    ASSERT_EQ(walkingStep.travelTime, jsonResponse["steps"][0]["travelTimeSeconds"]);
    ASSERT_EQ(TrRouting::Toolbox::convertSecondsToMinutes(walkingStep.travelTime), jsonResponse["steps"][0]["travelTimeMinutes"]);
    ASSERT_EQ(walkingStep.distanceMeters, jsonResponse["steps"][0]["distanceMeters"]);
    ASSERT_EQ(TrRouting::Toolbox::convertSecondsToFormattedTime(walkingStep.departureTime), jsonResponse["steps"][0]["departureTime"]);
    ASSERT_EQ(TrRouting::Toolbox::convertSecondsToFormattedTime(walkingStep.arrivalTime), jsonResponse["steps"][0]["arrivalTime"]);
    ASSERT_EQ(walkingStep.departureTime, jsonResponse["steps"][0]["departureTimeSeconds"]);
    ASSERT_EQ(walkingStep.arrivalTime, jsonResponse["steps"][0]["arrivalTimeSeconds"]);
    ASSERT_EQ(TrRouting::Toolbox::convertSecondsToFormattedTime(walkingStep.readyToBoardAt), jsonResponse["steps"][0]["readyToBoardAt"]);

    // Test the boarding step
    TrRouting::BoardingStep& boardingStep = dynamic_cast<TrRouting::BoardingStep&>(*result.steps[1].get());
    ASSERT_EQ("board", jsonResponse["steps"][1]["action"]);
    ASSERT_EQ(boardingStep.agencyAcronym, jsonResponse["steps"][1]["agencyAcronym"]);
    ASSERT_EQ(boardingStep.agencyName, jsonResponse["steps"][1]["agencyName"]);
    ASSERT_EQ(agencyUuid, jsonResponse["steps"][1]["agencyUuid"]);
    ASSERT_EQ(boardingStep.lineShortname, jsonResponse["steps"][1]["lineShortname"]);
    ASSERT_EQ(boardingStep.lineLongname, jsonResponse["steps"][1]["lineLongname"]);
    ASSERT_EQ(lineUuid, jsonResponse["steps"][1]["lineUuid"]);
    ASSERT_EQ(pathUuid, jsonResponse["steps"][1]["pathUuid"]);
    ASSERT_EQ(boardingStep.modeName, jsonResponse["steps"][1]["modeName"]);
    ASSERT_EQ(boardingStep.mode, jsonResponse["steps"][1]["mode"]);
    ASSERT_EQ(tripUuid, jsonResponse["steps"][1]["tripUuid"]);
    ASSERT_EQ(boardingStep.legSequenceInTrip, jsonResponse["steps"][1]["legSequenceInTrip"]);
    ASSERT_EQ(boardingStep.stopSequenceInTrip, jsonResponse["steps"][1]["stopSequenceInTrip"]);
    ASSERT_EQ(boardingStep.nodeName, jsonResponse["steps"][1]["nodeName"]);
    ASSERT_EQ(boardingStep.nodeCode, jsonResponse["steps"][1]["nodeCode"]);
    ASSERT_EQ(boardingNodeUuid, jsonResponse["steps"][1]["nodeUuid"]);
    ASSERT_EQ(boardingNode.longitude, jsonResponse["steps"][1]["nodeCoordinates"][0]);
    ASSERT_EQ(boardingNode.latitude, jsonResponse["steps"][1]["nodeCoordinates"][1]);
    ASSERT_EQ(TrRouting::Toolbox::convertSecondsToFormattedTime(boardingStep.departureTime), jsonResponse["steps"][1]["departureTime"]);
    ASSERT_EQ(boardingStep.departureTime, jsonResponse["steps"][1]["departureTimeSeconds"]);
    ASSERT_EQ(boardingStep.waitingTime, jsonResponse["steps"][1]["waitingTimeSeconds"]);
    ASSERT_EQ(TrRouting::Toolbox::convertSecondsToMinutes(boardingStep.waitingTime), jsonResponse["steps"][1]["waitingTimeMinutes"]);

    // Test the unboarding step
    TrRouting::UnboardingStep& unboardingStep = dynamic_cast<TrRouting::UnboardingStep&>(*result.steps[2].get());
    ASSERT_EQ("unboard", jsonResponse["steps"][2]["action"]);
    ASSERT_EQ(unboardingStep.agencyAcronym, jsonResponse["steps"][2]["agencyAcronym"]);
    ASSERT_EQ(unboardingStep.agencyName, jsonResponse["steps"][2]["agencyName"]);
    ASSERT_EQ(agencyUuid, jsonResponse["steps"][2]["agencyUuid"]);
    ASSERT_EQ(unboardingStep.lineShortname, jsonResponse["steps"][2]["lineShortname"]);
    ASSERT_EQ(unboardingStep.lineLongname, jsonResponse["steps"][2]["lineLongname"]);
    ASSERT_EQ(lineUuid, jsonResponse["steps"][2]["lineUuid"]);
    ASSERT_EQ(pathUuid, jsonResponse["steps"][2]["pathUuid"]);
    ASSERT_EQ(unboardingStep.modeName, jsonResponse["steps"][2]["modeName"]);
    ASSERT_EQ(unboardingStep.mode, jsonResponse["steps"][2]["mode"]);
    ASSERT_EQ(tripUuid, jsonResponse["steps"][2]["tripUuid"]);
    ASSERT_EQ(unboardingStep.legSequenceInTrip, jsonResponse["steps"][2]["legSequenceInTrip"]);
    ASSERT_EQ(unboardingStep.stopSequenceInTrip, jsonResponse["steps"][2]["stopSequenceInTrip"]);
    ASSERT_EQ(unboardingStep.nodeName, jsonResponse["steps"][2]["nodeName"]);
    ASSERT_EQ(unboardingStep.nodeCode, jsonResponse["steps"][2]["nodeCode"]);
    ASSERT_EQ(unboardingNodeUuid, jsonResponse["steps"][2]["nodeUuid"]);
    ASSERT_EQ(unboardingNode.longitude, jsonResponse["steps"][2]["nodeCoordinates"][0]);
    ASSERT_EQ(unboardingNode.latitude, jsonResponse["steps"][2]["nodeCoordinates"][1]);
    ASSERT_EQ(TrRouting::Toolbox::convertSecondsToFormattedTime(unboardingStep.arrivalTime), jsonResponse["steps"][2]["arrivalTime"]);
    ASSERT_EQ(unboardingStep.arrivalTime, jsonResponse["steps"][2]["arrivalTimeSeconds"]);
    ASSERT_EQ(unboardingStep.inVehicleTime, jsonResponse["steps"][2]["inVehicleTimeSeconds"]);
    ASSERT_EQ(TrRouting::Toolbox::convertSecondsToMinutes(unboardingStep.inVehicleTime), jsonResponse["steps"][2]["inVehicleTimeMinutes"]);
    ASSERT_EQ(unboardingStep.inVehicleDistanceMeters, jsonResponse["steps"][2]["inVehicleDistanceMeters"]);
}
