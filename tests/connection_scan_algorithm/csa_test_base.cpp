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
#include "station.hpp"
#include "stop.hpp"
#include "od_trip.hpp"

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

void addTransferableNode(TrRouting::Node* node, int index, int distance, int time)
{
    node->transferableNodesIdx.push_back(index);
    node->transferableDistancesMeters.push_back(distance);
    node->transferableTravelTimesSeconds.push_back(time);
    node->reverseTransferableNodesIdx.push_back(index);
    node->reverseTransferableDistancesMeters.push_back(distance);
    node->reverseTransferableTravelTimesSeconds.push_back(time);
}

void BaseCsaFixtureTests::setUpNodes()
{
    std::vector<std::unique_ptr<TrRouting::Node>>& array = calculator.nodes;
    std::map<boost::uuids::uuid, int>& arrayIndexesByUuid = calculator.nodeIndexesByUuid;

    // Create all 9 points, from south to north, then east to west. Each node MUST be transferable with itself
    // South2 has no transferable node
    std::unique_ptr<TrRouting::Node> south2 = std::make_unique<TrRouting::Node>();
    south2->uuid = nodeSouth2Uuid;
    south2->name = "South2";
    south2->point = std::make_unique<TrRouting::Point>(45.5269,-73.58912);
    addTransferableNode(south2.get(), array.size(), 0, 0);
    arrayIndexesByUuid[south2->uuid] = array.size();
    array.push_back(std::move(south2));

    // South1 has no transferable node
    std::unique_ptr<TrRouting::Node> south1 = std::make_unique<TrRouting::Node>();
    south1->uuid = nodeSouth1Uuid;
    south1->name = "South1";
    south1->point = std::make_unique<TrRouting::Point>(45.53258,-73.60196);
    addTransferableNode(south1.get(), array.size(), 0, 0);
    arrayIndexesByUuid[south1->uuid] = array.size();
    array.push_back(std::move(south1));

    // midNode is transferable with west1, east1 and north1
    std::unique_ptr<TrRouting::Node> midNode = std::make_unique<TrRouting::Node>();
    midNode->uuid = nodeMidNodeUuid;
    midNode->name = "MidPoint";
    midNode->point = std::make_unique<TrRouting::Point>(45.53827,-73.614436);
    addTransferableNode(midNode.get(), array.size(), 0, 0);
    addTransferableNode(midNode.get(), 7, 522, 558);
    addTransferableNode(midNode.get(), 6, 532, 480);
    addTransferableNode(midNode.get(), 3, 983, 801);
    arrayIndexesByUuid[midNode->uuid] = array.size();
    array.push_back(std::move(midNode));

    // transferable with midNode
    std::unique_ptr<TrRouting::Node> north1 = std::make_unique<TrRouting::Node>();
    north1->uuid = nodeNorth1Uuid;
    north1->name = "North1";
    north1->point = std::make_unique<TrRouting::Point>(45.54165,-73.62603);
    addTransferableNode(north1.get(), array.size(), 0, 0);
    addTransferableNode(north1.get(), 2, 983, 801);
    arrayIndexesByUuid[north1->uuid] = array.size();
    array.push_back(std::move(north1));

    // No transferable node
    std::unique_ptr<TrRouting::Node> north2 = std::make_unique<TrRouting::Node>();
    north2->uuid = nodeNorth2Uuid;
    north2->name = "North2";
    north2->point = std::make_unique<TrRouting::Point>(45.54634,-73.64266);
    addTransferableNode(north2.get(), array.size(), 0, 0);
    arrayIndexesByUuid[north2->uuid] = array.size();
    array.push_back(std::move(north2));

    // Transferable with east1
    std::unique_ptr<TrRouting::Node> east2 = std::make_unique<TrRouting::Node>();
    east2->uuid = nodeEast2Uuid;
    east2->name = "East2";
    east2->point = std::make_unique<TrRouting::Point>(45.55027,-73.60496);
    addTransferableNode(east2.get(), array.size(), 0, 0);
    addTransferableNode(east2.get(), 6, 1030, 857);
    arrayIndexesByUuid[east2->uuid] = array.size();
    array.push_back(std::move(east2));

    // Transferable with east2 and midNode
    std::unique_ptr<TrRouting::Node> east1 = std::make_unique<TrRouting::Node>();
    east1->uuid = nodeEast1Uuid;
    east1->name = "East1";
    east1->point = std::make_unique<TrRouting::Point>(45.54249,-73.61199);
    addTransferableNode(east1.get(), array.size(), 0, 0);
    addTransferableNode(east1.get(), 2, 532, 480);
    addTransferableNode(east1.get(), 5, 1030, 857);
    arrayIndexesByUuid[east1->uuid] = array.size();
    array.push_back(std::move(east1));

    // Transferable with midNode and west2
    std::unique_ptr<TrRouting::Node> west1 = std::make_unique<TrRouting::Node>();
    west1->uuid = nodeWest1Uuid;
    west1->name = "West1";
    west1->point = std::make_unique<TrRouting::Point>(45.53473,-73.61825);
    addTransferableNode(west1.get(), array.size(), 0, 0);
    addTransferableNode(west1.get(), 2, 522, 558);
    addTransferableNode(west1.get(), 8, 824, 655);
    arrayIndexesByUuid[west1->uuid] = array.size();
    array.push_back(std::move(west1));

    // Transferable with west1
    std::unique_ptr<TrRouting::Node> west2 = std::make_unique<TrRouting::Node>();
    west2->uuid = nodeWest2Uuid;
    west2->name = "West2";
    west2->point = std::make_unique<TrRouting::Point>(45.52962,-73.62265);
    addTransferableNode(west2.get(), array.size(), 0, 0);
    addTransferableNode(west2.get(), 7, 824, 655);
    arrayIndexesByUuid[west2->uuid] = array.size();
    array.push_back(std::move(west2));

    // Extra1 has no transferable node
    std::unique_ptr<TrRouting::Node> extra1 = std::make_unique<TrRouting::Node>();
    extra1->uuid = nodeExtra1Uuid;
    extra1->name = "Extra1";
    extra1->point = std::make_unique<TrRouting::Point>(45.55316,-73.61894);
    addTransferableNode(extra1.get(), array.size(), 0, 0);
    arrayIndexesByUuid[extra1->uuid] = array.size();
    array.push_back(std::move(extra1));

}

