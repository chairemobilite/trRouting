#include <errno.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "parameters.hpp"
#include "parameters_test.hpp"
#include "scenario.hpp"

const std::string EMPTY_SCENARIO_UUID = "acdcef12-1111-2222-3333-444455558888";

class RouteParametersFixtureTests : public BaseParametersFixtureTests
{
protected:
    std::vector<std::unique_ptr<TrRouting::Scenario>> scenarios;
    std::map<boost::uuids::uuid, int> scenarioIndexesByUuid;

public:
    void SetUp( ) override
    {

        const boost::uuids::string_generator uuidGenerator;

        // Create a valid scenario, services and other data don't have to exist, we are creating parameters, not using them
        std::unique_ptr<TrRouting::Scenario> scenario = std::make_unique<TrRouting::Scenario>();
        scenario->uuid = uuidGenerator("12345678-9999-0000-1111-ababbabaabab");

        scenario->name = "Test valid scenario";
        scenario->servicesIdx.push_back(0);
        scenario->servicesIdx.push_back(3);

        scenarioIndexesByUuid[scenario->uuid] = scenarios.size();
        scenarios.push_back(std::move(scenario));

        // Create an empty scenario
        scenario = std::make_unique<TrRouting::Scenario>();
        scenario->uuid = uuidGenerator(EMPTY_SCENARIO_UUID);
        scenario->name = "Test empty scenario";

        scenarioIndexesByUuid[scenario->uuid] = scenarios.size();
        scenarios.push_back(std::move(scenario));
    }

    void TearDown( ) override
    {
        // Nothing to do? Shoud we delete the scenarios?
        scenarios.clear();
        scenarioIndexesByUuid.clear();
    }
};

class InvalidParamTestSuite : public RouteParametersFixtureTests,
                              public testing::WithParamInterface<std::tuple<std::string, std::string, TrRouting::ParameterException::Type>> {
};

INSTANTIATE_TEST_SUITE_P(
    InvalidRouteParamsStringValues, InvalidParamTestSuite,
    testing::Values(
        std::make_tuple("origin", "", TrRouting::ParameterException::Type::MISSING_ORIGIN),
        std::make_tuple("destination", "", TrRouting::ParameterException::Type::MISSING_DESTINATION),
        std::make_tuple("scenario_id", "", TrRouting::ParameterException::Type::MISSING_SCENARIO),
        std::make_tuple("time_of_trip", "", TrRouting::ParameterException::Type::MISSING_TIME_OF_TRIP),
        std::make_tuple("time_of_trip", "-3", TrRouting::ParameterException::Type::MISSING_TIME_OF_TRIP),
        std::make_tuple("scenario_id", EMPTY_SCENARIO_UUID, TrRouting::ParameterException::Type::EMPTY_SCENARIO),
        std::make_tuple("origin", "45", TrRouting::ParameterException::Type::INVALID_ORIGIN),
        std::make_tuple("origin", "foo,bar", TrRouting::ParameterException::Type::INVALID_ORIGIN),
        std::make_tuple("destination", "-73", TrRouting::ParameterException::Type::INVALID_DESTINATION),
        std::make_tuple("destination", "foo,bar", TrRouting::ParameterException::Type::INVALID_DESTINATION),
        std::make_tuple("min_waiting_time", "nan", TrRouting::ParameterException::Type::INVALID_NUMERICAL_DATA),
        std::make_tuple("max_access_travel_time", "nan", TrRouting::ParameterException::Type::INVALID_NUMERICAL_DATA),
        std::make_tuple("max_egress_travel_time", "nan", TrRouting::ParameterException::Type::INVALID_NUMERICAL_DATA),
        std::make_tuple("max_transfer_travel_time", "nan", TrRouting::ParameterException::Type::INVALID_NUMERICAL_DATA),
        std::make_tuple("max_travel_time", "nan", TrRouting::ParameterException::Type::INVALID_NUMERICAL_DATA),
        std::make_tuple("max_first_waiting_time", "nan", TrRouting::ParameterException::Type::INVALID_NUMERICAL_DATA)
    )
);

TEST_P(InvalidParamTestSuite, TestParams)
{
    std::tuple<std::string, std::string, TrRouting::ParameterException::Type> param = GetParam();
    std::string paramName = std::get<0>(param);
    std::string paramValue = std::get<1>(param);
    TrRouting::ParameterException::Type exceptionType = std::get<2>(param);

    std::vector<std::pair<std::string, std::string>> parametersWithValues;
    if (paramName != "origin")
    {
        parametersWithValues.push_back(std::make_pair("origin", "-73.5,45.5544"));
    }
    if (paramName != "destination")
    {
        parametersWithValues.push_back(std::make_pair("destination", "-73.57786713522127,45.55239801892435"));
    }
    if (paramName != "time_of_trip")
    {
        parametersWithValues.push_back(std::make_pair("time_of_trip", "10800"));
    }
    if (paramName != "scenario_id")
    {
        parametersWithValues.push_back(std::make_pair("scenario_id", boost::uuids::to_string(scenarios[0]->uuid)));
    }
    if (!paramValue.empty())
    {
        parametersWithValues.push_back(std::make_pair(paramName, paramValue));
    }

    try {
        TrRouting::RouteParameters queryParams = TrRouting::RouteParameters::createRouteODParameter(parametersWithValues, scenarioIndexesByUuid, scenarios);
        FAIL() << "Expected TrRouting::ParameterException, no exception was thrown";
    }
    catch(TrRouting::ParameterException const & err) {
        EXPECT_EQ(err.getType(), exceptionType);
    }
    catch(...) {
        FAIL() << "Expected TrRouting::ParameterException, another type was thrown";
    }
}

