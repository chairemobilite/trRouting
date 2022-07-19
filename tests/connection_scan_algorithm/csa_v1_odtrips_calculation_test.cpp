#include <errno.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "gtest/gtest.h"
#include "calculator.hpp"
#include "csa_test_base.hpp"
#include "csa_v1_simple_calculation_test.hpp"
#include "constants.hpp"
#include "mode.hpp"
#include "data_source.hpp"
#include "household.hpp"
#include "person.hpp"
#include "place.hpp"
#include "agency.hpp"
#include "service.hpp"
#include "station.hpp"
#include "stop.hpp"
#include "line.hpp"
#include "path.hpp"
#include "trip.hpp"
#include "od_trip.hpp"
#include "node.hpp"
#include "osrm_fetcher.hpp"

/**
 * This file covers od trips use cases, where the value of od_trips is set to 1
 * */

// TODO:
// Do more than simple single use case, but at least we make sure the query path always works

class RouteOdTripsFixtureTests : public RouteCalculationFixtureTests
{
private:
    inline static const boost::uuids::uuid dataSourceUuid = uuidGenerator("12121212-3434-5656-7878-5f008b95c7b8");
    inline static const boost::uuids::uuid odTripUuid = uuidGenerator("21212121-4343-6565-8787-5422d6a36f46");

public:
    // Return a parameters vector with the scenario and minimum waiting time, which is repetitive and mandatory
    std::vector<std::string> initializeParameters();
    // Helper method to set parameters and calculate OD trip result. Test cases need only provide parameters and validate the result
    nlohmann::json calculateOdTrips(std::vector<std::string> parameters);
    void SetUp();
    void setupDataSources();
    void setupOdTrips();
};

void RouteOdTripsFixtureTests::setupDataSources() {
    std::map<boost::uuids::uuid, TrRouting::DataSource>& array = calculator.dataSources;

    TrRouting::DataSource dataSource;
    dataSource.uuid = dataSourceUuid;
    dataSource.shortname = "testDS";
    dataSource.name = "Test Data Source";

    array[dataSourceUuid] = dataSource;
}

void RouteOdTripsFixtureTests::setupOdTrips() {
    std::vector<std::unique_ptr<TrRouting::OdTrip>>& array = calculator.odTrips;
    std::map<boost::uuids::uuid, int>& arrayIndexesByUuid = calculator.odTripIndexesByUuid;

    std::vector<int> originNodesIdx;
    originNodesIdx.push_back(calculator.nodeIndexesByUuid[nodeSouth2Uuid]);
    std::vector<int> originNodesTravelTimesSeconds;
    originNodesTravelTimesSeconds.push_back(469);
    std::vector<int> originNodesDistancesMeters;
    originNodesDistancesMeters.push_back(500);        
    std::vector<int> destinationNodesIdx;
    destinationNodesIdx.push_back(calculator.nodeIndexesByUuid[nodeMidNodeUuid]);
    std::vector<int> destinationNodesTravelTimesSeconds;
    destinationNodesTravelTimesSeconds.push_back(138);
    std::vector<int> destinationNodesDistancesMeters;
    destinationNodesDistancesMeters.push_back(150);

    std::unique_ptr<TrRouting::OdTrip> odTrip = std::make_unique<TrRouting::OdTrip>(odTripUuid,
                                                                                    12345,
                                                                                    "12345",
                                                                                    calculator.dataSources.at(dataSourceUuid),
                                                                                    -1,
                                                                                    getTimeInSeconds(9, 45),
                                                                                    -1,
                                                                                    0,
                                                                                    0,
                                                                                    0,
                                                                                    1.0,
                                                                                    "",
                                                                                    "",
                                                                                    "",
                                                                                    originNodesIdx,
                                                                                    originNodesTravelTimesSeconds,
                                                                                    originNodesDistancesMeters,
                                                                                    destinationNodesIdx,
                                                                                    destinationNodesTravelTimesSeconds,
                                                                                    destinationNodesDistancesMeters,
                                                                                    std::make_unique<TrRouting::Point>(45.5242, -73.5817),
                                                                                    std::make_unique<TrRouting::Point>(45.54, -73.6146));
    arrayIndexesByUuid[odTrip->uuid] = array.size();
    array.push_back(std::move(odTrip));
}

// Simple test to make sure this code path still works
TEST_F(RouteOdTripsFixtureTests, NoOdTripsQuery)
{
    std::vector<std::string> parametersWithValues = initializeParameters();

    nlohmann::json result = calculateOdTrips(parametersWithValues);
    ASSERT_EQ(1, result["odTrips"].size());
    ASSERT_EQ(STATUS_SUCCESS, result["odTrips"][0]["status"]);
}

void RouteOdTripsFixtureTests::SetUp() {
    BaseCsaFixtureTests::SetUp();

    // Setup one od trip and data source for the test
    setupDataSources();
    setupOdTrips();
}

std::vector<std::string> RouteOdTripsFixtureTests::initializeParameters()
{
    std::vector<std::string> parametersWithValues = RouteCalculationFixtureTests::initializeParameters();
    parametersWithValues.push_back("od_trips=1");
    return parametersWithValues;
}

nlohmann::json RouteOdTripsFixtureTests::calculateOdTrips(std::vector<std::string> parameters)
{
    calculator.params.setDefaultValues();
    TrRouting::RouteParameters routeParams = calculator.params.update(parameters,
        calculator.scenarioIndexesByUuid,
        calculator.scenarios,
        calculator.odTripIndexesByUuid,
        calculator.odTrips,
        calculator.nodeIndexesByUuid,
        calculator.nodes,
        calculator.dataSources);
    TrRouting::OsrmFetcher::birdDistanceAccessibilityEnabled = true;

    // TODO Shouldn't need to do this, but we do for now, benchmark needs to be started
    calculator.algorithmCalculationTime.start();
    calculator.benchmarking.clear();

    std::string result =  calculator.odTripsRouting(routeParams);
    nlohmann::json json;
    nlohmann::json jsonResult = json.parse(result);
    return jsonResult;
}
