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

    setUpModes();
    setUpNodes();
    setUpAgencies();
    setUpLines();
    setUpPaths();
    setUpServices();
    setUpScenarios();
    setUpSchedules(connections);
    calculator.setConnections(connections);
    calculator.initializeCalculationData();
}

void addSelfTransferableNode(TrRouting::Node& node) {

  node.transferableNodes.push_back(TrRouting::NodeTimeDistance(node, 0, 0));
  node.reverseTransferableNodes.push_back(TrRouting::NodeTimeDistance(node, 0, 0));

}

void addTransferableNode(TrRouting::Node& node, const TrRouting::NodeTimeDistance &ntd)
{
    node.transferableNodes.push_back(ntd);

    //TODO is this right? Shouldn't we add node to the one in ntd?
    node.reverseTransferableNodes.push_back(ntd);
}

void BaseCsaFixtureTests::setUpNodes()
{
    std::map<boost::uuids::uuid,TrRouting::Node>& array = calculator.nodes;

    // Create all 9 points, from south to north, then east to west. Each node MUST be transferable with itself
    // South2 has no transferable node
    array.emplace(nodeSouth2Uuid, TrRouting::Node(nodeSouth2Uuid,
                                                  0,
                                                  "",
                                                  "South2",
                                                  "",
                                                  std::make_unique<TrRouting::Point>(45.5269,-73.58912)));
    addSelfTransferableNode(array.at(nodeSouth2Uuid));

    // South1 has no transferable node
    array.emplace(nodeSouth1Uuid, TrRouting::Node(nodeSouth1Uuid,
                                                  0,
                                                  "",
                                                  "South1",
                                                  "",
                                                  std::make_unique<TrRouting::Point>(45.53258,-73.60196)));
    addSelfTransferableNode(array.at(nodeSouth1Uuid));

    // midNode is transferable with west1, east1 and north1
    array.emplace(nodeMidNodeUuid, TrRouting::Node(nodeMidNodeUuid,
                                                   0,
                                                   "",
                                                   "MidPoint",
                                                   "",
                                                   std::make_unique<TrRouting::Point>(45.53827,-73.614436)));
    addSelfTransferableNode(array.at(nodeMidNodeUuid));

    // transferable with midNode
    array.emplace(nodeNorth1Uuid, TrRouting::Node(nodeNorth1Uuid,
                                                  0,
                                                  "",
                                                  "North1",
                                                  "",
                                                  std::make_unique<TrRouting::Point>(45.54165,-73.62603)));
    addSelfTransferableNode(array.at(nodeNorth1Uuid));

    // No transferable node
    array.emplace(nodeNorth2Uuid, TrRouting::Node(nodeNorth2Uuid,
                                                  0,
                                                  "",
                                                  "North1",
                                                  "",
                                                  std::make_unique<TrRouting::Point>(45.54634,-73.64266)));
    addSelfTransferableNode(array.at(nodeNorth2Uuid));

    // Transferable with east1
    array.emplace(nodeEast2Uuid, TrRouting::Node(nodeEast2Uuid,
                                                 0,
                                                 "",
                                                 "East2",
                                                 "",
                                                 std::make_unique<TrRouting::Point>(45.55027,-73.60496)));
    addSelfTransferableNode(array.at(nodeEast2Uuid));

    // Transferable with east2 and midNode
    array.emplace(nodeEast1Uuid, TrRouting::Node(nodeEast1Uuid,
                                                 0,
                                                 "",
                                                 "East1",
                                                 "",
                                                 std::make_unique<TrRouting::Point>(45.54249,-73.61199)));
    addSelfTransferableNode(array.at(nodeEast1Uuid));
        
    // Transferable with midNode and west2
    array.emplace(nodeWest1Uuid, TrRouting::Node(nodeWest1Uuid,
                                                 0,
                                                 "",
                                                 "West1",
                                                 "",
                                                 std::make_unique<TrRouting::Point>(45.53473,-73.61825)));
    addSelfTransferableNode(array.at(nodeWest1Uuid));

    // Transferable with west1
    array.emplace(nodeWest2Uuid, TrRouting::Node(nodeWest2Uuid,
                                                 0,
                                                 "",
                                                 "West2",
                                                 "",
                                                 std::make_unique<TrRouting::Point>(45.52962,-73.62265)));
    addSelfTransferableNode(array.at(nodeWest2Uuid));

    
    // Extra1 has no transferable node
    array.emplace(nodeExtra1Uuid, TrRouting::Node(nodeExtra1Uuid,
                                                 0,
                                                 "",
                                                 "Extra1",
                                                 "",
                                                 std::make_unique<TrRouting::Point>(45.55316,-73.61894)));
    addSelfTransferableNode(array.at(nodeExtra1Uuid));

    // Add all transferable nodes
    addTransferableNode(array.at(nodeWest1Uuid), TrRouting::NodeTimeDistance(array.at(nodeWest2Uuid), 655,824));
    addTransferableNode(array.at(nodeWest2Uuid), TrRouting::NodeTimeDistance(array.at(nodeWest1Uuid), 665,824));
    addTransferableNode(array.at(nodeWest1Uuid), TrRouting::NodeTimeDistance(array.at(nodeMidNodeUuid), 558, 522));
    addTransferableNode(array.at(nodeEast1Uuid), TrRouting::NodeTimeDistance(array.at(nodeMidNodeUuid), 480, 532));
    addTransferableNode(array.at(nodeEast1Uuid), TrRouting::NodeTimeDistance(array.at(nodeEast2Uuid), 857, 1030));
    addTransferableNode(array.at(nodeEast2Uuid), TrRouting::NodeTimeDistance(array.at(nodeEast1Uuid), 857, 1030));
    addTransferableNode(array.at(nodeNorth1Uuid), TrRouting::NodeTimeDistance(array.at(nodeMidNodeUuid), 801, 983));
    addTransferableNode(array.at(nodeMidNodeUuid), TrRouting::NodeTimeDistance(array.at(nodeNorth1Uuid), 801, 983));
    addTransferableNode(array.at(nodeMidNodeUuid), TrRouting::NodeTimeDistance(array.at(nodeWest1Uuid), 558, 522));
    addTransferableNode(array.at(nodeMidNodeUuid), TrRouting::NodeTimeDistance(array.at(nodeEast1Uuid), 480, 532));
}