TEST_F(RouteParametersFixtureTests, DefaultParameters)
{
    std::vector<std::pair<std::string, std::string>> parametersWithValues;
    parametersWithValues.push_back(std::make_pair("scenario_id", boost::uuids::to_string(scenarios[0]->uuid)));
    parametersWithValues.push_back(std::make_pair("origin", "-73.5,45.5544"));
    parametersWithValues.push_back(std::make_pair("destination", "-73.57786713522127, 45.55239801892435"));
    parametersWithValues.push_back(std::make_pair("time_of_trip", "10800"));

    TrRouting::RouteParameters queryParams = TrRouting::RouteParameters::createRouteODParameter(parametersWithValues, scenarioIndexesByUuid, scenarios);
    EXPECT_DOUBLE_EQ(queryParams.getOrigin()->latitude, 45.5544);
    EXPECT_DOUBLE_EQ(queryParams.getOrigin()->longitude, -73.5);
    EXPECT_DOUBLE_EQ(queryParams.getDestination()->latitude, 45.55239801892435);
    EXPECT_DOUBLE_EQ(queryParams.getDestination()->longitude, -73.57786713522127);
    EXPECT_EQ(queryParams.isWithAlternatives(), false);
    EXPECT_EQ(queryParams.isForwardCalculation(), true);

    // Values that should be set to defaults
    EXPECT_EQ(queryParams.getTimeOfTrip(), 10800);
    EXPECT_EQ(queryParams.getMinWaitingTimeSeconds(), 3 * 60);
    EXPECT_EQ(queryParams.getMaxTotalTravelTimeSeconds(), TrRouting::MAX_INT);
    EXPECT_EQ(queryParams.getMaxAccessWalkingTravelTimeSeconds(), 20 * 60);
    EXPECT_EQ(queryParams.getMaxEgressWalkingTravelTimeSeconds(), 20 * 60);
    EXPECT_EQ(queryParams.getMaxTransferWalkingTravelTimeSeconds(), 20 * 60);
    EXPECT_EQ(queryParams.getMaxFirstWaitingTimeSeconds(), 30 * 60);
}

TEST_F(RouteParametersFixtureTests, SetAllParameters)
{
    // Set arbitrary values for parameters, different for each case
    int minWaitingTime = 60, maxTotalTime = 3600, maxAccess = 600;
    int maxEgress = 900, maxTransfer = 750, maxFirst = 300;

    std::vector<std::pair<std::string, std::string>> parametersWithValues;
    parametersWithValues.push_back(std::make_pair("scenario_id", boost::uuids::to_string(scenarios[0]->uuid)));
    parametersWithValues.push_back(std::make_pair("origin", "-73.5,45.5544"));
    parametersWithValues.push_back(std::make_pair("destination", "-73.57786713522127, 45.55239801892435"));
    parametersWithValues.push_back(std::make_pair("time_of_trip", "10800"));
    parametersWithValues.push_back(std::make_pair("time_type", "1"));
    parametersWithValues.push_back(std::make_pair("alternatives", "1"));
    parametersWithValues.push_back(std::make_pair("min_waiting_time", std::to_string(minWaitingTime)));
    parametersWithValues.push_back(std::make_pair("max_access_travel_time", std::to_string(maxAccess)));
    parametersWithValues.push_back(std::make_pair("max_egress_travel_time", std::to_string(maxEgress)));
    parametersWithValues.push_back(std::make_pair("max_transfer_travel_time", std::to_string(maxTransfer)));
    parametersWithValues.push_back(std::make_pair("max_travel_time", std::to_string(maxTotalTime)));
    parametersWithValues.push_back(std::make_pair("max_first_waiting_time", std::to_string(maxFirst)));

    TrRouting::RouteParameters queryParams = TrRouting::RouteParameters::createRouteODParameter(parametersWithValues, scenarioIndexesByUuid, scenarios);
    EXPECT_DOUBLE_EQ(queryParams.getOrigin()->latitude, 45.5544);
    EXPECT_DOUBLE_EQ(queryParams.getOrigin()->longitude, -73.5);
    EXPECT_DOUBLE_EQ(queryParams.getDestination()->latitude, 45.55239801892435);
    EXPECT_DOUBLE_EQ(queryParams.getDestination()->longitude, -73.57786713522127);
    EXPECT_EQ(queryParams.isWithAlternatives(), true);
    EXPECT_EQ(queryParams.isForwardCalculation(), false);

    // Values that should be set to defaults
    EXPECT_EQ(queryParams.getTimeOfTrip(), 10800);
    EXPECT_EQ(queryParams.getMinWaitingTimeSeconds(), minWaitingTime);
    EXPECT_EQ(queryParams.getMaxTotalTravelTimeSeconds(), maxTotalTime);
    EXPECT_EQ(queryParams.getMaxAccessWalkingTravelTimeSeconds(), maxAccess);
    EXPECT_EQ(queryParams.getMaxEgressWalkingTravelTimeSeconds(), maxEgress);
    EXPECT_EQ(queryParams.getMaxTransferWalkingTravelTimeSeconds(), maxTransfer);
    EXPECT_EQ(queryParams.getMaxFirstWaitingTimeSeconds(), maxFirst);
}
