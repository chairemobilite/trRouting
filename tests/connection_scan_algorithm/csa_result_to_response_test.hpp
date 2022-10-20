#include <errno.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/string_generator.hpp>
#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "routing_result.hpp"
#include "json.hpp"
#include "parameters.hpp"
#include "toolbox.hpp" //MAX_INT
#include "scenario.hpp"


// Base class for all result to responses conversions
class ResultToResponseFixtureTest : public ::testing::Test
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
    virtual void assertResultConversion(nlohmann::json jsonResponse, TrRouting::SingleCalculationResult &result, TrRouting::RouteParameters &params) = 0;

public:
    void SetUp();

    void TearDown()
    {
        // Nothing to tear down
    }
};
