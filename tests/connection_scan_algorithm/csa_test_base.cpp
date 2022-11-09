#include <errno.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "gtest/gtest.h"
#include "calculator.hpp"
#include "csa_test_base.hpp"
#include "constants.hpp"
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
#include "spdlog/spdlog.h"

/**
 *  Create a default data set:
 * - one agency
 * - 2 orthogonal lines with a 5 stops each, joining in the their middle and 1
 *   extra line to test walking transfers
 * - One inbound path per line
 * - One service
 * - 2 trips per path, one of which is cadenced to make transfers easier.
 *
 * For easy updates and manipulations for other test cases, the textual gtfs
 * files are provided in the unit_test_data_gtfs directory. The original data
 * was arbitrarily generated in Transition, then the time numbers were rounded
 * to times more readable by a test case developer. The distances data was kept
 * as original because some bird distance calculations actually do take the
 * geographical data into account.
 *
 * To import in Transition again and visualize the paths and data, simply zip
 * the `unit_test_data_gtfs` directory in a `gtfs.zip` file, and in Transition,
 * import the gtfs as any other gtfs.
 *
 * TODO: Mock the bird distance calculations to use completely arbitrary data?
 * */
void BaseCsaFixtureTests::SetUp() {
    // Enable full debug output in the test runs
    spdlog::set_level(spdlog::level::debug);

    std::vector<std::shared_ptr<TrRouting::ConnectionTuple>> connections;

    calculator.initializeCalculationData();
}

void BaseCsaFixtureTests::assertNoRouting(const TrRouting::NoRoutingFoundException& exception, TrRouting::NoRoutingReason expectedReason)
{
    ASSERT_EQ(expectedReason, exception.getReason());
}

void BaseCsaFixtureTests::assertSuccessResults(TrRouting::RoutingResult& result,
    int origDepartureTime,
    int expTransitDepartureTime,
    int expInVehicleTravelTime,
    int expAccessTime,
    int expEgressTime,
    int expNbTransfers,
    int minWaitingTime,
    int expTotalWaitingTime,
    int expTransferWaitingTime,
    int expTransferTravelTime)
{
    ASSERT_EQ(TrRouting::result_type::SINGLE_CALCULATION, result.resType);
    TrRouting::SingleCalculationResult& routingResult = dynamic_cast<TrRouting::SingleCalculationResult&>(result);
    ASSERT_LE(origDepartureTime, routingResult.departureTime);
    ASSERT_EQ(expInVehicleTravelTime + expAccessTime + expEgressTime + expTotalWaitingTime + expTransferTravelTime, routingResult.totalTravelTime);
    ASSERT_EQ(expTransitDepartureTime + expInVehicleTravelTime + expEgressTime + (expTotalWaitingTime - minWaitingTime) + expTransferTravelTime, routingResult.arrivalTime);
    ASSERT_EQ(expTransitDepartureTime - expAccessTime - minWaitingTime, routingResult.departureTime);
    ASSERT_EQ(expNbTransfers, routingResult.numberOfTransfers);
    ASSERT_EQ(expInVehicleTravelTime, routingResult.totalInVehicleTime);
    ASSERT_EQ(expTransferTravelTime, routingResult.transferWalkingTime);
    ASSERT_EQ(expTotalWaitingTime, routingResult.totalWaitingTime);
    ASSERT_EQ(expAccessTime, routingResult.accessTravelTime);
    ASSERT_EQ(expEgressTime, routingResult.egressTravelTime);
    ASSERT_EQ(expTransferWaitingTime, routingResult.transferWaitingTime);
    ASSERT_EQ(minWaitingTime, routingResult.firstWaitingTime);
    ASSERT_EQ(expAccessTime + expEgressTime + expTransferTravelTime, routingResult.totalNonTransitTravelTime);
}