void BaseCsaFixtureTests::setUpAgencies()
{
    TrRouting::Agency agency;
    agency.uuid = agencyUuid;
    agency.name = "Unit Test Agency";
    agency.acronym = "UT";
    calculator.agencies[agencyUuid] = agency;
}

void BaseCsaFixtureTests::setUpLines()
{
    auto & busMode = calculator.getModes().at("bus");
    const auto & defaultAgency = calculator.agencies.at(agencyUuid);

    calculator.lines.emplace(lineSNUuid, TrRouting::Line(lineSNUuid, defaultAgency, busMode, "01", "South/North", "", 0));

    calculator.lines.emplace(lineEWUuid, TrRouting::Line(lineEWUuid, defaultAgency, busMode, "02", "East/West", "", 0));

    calculator.lines.emplace(lineExtraUuid, TrRouting::Line(lineExtraUuid, defaultAgency, busMode, "03", "Extra", "", 0));
}

void BaseCsaFixtureTests::setUpServices()
{

    TrRouting::Service service;
    service.uuid = serviceUuid;
    service.name = "Single Unit Test";

    calculator.services[serviceUuid] = service;
}

void BaseCsaFixtureTests::setUpScenarios()
{
    std::map<boost::uuids::uuid, TrRouting::Scenario>& array = calculator.scenarios;

    array[scenarioUuid].uuid = scenarioUuid;
    array[scenarioUuid].name = "Test valid scenario";
    array[scenarioUuid].servicesList.push_back(calculator.services.at(serviceUuid));
}

void addNodeToPath(std::vector<TrRouting::NodeTimeDistance>& nodesref, const TrRouting::Node &node, int timeTraveled, int distance) {
  nodesref.push_back(TrRouting::NodeTimeDistance(node,timeTraveled, distance));
}

void BaseCsaFixtureTests::setUpPaths()
{

    std::vector<int> emptyVector;
    std::vector<TrRouting::NodeTimeDistance> nodesref;
    addNodeToPath(nodesref, calculator.nodes.at(nodeSouth2Uuid), 210, 1186);
    addNodeToPath(nodesref, calculator.nodes.at(nodeSouth1Uuid), 190, 1160);
    addNodeToPath(nodesref, calculator.nodes.at(nodeMidNodeUuid), 180, 980);
    addNodeToPath(nodesref, calculator.nodes.at(nodeNorth1Uuid), 270, 1544);
    addNodeToPath(nodesref, calculator.nodes.at(nodeNorth2Uuid), -1, -1);
    calculator.paths.emplace(pathSNUuid, TrRouting::Path(pathSNUuid,
                                                         calculator.lines.at(lineSNUuid),
                                                         "outbound",
                                                         "",
                                                         nodesref,
                                                         emptyVector));
    // Path's trip data will be filled in the setUpSchedules
    
    nodesref.clear();
    addNodeToPath(nodesref, calculator.nodes.at(nodeEast2Uuid), 150, 1025);
    addNodeToPath(nodesref, calculator.nodes.at(nodeEast1Uuid), 110, 510);
    addNodeToPath(nodesref, calculator.nodes.at(nodeMidNodeUuid), 150, 498);
    addNodeToPath(nodesref, calculator.nodes.at(nodeWest1Uuid), 120, 668);
    addNodeToPath(nodesref, calculator.nodes.at(nodeWest2Uuid), -1, -1);
    calculator.paths.emplace(pathEWUuid, TrRouting::Path(pathEWUuid,
                                                         calculator.lines.at(lineEWUuid),
                                                         "outbound",
                                                         "",
                                                         nodesref,
                                                         emptyVector));
    // Path's trip data will be filled in the setUpSchedules

    nodesref.clear();
    addNodeToPath(nodesref, calculator.nodes.at(nodeEast1Uuid), 300, 1760);
    addNodeToPath(nodesref, calculator.nodes.at(nodeExtra1Uuid), -1, -1);
    calculator.paths.emplace(pathExtraUuid, TrRouting::Path(pathExtraUuid,
                                                            calculator.lines.at(lineExtraUuid),
                                                            "outbound",
                                                            "",
                                                            nodesref,
                                                            emptyVector));
    // Path's trip data will be filled in the setUpSchedules

}

