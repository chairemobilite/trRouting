#include <errno.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "gtest/gtest.h"
#include "calculator.hpp"
#include "csa_test_base.hpp"
#include "csa_v1_simple_calculation_test.hpp"
#include "constants.hpp"
#include "od_trip.hpp"
#include "data_source.hpp"

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
    std::vector<std::unique_ptr<TrRouting::DataSource>>& array = calculator.dataSources;
    std::map<boost::uuids::uuid, int>& arrayIndexesByUuid = calculator.dataSourceIndexesByUuid;

    std::unique_ptr<TrRouting::DataSource> dataSource = std::make_unique<TrRouting::DataSource>();
    dataSource->uuid = dataSourceUuid;
    dataSource->shortname = "testDS";
    dataSource->name = "Test Data Source";

    arrayIndexesByUuid[dataSource->uuid] = array.size();
    array.push_back(std::move(dataSource));
}

void RouteOdTripsFixtureTests::setupOdTrips() {
    std::vector<std::unique_ptr<TrRouting::OdTrip>>& array = calculator.odTrips;
    std::map<boost::uuids::uuid, int>& arrayIndexesByUuid = calculator.odTripIndexesByUuid;

    std::unique_ptr<TrRouting::OdTrip> odTrip = std::make_unique<TrRouting::OdTrip>();
    odTrip->uuid = odTripUuid;
    odTrip->dataSourceIdx = calculator.dataSourceIndexesByUuid[dataSourceUuid];
    odTrip->departureTimeSeconds = getTimeInSeconds(9, 45);
    odTrip->origin = std::make_unique<TrRouting::Point>(45.5242, -73.5817);
    odTrip->destination = std::make_unique<TrRouting::Point>(45.54, -73.6146);
    odTrip->originNodesIdx.push_back(calculator.nodeIndexesByUuid[nodeSouth2Uuid]);
    odTrip->originNodesTravelTimesSeconds.push_back(469);
    odTrip->originNodesDistancesMeters.push_back(500);
    odTrip->destinationNodesIdx.push_back(calculator.nodeIndexesByUuid[nodeMidNodeUuid]);
    odTrip->destinationNodesTravelTimesSeconds.push_back(138);
    odTrip->destinationNodesDistancesMeters.push_back(150);

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
        calculator.dataSourceIndexesByUuid);
    calculator.params.birdDistanceAccessibilityEnabled = true;

    // TODO Shouldn't need to do this, but we do for now, benchmark needs to be started
    calculator.algorithmCalculationTime.start();
    calculator.benchmarking.clear();

    std::string result =  calculator.odTripsRouting(routeParams);
    nlohmann::json json;
    nlohmann::json jsonResult = json.parse(result);
    return jsonResult;
}
