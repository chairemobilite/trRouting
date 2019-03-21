#include "calculator.hpp"

namespace TrRouting
{
    
  void Calculator::prepare()
  {
    
    if (params.debugDisplay)
      std::cerr << "preparing nodes, routes, trips, connections and footpaths..." << std::endl;
    if (params.dataFetcherShortname == "cache")
    {
      std::tie(modes,       modeIndexesByShortname)  = params.cacheFetcher->getModes();
      std::tie(dataSources, dataSourceIndexesByUuid) = params.cacheFetcher->getDataSources(params);
      std::tie(households,  householdIndexesByUuid)  = params.cacheFetcher->getHouseholds(dataSourceIndexesByUuid, params);
      std::tie(persons,     personIndexesByUuid)     = params.cacheFetcher->getPersons(dataSourceIndexesByUuid, householdIndexesByUuid, params);
      std::tie(odTrips,     odTripIndexesByUuid)     = params.cacheFetcher->getOdTrips(dataSourceIndexesByUuid, householdIndexesByUuid, personIndexesByUuid, params);
      std::tie(places,      placeIndexesByUuid)      = params.cacheFetcher->getPlaces(dataSourceIndexesByUuid, params);
      std::tie(services,    serviceIndexesByUuid)    = params.cacheFetcher->getServices(params);
      std::tie(scenarios,   scenarioIndexesByUuid)   = params.cacheFetcher->getScenarios(serviceIndexesByUuid, params);
      std::tie(stations,    stationIndexesByUuid)    = params.cacheFetcher->getStations(params);
      std::tie(nodes,       nodeIndexesByUuid)       = params.cacheFetcher->getNodes(stationIndexesByUuid, params);
               nodes                                 = params.cacheFetcher->getNodeFootpaths(nodes, nodeIndexesByUuid, params);
      std::tie(agencies,    agencyIndexesByUuid)     = params.cacheFetcher->getAgencies(params);
      std::tie(lines,       lineIndexesByUuid)       = params.cacheFetcher->getLines(agencyIndexesByUuid, modeIndexesByShortname, params);
      std::tie(paths,       pathIndexesByUuid)       = params.cacheFetcher->getPaths(lineIndexesByUuid, nodeIndexesByUuid, params);
      
      std::tie(trips, tripIndexesByUuid, blocks, blockIndexesByUuid, forwardConnections, reverseConnections) = params.cacheFetcher->getTripsAndConnections(agencyIndexesByUuid, lines, lineIndexesByUuid, paths, pathIndexesByUuid, nodeIndexesByUuid, serviceIndexesByUuid, params);

      //for (auto & connection : forwardConnections)
      //{
      //  forwardConnectionsDepartureNodeIndexes.push_back(std::get<connectionIndexes::NODE_DEP>(connection));
      //  forwardConnectionsArrivalNodeIndexes.push_back(std::get<connectionIndexes::NODE_ARR>(connection));
      //  forwardConnectionsDepartureTimesSeconds.push_back(std::get<connectionIndexes::TIME_DEP>(connection));
      //  forwardConnectionsArrivalTimesSeconds.push_back(std::get<connectionIndexes::TIME_ARR>(connection));
      //  forwardConnectionsTripIndexes.push_back(std::get<connectionIndexes::TRIP>(connection));
      //  forwardConnectionsCanBoards.push_back(std::get<connectionIndexes::CAN_BOARD>(connection));
      //  forwardConnectionsCanUnboards.push_back(std::get<connectionIndexes::CAN_UNBOARD>(connection));
      //  forwardConnectionsSequences.push_back(std::get<connectionIndexes::SEQUENCE>(connection));
      //  forwardConnectionsLineIndexes.push_back(std::get<connectionIndexes::LINE>(connection));
      //  forwardConnectionsBlockIndexes.push_back(std::get<connectionIndexes::BLOCK>(connection));
      //  forwardConnectionsCanTransferSameLines.push_back(std::get<connectionIndexes::CAN_TRANSFER_SAME_LINE>(connection));
      //}
     

      /*for (auto & node : nodes)
      {
        std::cout << node.toString() << std::endl;
        for (int transferableNodeIdx : node.transferableNodesIdx)
        {
          std::cout << "    " << transferableNodeIdx << " (" << nodes[transferableNodeIdx].uuid << ")" << std::endl;
        }
        std::cout << std::endl;
      }*/
      
    }
    else if (params.dataFetcherShortname == "gtfs")
    {
      // not yet implemented
    }
    else if (params.dataFetcherShortname == "csv")
    {
      // not yet implemented
    }
    
    std::cout << "preparing nodes tentative times, trips enter connections and journeys..." << std::endl;
    
    nodesTentativeTime                     = std::vector<int>(nodes.size());
    nodesReverseTentativeTime              = std::vector<int>(nodes.size());
    nodesAccessTravelTime                  = std::vector<int>(nodes.size());
    nodesEgressTravelTime                  = std::vector<int>(nodes.size());
    tripsEnterConnection                   = std::vector<int>(trips.size());
    tripsExitConnection                    = std::vector<int>(trips.size());
    tripsEnterConnectionTransferTravelTime = std::vector<int>(trips.size());
    tripsExitConnectionTransferTravelTime  = std::vector<int>(trips.size());
    tripsEnabled                           = std::vector<int>(trips.size());
    tripsUsable                            = std::vector<int>(trips.size());
    forwardJourneys                        = std::vector<std::tuple<int,int,int,int,int,short>>(nodes.size());
    forwardEgressJourneys                  = std::vector<std::tuple<int,int,int,int,int,short>>(nodes.size());
    reverseJourneys                        = std::vector<std::tuple<int,int,int,int,int,short>>(nodes.size());
    reverseAccessJourneys                  = std::vector<std::tuple<int,int,int,int,int,short>>(nodes.size());
    
    
  }
  
}