void addTripData(TrRouting::Calculator& calculator, TrRouting::Trip *trip, TrRouting::Path & path, std::vector<std::shared_ptr<TrRouting::ConnectionTuple>>& connections, int arrivalTimes[], int departureTimes[], int arraySize, int tripIdx)
{
    path.tripsIdx.push_back(tripIdx);

    std::vector<std::unique_ptr<int>> connectionDepartureTimes = std::vector<std::unique_ptr<int>>(arraySize);

    for (int nodeTimeI = 0; nodeTimeI < arraySize - 1; nodeTimeI++) {
        std::shared_ptr<TrRouting::ConnectionTuple> forwardConnection(std::make_shared<TrRouting::ConnectionTuple>(TrRouting::ConnectionTuple(
            path.nodesRef[nodeTimeI],
            path.nodesRef[nodeTimeI + 1],
            departureTimes[nodeTimeI],
            arrivalTimes[nodeTimeI + 1],
            tripIdx,
            1,
            1,
            nodeTimeI + 1,
            trip->allowSameLineTransfers,
            -1
        )));

        connections.push_back(std::move(forwardConnection));

        connectionDepartureTimes[nodeTimeI] = std::make_unique<int>(departureTimes[nodeTimeI]);

    }

    calculator.tripConnectionDepartureTimes.push_back(std::move(connectionDepartureTimes));

}

