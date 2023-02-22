#include "csa_test_data_fetcher.hpp"

TestDataFetcher::TestDataFetcher() {

}
TestDataFetcher::~TestDataFetcher() {
}

 
const std::map<std::string, TrRouting::Mode> TestDataFetcher::getModes() {
  std::map<std::string, TrRouting::Mode> modes;
  
  modes.emplace("bus", TrRouting::Mode("bus", "Bus", 3, 700));

  return modes;
}

int TestDataFetcher::getDataSources(
                                    std::map<boost::uuids::uuid, TrRouting::DataSource>& ts,
                                    std::string
                                    ) {
  TrRouting::DataSource dataSource;
  dataSource.uuid = dataSourceUuid;
  dataSource.shortname = "testDS";
  dataSource.name = "Test Data Source";
  
  ts[dataSourceUuid] = dataSource;

  return 0;
}

    int TestDataFetcher::getPersons(
      std::map<boost::uuids::uuid, TrRouting::Person>&,
      const std::map<boost::uuids::uuid, TrRouting::DataSource>&,
      std::string
    )
    {
      // No person in this test
      return 0;
    }

int TestDataFetcher::getOdTrips(
                                std::map<boost::uuids::uuid, TrRouting::OdTrip>& ts,
                                const std::map<boost::uuids::uuid, TrRouting::DataSource>& dataSources,
                                const std::map<boost::uuids::uuid, TrRouting::Person>&,
                                const std::map<boost::uuids::uuid, TrRouting::Node>& nodes,
                                std::string
                                ) {
  std::vector<TrRouting::NodeTimeDistance> originNodes;
  originNodes.push_back(TrRouting::NodeTimeDistance(nodes.at(nodeSouth2Uuid), 469, 500));
  std::vector<TrRouting::NodeTimeDistance> destinationNodes;
  destinationNodes.push_back(TrRouting::NodeTimeDistance(nodes.at(nodeMidNodeUuid), 138, 150));
  
  ts.emplace(odTripUuid, TrRouting::OdTrip(odTripUuid,
                                              12345,
                                              "12345",
                                              dataSources.at(dataSourceUuid),
                                              std::nullopt,
                                              getTimeInSeconds(9, 45),
                                              -1,
                                              0,
                                              0,
                                              0,
                                              1.0,
                                              "",
                                              "",
                                              "",
                                              originNodes,
                                              destinationNodes,
                                              std::make_unique<TrRouting::Point>(45.5242, -73.5817),
                                              std::make_unique<TrRouting::Point>(45.54, -73.6146)));
  
  return 0;
}

int TestDataFetcher::getAgencies(
                                 std::map<boost::uuids::uuid, TrRouting::Agency>& ts,
                                 std::string
                                 ) {
  
  TrRouting::Agency agency;
  agency.uuid = agencyUuid;
  agency.name = "Unit Test Agency";
  agency.acronym = "UT";
  ts[agencyUuid] = agency;
  return 0;
}

