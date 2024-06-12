#include <errno.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/string_generator.hpp>

#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "parameters.hpp"
#include "parameters_test.hpp"
#include "scenario.hpp"
#include "point.hpp"
#include "service.hpp"
#include "toolbox.hpp" //MAX_INT

const std::string EMPTY_SCENARIO_UUID = "acdcef12-1111-2222-3333-444455558888";
const std::string TEST_SCENARIO_UUID = "12345678-9999-0000-1111-ababbabaabab";
const std::string INVALID_SCENARIO_UUID = "11111111-0000-0000-1111-ababbabaabab";

class AccessibilityParametersFixtureTests : public BaseParametersFixtureTests
{
protected:
    std::map<boost::uuids::uuid, TrRouting::Scenario> scenarios;
    TrRouting::Service service; //Empty service, as the param parser expect at least one
public:
    void SetUp( ) override
    {

        const boost::uuids::string_generator uuidGenerator;

        // Create a valid scenario, services and other data don't have to exist, we are creating parameters, not using them
        auto uuid = uuidGenerator(TEST_SCENARIO_UUID);
        scenarios[uuid].uuid = uuid;
        scenarios[uuid].name = "Test valid scenario";
        scenarios[uuid].servicesList.push_back(service);

        // Create an empty scenario
        uuid = uuidGenerator(EMPTY_SCENARIO_UUID);
        scenarios[uuid].uuid = uuid;
        scenarios[uuid].name = "Test empty scenario";
    }

    void TearDown( ) override
    {
        // Nothing to do? Shoud we delete the scenarios?
        scenarios.clear();
    }
};

class InvalidAccessibilityParamTestSuite : public AccessibilityParametersFixtureTests,
                              public testing::WithParamInterface<std::tuple<std::string, std::string, TrRouting::ParameterException::Type>> {
};

INSTANTIATE_TEST_SUITE_P(
    InvalidAccessibilityParamsStringValues, InvalidAccessibilityParamTestSuite,
    testing::Values(
        std::make_tuple("place", "", TrRouting::ParameterException::Type::MISSING_PLACE),
        std::make_tuple("scenario_id", "", TrRouting::ParameterException::Type::MISSING_SCENARIO),
        std::make_tuple("time_of_trip", "", TrRouting::ParameterException::Type::MISSING_TIME_OF_TRIP),
        std::make_tuple("time_of_trip", "-3", TrRouting::ParameterException::Type::MISSING_TIME_OF_TRIP),
        std::make_tuple("scenario_id", EMPTY_SCENARIO_UUID, TrRouting::ParameterException::Type::EMPTY_SCENARIO),
        std::make_tuple("scenario_id", "SOMEGARBAGE", TrRouting::ParameterException::Type::INVALID_SCENARIO),
        std::make_tuple("scenario_id", INVALID_SCENARIO_UUID, TrRouting::ParameterException::Type::INVALID_SCENARIO),
        std::make_tuple("place", "45", TrRouting::ParameterException::Type::INVALID_PLACE),
        std::make_tuple("place", "foo,bar", TrRouting::ParameterException::Type::INVALID_PLACE),
        std::make_tuple("min_waiting_time", "nan", TrRouting::ParameterException::Type::INVALID_NUMERICAL_DATA),
        std::make_tuple("max_access_travel_time", "nan", TrRouting::ParameterException::Type::INVALID_NUMERICAL_DATA),
        std::make_tuple("max_egress_travel_time", "nan", TrRouting::ParameterException::Type::INVALID_NUMERICAL_DATA),
        std::make_tuple("max_transfer_travel_time", "nan", TrRouting::ParameterException::Type::INVALID_NUMERICAL_DATA),
        std::make_tuple("max_travel_time", "nan", TrRouting::ParameterException::Type::INVALID_NUMERICAL_DATA),
        std::make_tuple("max_first_waiting_time", "nan", TrRouting::ParameterException::Type::INVALID_NUMERICAL_DATA)
    )
);