void BaseCsaFixtureTests::setUpAgencies()
{
    std::vector<std::unique_ptr<TrRouting::Agency>>& array = calculator.agencies;
    std::map<boost::uuids::uuid, int>& arrayIndexesByUuid = calculator.agencyIndexesByUuid;

    std::unique_ptr<TrRouting::Agency> agency = std::make_unique<TrRouting::Agency>();
    agency->uuid = agencyUuid;
    agency->name = "Unit Test Agency";
    agency->acronym = "UT";
    arrayIndexesByUuid[agency->uuid] = array.size();
    array.push_back(std::move(agency));
}

void BaseCsaFixtureTests::setUpLines()
{
    std::vector<std::unique_ptr<TrRouting::Line>>& array = calculator.lines;
    std::map<boost::uuids::uuid, int>& arrayIndexesByUuid = calculator.lineIndexesByUuid;

    std::unique_ptr<TrRouting::Line> lineSN = std::make_unique<TrRouting::Line>();
    lineSN->uuid = lineSNUuid;
    lineSN->agencyIdx = 0;
    lineSN->modeIdx = 0;
    lineSN->shortname = "01";
    lineSN->longname = "South/North";
    lineSN->allowSameLineTransfers = 0;
    arrayIndexesByUuid[lineSN->uuid] = array.size();
    array.push_back(std::move(lineSN));

    std::unique_ptr<TrRouting::Line> lineEW = std::make_unique<TrRouting::Line>();
    lineEW->uuid = lineEWUuid;
    lineEW->agencyIdx = 0;
    lineEW->modeIdx = 0;
    lineEW->shortname = "02";
    lineEW->longname = "East/West";
    lineEW->allowSameLineTransfers = 0;
    arrayIndexesByUuid[lineEW->uuid] = array.size();
    array.push_back(std::move(lineEW));

    std::unique_ptr<TrRouting::Line> lineExtra = std::make_unique<TrRouting::Line>();
    lineExtra->uuid = lineExtraUuid;
    lineExtra->agencyIdx = 0;
    lineExtra->modeIdx = 0;
    lineExtra->shortname = "03";
    lineExtra->longname = "Extra";
    lineExtra->allowSameLineTransfers = 0;
    arrayIndexesByUuid[lineExtra->uuid] = array.size();
    array.push_back(std::move(lineExtra));
}

