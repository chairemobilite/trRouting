#include <errno.h>
#include <experimental/filesystem>

#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "result_to_response.hpp"
#include "routing_result.hpp"
#include "json.hpp"

namespace fs = std::experimental::filesystem;

class ResultToV1FixtureTest : public ::testing::Test
{
protected:
    inline static const boost::uuids::string_generator uuidGenerator;
    // test parameters, actual values are not important, it's just for result generation
    static const int DEFAULT_MIN_WAITING_TIME = 3 * 60;
    static const int DEFAULT_MAX_TOTAL_TIME = TrRouting::MAX_INT;
    static const int DEFAULT_MAX_ACCESS_TRAVEL_TIME = 20 * 60;
    static const int DEFAULT_MAX_EGRESS_TRAVEL_TIME = 20 * 60;
    static const int DEFAULT_MAX_TRANSFER_TRAVEL_TIME = 20 * 60;
    static const int DEFAULT_FIRST_WAITING_TIME = 30 * 60;
    static const int DEFAULT_TIME = 8 * 60 * 60;

    std::unique_ptr<TrRouting::Scenario> scenario;
    std::unique_ptr<TrRouting::RouteParameters> testParameters;

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

TEST_F(ResultToV1FixtureTest, TestNoRoutingFoundResult)
{
    TrRouting::NoRoutingFoundResult result = TrRouting::NoRoutingFoundResult(*testParameters.get());
    TrRouting::ResultToV1Response converter = TrRouting::ResultToV1Response();
    nlohmann::json jsonResponse = converter.resultToJsonString(result);

    TrRouting::Point* origin = testParameters.get()->getOrigin();
    TrRouting::Point* destination = testParameters.get()->getDestination();

    ASSERT_EQ("no_routing_found", jsonResponse["status"]);
    ASSERT_EQ(origin->latitude, jsonResponse["origin"][0]);
    ASSERT_EQ(origin->longitude, jsonResponse["origin"][1]);
    ASSERT_EQ(destination->latitude, jsonResponse["destination"][0]);
    ASSERT_EQ(destination->longitude, jsonResponse["destination"][1]);
}