// TODO: This is error prone, the procedure for the trip data (including the
// addTripData function above) was taken from
// trips_and_connection_cache_fetcher.cpp. This function should be split in
// smaller functions which could then be re-used by this test case. But before
// refactoring, let's add some tests! It's the chickend or the egg!
void BaseCsaFixtureTests::setUpSchedules(std::vector<std::shared_ptr<TrRouting::ConnectionTuple>> &connections)
{

    int tripIdx;
    std::vector<std::unique_ptr<TrRouting::Trip>>& array = calculator.trips;
    std::map<boost::uuids::uuid, int>& arrayIndexesByUuid = calculator.tripIndexesByUuid;
    auto & busMode = calculator.getModes().at("bus");

    // South/North trip 1 at 10
    std::unique_ptr<TrRouting::Trip> snTrip1 = std::make_unique<TrRouting::Trip>(trip1SNUuid,
                                                                                 calculator.agencies.at(agencyUuid),
                                                                                 calculator.lines.at(lineSNUuid),
                                                                                 calculator.paths.at(pathSNUuid),
                                                                                 busMode,
                                                                                 calculator.services.at(serviceUuid),
                                                                                 -1,
                                                                                 0);
    tripIdx = array.size();
    arrayIndexesByUuid[snTrip1->uuid] = tripIdx;

    int arrivalTimesT1[5] = { getTimeInSeconds(10), getTimeInSeconds(10, 3, 30), getTimeInSeconds(10, 7), getTimeInSeconds(10, 13), getTimeInSeconds(10, 18) };
    int departureTimesT1[5] = { getTimeInSeconds(10), getTimeInSeconds(10, 3, 50), getTimeInSeconds(10, 10), getTimeInSeconds(10, 13, 30), getTimeInSeconds(10, 18) };

    addTripData(calculator, snTrip1.get(), calculator.paths.at(pathSNUuid), connections, arrivalTimesT1, departureTimesT1, 5, tripIdx);
    array.push_back(std::move(snTrip1));

    // South/North trip 2 at 11
    std::unique_ptr<TrRouting::Trip> snTrip2 = std::make_unique<TrRouting::Trip>(trip2SNUuid,
                                                                                 calculator.agencies.at(agencyUuid),
                                                                                 calculator.lines.at(lineSNUuid),
                                                                                 calculator.paths.at(pathSNUuid),
                                                                                 busMode,
                                                                                 calculator.services.at(serviceUuid),
                                                                                 -1,
                                                                                 0);
    tripIdx = array.size();
    arrayIndexesByUuid[snTrip2->uuid] = tripIdx;

    int arrivalTimesT2[5] = { getTimeInSeconds(11), getTimeInSeconds(11, 3, 30), getTimeInSeconds(11, 7), getTimeInSeconds(11, 11), getTimeInSeconds(11, 16) };
    int departureTimesT2[5] = { getTimeInSeconds(11), getTimeInSeconds(11, 3, 50), getTimeInSeconds(11, 8), getTimeInSeconds(11, 11, 30), getTimeInSeconds(11, 17) };

    addTripData(calculator, snTrip2.get(), calculator.paths.at(pathSNUuid),  connections, arrivalTimesT2, departureTimesT2, 5, tripIdx);
    array.push_back(std::move(snTrip2));

    // East/West trip 1 at 9
    std::unique_ptr<TrRouting::Trip> ewTrip1 = std::make_unique<TrRouting::Trip>(trip1EWUuid,
                                                                                 calculator.agencies.at(agencyUuid),
                                                                                 calculator.lines.at(lineEWUuid),
                                                                                 calculator.paths.at(pathEWUuid),
                                                                                 busMode,
                                                                                 calculator.services.at(serviceUuid),
                                                                                 -1,
                                                                                 0);
    tripIdx = array.size();
    arrayIndexesByUuid[ewTrip1->uuid] = tripIdx;

    int arrivalTimesT3[5] = { getTimeInSeconds(9), getTimeInSeconds(9, 2, 30), getTimeInSeconds(9, 4, 40), getTimeInSeconds(9, 7, 30), getTimeInSeconds(9, 10) };
    int departureTimesT3[5] = { getTimeInSeconds(9), getTimeInSeconds(9, 2, 50), getTimeInSeconds(9, 5), getTimeInSeconds(9, 8), getTimeInSeconds(9, 11) };

    addTripData(calculator, ewTrip1.get(), calculator.paths.at(pathEWUuid), connections, arrivalTimesT3, departureTimesT3, 5, tripIdx);
    array.push_back(std::move(ewTrip1));

    // East/West trip 2 at 10:02
    std::unique_ptr<TrRouting::Trip> ewTrip2 = std::make_unique<TrRouting::Trip>(trip2EWUuid,
                                                                                 calculator.agencies.at(agencyUuid),
                                                                                 calculator.lines.at(lineEWUuid),
                                                                                 calculator.paths.at(pathEWUuid),
                                                                                 busMode,
                                                                                 calculator.services.at(serviceUuid),
                                                                                 -1,
                                                                                 0);
    tripIdx = array.size();
    arrayIndexesByUuid[ewTrip2->uuid] = tripIdx;

    int arrivalTimesT4[5] = { getTimeInSeconds(10, 2), getTimeInSeconds(10, 4, 30), getTimeInSeconds(10, 7), getTimeInSeconds(10, 11, 30), getTimeInSeconds(10, 14) };
    int departureTimesT4[5] = { getTimeInSeconds(10, 2), getTimeInSeconds(10, 5, 10), getTimeInSeconds(10, 9), getTimeInSeconds(10, 12), getTimeInSeconds(10, 15) };

    addTripData(calculator, ewTrip2.get(), calculator.paths.at(pathEWUuid), connections, arrivalTimesT4, departureTimesT4, 5, tripIdx);
    array.push_back(std::move(ewTrip2));

    // Extra trip at 10h20
    std::unique_ptr<TrRouting::Trip> extraTrip1 = std::make_unique<TrRouting::Trip>(trip1ExtraUuid,
                                                                                 calculator.agencies.at(agencyUuid),
                                                                                 calculator.lines.at(lineExtraUuid),
                                                                                 calculator.paths.at(pathExtraUuid),
                                                                                 busMode,
                                                                                 calculator.services.at(serviceUuid),
                                                                                 -1,
                                                                                 0);
    tripIdx = array.size();
    arrayIndexesByUuid[extraTrip1->uuid] = tripIdx;

    int arrivalTimesT5[2] = { getTimeInSeconds(10, 20), getTimeInSeconds(10, 25) };
    int departureTimesT5[2] = { getTimeInSeconds(10, 20), getTimeInSeconds(10,26) };

    addTripData(calculator, extraTrip1.get(), calculator.paths.at(pathExtraUuid), connections, arrivalTimesT5, departureTimesT5, 2, tripIdx);
    array.push_back(std::move(extraTrip1));

}

void BaseCsaFixtureTests::setUpModes()
{
    calculator.modes.emplace("bus", TrRouting::Mode("bus", "Bus", 3, 700));
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