void BaseCsaFixtureTests::setUpServices()
{
    std::vector<std::unique_ptr<TrRouting::Service>>& array = calculator.services;
    std::map<boost::uuids::uuid, int>& arrayIndexesByUuid = calculator.serviceIndexesByUuid;

    std::unique_ptr<TrRouting::Service> service = std::make_unique<TrRouting::Service>();
    service->uuid = serviceUuid;
    service->name = "Single Unit Test";
    arrayIndexesByUuid[service->uuid] = array.size();
    array.push_back(std::move(service));
}

void BaseCsaFixtureTests::setUpScenarios()
{
    std::vector<std::unique_ptr<TrRouting::Scenario>>& array = calculator.scenarios;
    std::map<boost::uuids::uuid, int>& arrayIndexesByUuid = calculator.scenarioIndexesByUuid;

    std::unique_ptr<TrRouting::Scenario> scenario = std::make_unique<TrRouting::Scenario>();
    scenario->uuid = scenarioUuid;
    scenario->name = "Test valid scenario";
    scenario->servicesIdx.push_back(0);
    arrayIndexesByUuid[scenario->uuid] = array.size();
    array.push_back(std::move(scenario));
}

void addNodeToPath(TrRouting::Path* path, int nodeIdx, int timeTraveled, int distance) {
    path->nodesIdx.push_back(nodeIdx);
    if (distance > 0) {
        path->segmentsDistanceMeters.push_back(distance);
    }
    if (timeTraveled > 0) {
        path->segmentsTravelTimeSeconds.push_back(timeTraveled);
    }
}

void BaseCsaFixtureTests::setUpPaths()
{
    std::vector<std::unique_ptr<TrRouting::Path>>& array = calculator.paths;
    std::map<boost::uuids::uuid, int>& arrayIndexesByUuid = calculator.pathIndexesByUuid;

    std::unique_ptr<TrRouting::Path> snPath = std::make_unique<TrRouting::Path>();
    snPath->uuid = pathSNUuid;
    snPath->lineIdx = calculator.lineIndexesByUuid[lineSNUuid];
    snPath->direction = "outbound";
    addNodeToPath(snPath.get(), calculator.nodeIndexesByUuid[nodeSouth2Uuid], 210, 1186);
    addNodeToPath(snPath.get(), calculator.nodeIndexesByUuid[nodeSouth1Uuid], 190, 1160);
    addNodeToPath(snPath.get(), calculator.nodeIndexesByUuid[nodeMidNodeUuid], 180, 980);
    addNodeToPath(snPath.get(), calculator.nodeIndexesByUuid[nodeNorth1Uuid], 270, 1544);
    addNodeToPath(snPath.get(), calculator.nodeIndexesByUuid[nodeNorth2Uuid], -1, -1);
    // Path's trip data will be filled in the setUpSchedules
    arrayIndexesByUuid[snPath->uuid] = array.size();
    array.push_back(std::move(snPath));

    std::unique_ptr<TrRouting::Path> ewPath = std::make_unique<TrRouting::Path>();
    ewPath->uuid = pathEWUuid;
    ewPath->lineIdx = calculator.lineIndexesByUuid[lineSNUuid];
    ewPath->direction = "outbound";
    addNodeToPath(ewPath.get(), calculator.nodeIndexesByUuid[nodeEast2Uuid], 150, 1025);
    addNodeToPath(ewPath.get(), calculator.nodeIndexesByUuid[nodeEast1Uuid], 110, 510);
    addNodeToPath(ewPath.get(), calculator.nodeIndexesByUuid[nodeMidNodeUuid], 150, 498);
    addNodeToPath(ewPath.get(), calculator.nodeIndexesByUuid[nodeWest1Uuid], 120, 668);
    addNodeToPath(ewPath.get(), calculator.nodeIndexesByUuid[nodeWest2Uuid], -1, -1);
    // Path's trip data will be filled in the setUpSchedules
    arrayIndexesByUuid[ewPath->uuid] = array.size();
    array.push_back(std::move(ewPath));

    std::unique_ptr<TrRouting::Path> extraPath = std::make_unique<TrRouting::Path>();
    extraPath->uuid = pathExtraUuid;
    extraPath->lineIdx = calculator.lineIndexesByUuid[lineExtraUuid];
    extraPath->direction = "outbound";
    addNodeToPath(extraPath.get(), calculator.nodeIndexesByUuid[nodeEast1Uuid], 300, 1760);
    addNodeToPath(extraPath.get(), calculator.nodeIndexesByUuid[nodeExtra1Uuid], -1, -1);
    // Path's trip data will be filled in the setUpSchedules
    arrayIndexesByUuid[extraPath->uuid] = array.size();
    array.push_back(std::move(extraPath));

}