int TestDataFetcher::getServices(
                                 std::map<boost::uuids::uuid, TrRouting::Service>& ts,
                                 std::string
                                 ) {

  
  TrRouting::Service service;
  service.uuid = serviceUuid;
  service.name = "Single Unit Test";
  
  ts[serviceUuid] = service;
  
  return 0;
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

int TestDataFetcher::getNodes(
                              std::map<boost::uuids::uuid, TrRouting::Node>& array,
                              std::string
                              ) {

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
                                                "North2",
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

  return 0;
}

int TestDataFetcher::getLines(
                              std::map<boost::uuids::uuid, TrRouting::Line>& ts,
                              const std::map<boost::uuids::uuid, TrRouting::Agency>& agencies,
                              const std::map<std::string, TrRouting::Mode>& modes,
                              std::string
                              ) {
  
  auto & busMode = modes.at("bus");
  const auto & defaultAgency = agencies.at(agencyUuid);

  ts.emplace(lineSNUuid, TrRouting::Line(lineSNUuid, defaultAgency, busMode, "01", "South/North", "", 0));

  ts.emplace(lineEWUuid, TrRouting::Line(lineEWUuid, defaultAgency, busMode, "02", "East/West", "", 0));
  
  ts.emplace(lineExtraUuid, TrRouting::Line(lineExtraUuid, defaultAgency, busMode, "03", "Extra", "", 0));
  return 0;
}

void addNodeToPath(std::vector<TrRouting::NodeTimeDistance>& nodesref, const TrRouting::Node &node, int timeTraveled, int distance) {
  nodesref.push_back(TrRouting::NodeTimeDistance(node,timeTraveled, distance));
}

int TestDataFetcher::getPaths(
                              std::map<boost::uuids::uuid, TrRouting::Path>& ts,
                              const std::map<boost::uuids::uuid, TrRouting::Line>& lines,
                              const std::map<boost::uuids::uuid, TrRouting::Node>& nodes,
                              std::string
                              ) {

  std::vector<std::reference_wrapper<const TrRouting::Trip>> emptyVector;
  std::vector<TrRouting::NodeTimeDistance> nodesref;
  addNodeToPath(nodesref, nodes.at(nodeSouth2Uuid), 210, 1186);
  addNodeToPath(nodesref, nodes.at(nodeSouth1Uuid), 190, 1160);
  addNodeToPath(nodesref, nodes.at(nodeMidNodeUuid), 180, 980);
  addNodeToPath(nodesref, nodes.at(nodeNorth1Uuid), 270, 1544);
  addNodeToPath(nodesref, nodes.at(nodeNorth2Uuid), -1, -1);
  ts.emplace(pathSNUuid, TrRouting::Path(pathSNUuid,
                                         lines.at(lineSNUuid),
                                         "outbound",
                                         "",
                                         nodesref,
                                         emptyVector));
  // Path's trip data will be filled in the setUpSchedules
  
  nodesref.clear();
  addNodeToPath(nodesref, nodes.at(nodeEast2Uuid), 150, 1025);
  addNodeToPath(nodesref, nodes.at(nodeEast1Uuid), 110, 510);
  addNodeToPath(nodesref, nodes.at(nodeMidNodeUuid), 150, 498);
  addNodeToPath(nodesref, nodes.at(nodeWest1Uuid), 120, 668);
  addNodeToPath(nodesref, nodes.at(nodeWest2Uuid), -1, -1);
  ts.emplace(pathEWUuid, TrRouting::Path(pathEWUuid,
                                         lines.at(lineEWUuid),
                                         "outbound",
                                         "",
                                         nodesref,
                                         emptyVector));
  // Path's trip data will be filled in the setUpSchedules

  nodesref.clear();
  addNodeToPath(nodesref, nodes.at(nodeEast1Uuid), 300, 1760);
  addNodeToPath(nodesref, nodes.at(nodeExtra1Uuid), -1, -1);
  ts.emplace(pathExtraUuid, TrRouting::Path(pathExtraUuid,
                                            lines.at(lineExtraUuid),
                                            "outbound",
                                            "",
                                            nodesref,
                                            emptyVector));
  // Path's trip data will be filled in the setUpSchedules
  return 0;
}

int TestDataFetcher::getScenarios(
                                  std::map<boost::uuids::uuid, TrRouting::Scenario>& array,
                                  const std::map<boost::uuids::uuid, TrRouting::Service>& services,
                                  const std::map<boost::uuids::uuid, TrRouting::Line>& lines,
                                  const std::map<boost::uuids::uuid, TrRouting::Agency>&,
                                  const std::map<boost::uuids::uuid, TrRouting::Node>&,
                                  const std::map<std::string, TrRouting::Mode>&,
                                  std::string
                                  ) {
  array[scenarioUuid].uuid = scenarioUuid;
  array[scenarioUuid].name = "Test valid scenario";
  array[scenarioUuid].servicesList.push_back(services.at(serviceUuid));

  array[scenario2Uuid].uuid = scenario2Uuid;
  array[scenario2Uuid].name = "Test valid without one line";
  array[scenario2Uuid].servicesList.push_back(services.at(serviceUuid));
  array[scenario2Uuid].exceptLines.push_back(lines.at(lineEWUuid));
  
  return 0;
}



void addTripData(TrRouting::Trip & trip, TrRouting::Path & path, std::vector<std::shared_ptr<TrRouting::Connection>>& connections, int arrivalTimes[], int departureTimes[], int arraySize)
{
    path.tripsRef.push_back(trip);

    trip.connectionDepartureTimes.resize(arraySize);

    for (int nodeTimeI = 0; nodeTimeI < arraySize - 1; nodeTimeI++) {
      std::shared_ptr<TrRouting::Connection> forwardConnection(std::make_shared<TrRouting::Connection>(TrRouting::Connection(
            path.nodesRef[nodeTimeI],
            path.nodesRef[nodeTimeI + 1],
            departureTimes[nodeTimeI],
            arrivalTimes[nodeTimeI + 1],
            trip,
            true,
            true,
            nodeTimeI + 1,
            trip.allowSameLineTransfers,
            -1
        )));

        connections.push_back(std::move(forwardConnection));

        trip.connectionDepartureTimes[nodeTimeI] = departureTimes[nodeTimeI];

    }

}
// TODO: This is error prone, the procedure for the trip data (including the
// addTripData function above) was taken from
// trips_and_connection_cache_fetcher.cpp. This function should be split in
// smaller functions which could then be re-used by this test case. But before
// refactoring, let's add some tests! It's the chickend or the egg!
int TestDataFetcher::getSchedules(
                                  std::map<boost::uuids::uuid, TrRouting::Trip>& array,
                                  const std::map<boost::uuids::uuid, TrRouting::Line>& lines,
                                  std::map<boost::uuids::uuid, TrRouting::Path>& paths,
                                  const std::map<boost::uuids::uuid, TrRouting::Service>& services,
                                  std::vector<std::shared_ptr<TrRouting::Connection>>& connections,
                                  std::string
                                  ) {

  //TODO This could be internal to the Trip object, just copy the mode from the line object
  auto & busMode = lines.at(lineSNUuid).mode;

  // South/North trip 1 at 10
  array.emplace(trip1SNUuid, TrRouting::Trip(trip1SNUuid,
                                             lines.at(lineSNUuid).agency,
                                             lines.at(lineSNUuid),
                                             paths.at(pathSNUuid),
                                             busMode,
                                             services.at(serviceUuid),
                                             -1,
                                             0));
  int arrivalTimesT1[5] = { getTimeInSeconds(10), getTimeInSeconds(10, 3, 30), getTimeInSeconds(10, 7), getTimeInSeconds(10, 13), getTimeInSeconds(10, 18) };
  int departureTimesT1[5] = { getTimeInSeconds(10), getTimeInSeconds(10, 3, 50), getTimeInSeconds(10, 10), getTimeInSeconds(10, 13, 30), getTimeInSeconds(10, 18) };

  addTripData(array.at(trip1SNUuid), paths.at(pathSNUuid), connections, arrivalTimesT1, departureTimesT1, 5);

  // South/North trip 2 at 11
  array.emplace(trip2SNUuid,  TrRouting::Trip(trip2SNUuid,
                                              lines.at(lineSNUuid).agency,
                                              lines.at(lineSNUuid),
                                              paths.at(pathSNUuid),
                                              busMode,
                                              services.at(serviceUuid),
                                              -1,
                                              0));
  int arrivalTimesT2[5] = { getTimeInSeconds(11), getTimeInSeconds(11, 3, 30), getTimeInSeconds(11, 7), getTimeInSeconds(11, 11), getTimeInSeconds(11, 16) };
  int departureTimesT2[5] = { getTimeInSeconds(11), getTimeInSeconds(11, 3, 50), getTimeInSeconds(11, 8), getTimeInSeconds(11, 11, 30), getTimeInSeconds(11, 17) };
  
  addTripData(array.at(trip2SNUuid), paths.at(pathSNUuid),  connections, arrivalTimesT2, departureTimesT2, 5);

  // East/West trip 1 at 9
  array.emplace(trip1EWUuid,TrRouting::Trip(trip1EWUuid,
                                            lines.at(lineSNUuid).agency,                                            
                                            lines.at(lineEWUuid),
                                            paths.at(pathEWUuid),
                                            busMode,
                                            services.at(serviceUuid),
                                            -1,
                                            0));
  
  int arrivalTimesT3[5] = { getTimeInSeconds(9), getTimeInSeconds(9, 2, 30), getTimeInSeconds(9, 4, 40), getTimeInSeconds(9, 7, 30), getTimeInSeconds(9, 10) };
  int departureTimesT3[5] = { getTimeInSeconds(9), getTimeInSeconds(9, 2, 50), getTimeInSeconds(9, 5), getTimeInSeconds(9, 8), getTimeInSeconds(9, 11) };
  
  addTripData(array.at(trip1EWUuid), paths.at(pathEWUuid), connections, arrivalTimesT3, departureTimesT3, 5);
  //array.push_back(std::move(ewTrip1));
  
  // East/West trip 2 at 10:02
  //std::unique_ptr<TrRouting::Trip> ewTrip2 = std::make_unique<TrRouting::Trip>(trip2EWUuid,
  array.emplace(trip2EWUuid, TrRouting::Trip(trip2EWUuid,
                                             lines.at(lineSNUuid).agency,
                                             lines.at(lineEWUuid),
                                             paths.at(pathEWUuid),
                                             busMode,
                                             services.at(serviceUuid),
                                             -1,
                                             0));
  
  int arrivalTimesT4[5] = { getTimeInSeconds(10, 2), getTimeInSeconds(10, 4, 30), getTimeInSeconds(10, 7), getTimeInSeconds(10, 11, 30), getTimeInSeconds(10, 14) };
  int departureTimesT4[5] = { getTimeInSeconds(10, 2), getTimeInSeconds(10, 5, 10), getTimeInSeconds(10, 9), getTimeInSeconds(10, 12), getTimeInSeconds(10, 15) };
  
  addTripData(array.at(trip2EWUuid), paths.at(pathEWUuid), connections, arrivalTimesT4, departureTimesT4, 5);
  
  // Extra trip at 10h20
  array.emplace(trip1ExtraUuid, TrRouting::Trip(trip1ExtraUuid,
                                                lines.at(lineSNUuid).agency,
                                                lines.at(lineExtraUuid),
                                                paths.at(pathExtraUuid),
                                                busMode,
                                                services.at(serviceUuid),
                                                -1,
                                                0));
  
  int arrivalTimesT5[2] = { getTimeInSeconds(10, 20), getTimeInSeconds(10, 25) };
  int departureTimesT5[2] = { getTimeInSeconds(10, 20), getTimeInSeconds(10,26) };
  
  addTripData(array.at(trip1ExtraUuid), paths.at(pathExtraUuid), connections, arrivalTimesT5, departureTimesT5, 2);
  
  return 0;
}
