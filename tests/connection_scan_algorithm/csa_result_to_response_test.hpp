#include <errno.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/string_generator.hpp>
#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "routing_result.hpp"
#include "parameters.hpp"
#include "toolbox.hpp" //MAX_INT
#include "scenario.hpp"
#include "mode.hpp"
#include "agency.hpp"
#include "line.hpp"
#include "path.hpp"
#include "service.hpp"
#include "trip.hpp"
#include "node.hpp"

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
    inline static TrRouting::Point boardingNodePoint = TrRouting::Point(45.526, -73.59);
    inline static TrRouting::Point unboardingNodePoint = TrRouting::Point(45.53, -73.61);

    std::unique_ptr<TrRouting::Scenario> scenario;
    std::unique_ptr<TrRouting::RouteParameters> testParameters;

    std::unique_ptr<TrRouting::Mode> mode;
    std::unique_ptr<TrRouting::Agency> agency;
    std::unique_ptr<TrRouting::Line> line;
    std::unique_ptr<TrRouting::Path> path;
    std::unique_ptr<TrRouting::Service> service;
    std::unique_ptr<TrRouting::Trip> trip;
    std::unique_ptr<TrRouting::Node> boardingNode;
    std::unique_ptr<TrRouting::Node> unboardingNode;

    std::unique_ptr<TrRouting::SingleCalculationResult> getSingleResult();

public:
    void SetUp();

    void TearDown()
    {
        // Nothing to tear down
    }
};