void addTripData(TrRouting::Calculator& calculator, TrRouting::Trip *trip, std::vector<std::shared_ptr<TrRouting::ConnectionTuple>>& connections, int arrivalTimes[], int departureTimes[], int arraySize, int tripIdx)
{
    TrRouting::Path * path = calculator.paths[trip->pathIdx].get();
    path->tripsIdx.push_back(tripIdx);

    std::vector<std::unique_ptr<int>> connectionDepartureTimes = std::vector<std::unique_ptr<int>>(arraySize);
    std::vector<std::unique_ptr<float>> connectionDemands = std::vector<std::unique_ptr<float>>(arraySize);

    for (int nodeTimeI = 0; nodeTimeI < arraySize - 1; nodeTimeI++) {
        std::shared_ptr<TrRouting::ConnectionTuple> forwardConnection(std::make_shared<TrRouting::ConnectionTuple>(TrRouting::ConnectionTuple(
            path->nodesIdx[nodeTimeI],
            path->nodesIdx[nodeTimeI + 1],
            departureTimes[nodeTimeI],
            arrivalTimes[nodeTimeI + 1],
            tripIdx,
            1,
            1,
            nodeTimeI + 1,
            trip->lineIdx,
            trip->blockIdx,
            trip->allowSameLineTransfers,
            -1
        )));

        connections.push_back(std::move(forwardConnection));

        connectionDepartureTimes[nodeTimeI] = std::make_unique<int>(departureTimes[nodeTimeI]);
        connectionDemands[nodeTimeI] = std::make_unique<float>(0.0);

    }

    calculator.tripConnectionDepartureTimes.push_back(std::move(connectionDepartureTimes));
    calculator.tripConnectionDemands.push_back(std::move(connectionDemands));

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

    // South/North trip 1 at 10
    std::unique_ptr<TrRouting::Trip> snTrip1 = std::make_unique<TrRouting::Trip>();
    snTrip1->uuid = trip1SNUuid;
    snTrip1->agencyIdx = calculator.agencyIndexesByUuid[agencyUuid];
    snTrip1->lineIdx = calculator.lineIndexesByUuid[lineSNUuid];
    snTrip1->pathIdx = calculator.pathIndexesByUuid[pathSNUuid];
    snTrip1->modeIdx = 0;
    snTrip1->serviceIdx = calculator.serviceIndexesByUuid[serviceUuid];
    snTrip1->blockIdx = -1;

    tripIdx = array.size();
    arrayIndexesByUuid[snTrip1->uuid] = tripIdx;

    int arrivalTimesT1[5] = { getTimeInSeconds(10), getTimeInSeconds(10, 3, 30), getTimeInSeconds(10, 7), getTimeInSeconds(10, 13), getTimeInSeconds(10, 18) };
    int departureTimesT1[5] = { getTimeInSeconds(10), getTimeInSeconds(10, 3, 50), getTimeInSeconds(10, 10), getTimeInSeconds(10, 13, 30), getTimeInSeconds(10, 18) };

    addTripData(calculator, snTrip1.get(), connections, arrivalTimesT1, departureTimesT1, 5, tripIdx);
    array.push_back(std::move(snTrip1));

    // South/North trip 2 at 11
    std::unique_ptr<TrRouting::Trip> snTrip2 = std::make_unique<TrRouting::Trip>();
    snTrip2->uuid = trip2SNUuid;
    snTrip2->agencyIdx = calculator.agencyIndexesByUuid[agencyUuid];
    snTrip2->lineIdx = calculator.lineIndexesByUuid[lineSNUuid];
    snTrip2->pathIdx = calculator.pathIndexesByUuid[pathSNUuid];
    snTrip2->modeIdx = 0;
    snTrip2->serviceIdx = calculator.serviceIndexesByUuid[serviceUuid];
    snTrip2->blockIdx = -1;

    tripIdx = array.size();
    arrayIndexesByUuid[snTrip2->uuid] = tripIdx;

    int arrivalTimesT2[5] = { getTimeInSeconds(11), getTimeInSeconds(11, 3, 30), getTimeInSeconds(11, 7), getTimeInSeconds(11, 11), getTimeInSeconds(11, 16) };
    int departureTimesT2[5] = { getTimeInSeconds(11), getTimeInSeconds(11, 3, 50), getTimeInSeconds(11, 8), getTimeInSeconds(11, 11, 30), getTimeInSeconds(11, 17) };

    addTripData(calculator, snTrip2.get(), connections, arrivalTimesT2, departureTimesT2, 5, tripIdx);
    array.push_back(std::move(snTrip2));

    // East/West trip 1 at 9
    std::unique_ptr<TrRouting::Trip> ewTrip1 = std::make_unique<TrRouting::Trip>();
    ewTrip1->uuid = trip1EWUuid;
    ewTrip1->agencyIdx = calculator.agencyIndexesByUuid[agencyUuid];
    ewTrip1->lineIdx = calculator.lineIndexesByUuid[lineEWUuid];
    ewTrip1->pathIdx = calculator.pathIndexesByUuid[pathEWUuid];
    ewTrip1->modeIdx = 0;
    ewTrip1->serviceIdx = calculator.serviceIndexesByUuid[serviceUuid];
    ewTrip1->blockIdx = -1;

    tripIdx = array.size();
    arrayIndexesByUuid[ewTrip1->uuid] = tripIdx;

    int arrivalTimesT3[5] = { getTimeInSeconds(9), getTimeInSeconds(9, 2, 30), getTimeInSeconds(9, 4, 40), getTimeInSeconds(9, 7, 30), getTimeInSeconds(9, 10) };
    int departureTimesT3[5] = { getTimeInSeconds(9), getTimeInSeconds(9, 2, 50), getTimeInSeconds(9, 5), getTimeInSeconds(9, 8), getTimeInSeconds(9, 11) };

    addTripData(calculator, ewTrip1.get(), connections, arrivalTimesT3, departureTimesT3, 5, tripIdx);
    array.push_back(std::move(ewTrip1));

    // East/West trip 2 at 10:02
    std::unique_ptr<TrRouting::Trip> ewTrip2 = std::make_unique<TrRouting::Trip>();
    ewTrip2->uuid = trip2EWUuid;
    ewTrip2->agencyIdx = calculator.agencyIndexesByUuid[agencyUuid];
    ewTrip2->lineIdx = calculator.lineIndexesByUuid[lineEWUuid];
    ewTrip2->pathIdx = calculator.pathIndexesByUuid[pathEWUuid];
    ewTrip2->modeIdx = 0;
    ewTrip2->serviceIdx = calculator.serviceIndexesByUuid[serviceUuid];
    ewTrip2->blockIdx = -1;

    tripIdx = array.size();
    arrayIndexesByUuid[ewTrip2->uuid] = tripIdx;

    int arrivalTimesT4[5] = { getTimeInSeconds(10, 2), getTimeInSeconds(10, 4, 30), getTimeInSeconds(10, 7), getTimeInSeconds(10, 11, 30), getTimeInSeconds(10, 14) };
    int departureTimesT4[5] = { getTimeInSeconds(10, 2), getTimeInSeconds(10, 5, 10), getTimeInSeconds(10, 9), getTimeInSeconds(10, 12), getTimeInSeconds(10, 15) };

    addTripData(calculator, ewTrip2.get(), connections, arrivalTimesT4, departureTimesT4, 5, tripIdx);
    array.push_back(std::move(ewTrip2));

    // Extra trip at 10h20
    std::unique_ptr<TrRouting::Trip> extraTrip1 = std::make_unique<TrRouting::Trip>();
    extraTrip1->uuid = trip1ExtraUuid;
    extraTrip1->agencyIdx = calculator.agencyIndexesByUuid[agencyUuid];
    extraTrip1->lineIdx = calculator.lineIndexesByUuid[lineExtraUuid];
    extraTrip1->pathIdx = calculator.pathIndexesByUuid[pathExtraUuid];
    extraTrip1->modeIdx = 0;
    extraTrip1->serviceIdx = calculator.serviceIndexesByUuid[serviceUuid];
    extraTrip1->blockIdx = -1;

    tripIdx = array.size();
    arrayIndexesByUuid[extraTrip1->uuid] = tripIdx;

    int arrivalTimesT5[2] = { getTimeInSeconds(10, 20), getTimeInSeconds(10, 25) };
    int departureTimesT5[2] = { getTimeInSeconds(10, 20), getTimeInSeconds(10,26) };

    addTripData(calculator, extraTrip1.get(), connections, arrivalTimesT5, departureTimesT5, 2, tripIdx);
    array.push_back(std::move(extraTrip1));

}

void BaseCsaFixtureTests::setUpModes()
{
    TrRouting::Mode mode = {"bus", "Bus", 3, 700};
    calculator.modeIndexesByShortname[mode.shortname] = 0;
    calculator.modes.push_back(mode);
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