TEST_P(InvalidAccessibilityParamTestSuite, TestAccessibilityParams)
{
    std::tuple<std::string, std::string, TrRouting::ParameterException::Type> param = GetParam();
    std::string paramName = std::get<0>(param);
    std::string paramValue = std::get<1>(param);
    TrRouting::ParameterException::Type exceptionType = std::get<2>(param);

    std::vector<std::pair<std::string, std::string>> parametersWithValues;
    if (paramName != "place")
    {
        parametersWithValues.push_back(std::make_pair("place", "-73.5,45.5544"));
    }
    if (paramName != "time_of_trip")
    {
        parametersWithValues.push_back(std::make_pair("time_of_trip", "10800"));
    }
    if (paramName != "scenario_id")
    {
        parametersWithValues.push_back(std::make_pair("scenario_id",  TEST_SCENARIO_UUID));
    }
    if (!paramValue.empty())
    {
        parametersWithValues.push_back(std::make_pair(paramName, paramValue));
    }

    try {
        TrRouting::AccessibilityParameters queryParams = TrRouting::AccessibilityParameters::createAccessibilityParameter(parametersWithValues, scenarios);
        FAIL() << "Expected TrRouting::ParameterException, no exception was thrown";
    }
    catch(TrRouting::ParameterException const & err) {
        EXPECT_EQ(err.getType(), exceptionType);
    }
    catch(...) {
        FAIL() << "Expected TrRouting::ParameterException, another type was thrown";
    }
}

TEST_F(AccessibilityParametersFixtureTests, DefaultParameters)
{
    std::vector<std::pair<std::string, std::string>> parametersWithValues;
    parametersWithValues.push_back(std::make_pair("scenario_id",  TEST_SCENARIO_UUID));
    parametersWithValues.push_back(std::make_pair("place", "-73.5,45.5544"));
    parametersWithValues.push_back(std::make_pair("time_of_trip", "10800"));

    TrRouting::AccessibilityParameters queryParams = TrRouting::AccessibilityParameters::createAccessibilityParameter(parametersWithValues, scenarios);
    EXPECT_DOUBLE_EQ(queryParams.getPlace()->latitude, 45.5544);
    EXPECT_DOUBLE_EQ(queryParams.getPlace()->longitude, -73.5);
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

TEST_F(AccessibilityParametersFixtureTests, SetAllParameters)
{
    // Set arbitrary values for parameters, different for each case
    int minWaitingTime = 60, maxTotalTime = 3600, maxAccess = 600;
    int maxEgress = 900, maxTransfer = 750, maxFirst = 300;

    std::vector<std::pair<std::string, std::string>> parametersWithValues;
    parametersWithValues.push_back(std::make_pair("scenario_id",  TEST_SCENARIO_UUID));
    parametersWithValues.push_back(std::make_pair("place", "-73.5,45.5544"));
    parametersWithValues.push_back(std::make_pair("time_of_trip", "10800"));
    parametersWithValues.push_back(std::make_pair("time_type", "1"));
    parametersWithValues.push_back(std::make_pair("min_waiting_time", std::to_string(minWaitingTime)));
    parametersWithValues.push_back(std::make_pair("max_access_travel_time", std::to_string(maxAccess)));
    parametersWithValues.push_back(std::make_pair("max_egress_travel_time", std::to_string(maxEgress)));
    parametersWithValues.push_back(std::make_pair("max_transfer_travel_time", std::to_string(maxTransfer)));
    parametersWithValues.push_back(std::make_pair("max_travel_time", std::to_string(maxTotalTime)));
    parametersWithValues.push_back(std::make_pair("max_first_waiting_time", std::to_string(maxFirst)));

    TrRouting::AccessibilityParameters queryParams = TrRouting::AccessibilityParameters::createAccessibilityParameter(parametersWithValues, scenarios);
    EXPECT_DOUBLE_EQ(queryParams.getPlace()->latitude, 45.5544);
    EXPECT_DOUBLE_EQ(queryParams.getPlace()->longitude, -73.5);
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
