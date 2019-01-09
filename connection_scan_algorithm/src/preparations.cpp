#include "calculator.hpp"

namespace TrRouting
{
    
  void Calculator::prepare()
  {
    
    if (params.debugDisplay)
      std::cerr << "preparing nodes, routes, trips, connections and footpaths..." << std::endl;
    if (params.dataFetcherShortname == "cache")
    {
      std::tie(modes,     modeIndexesByShortname) = params.cacheFetcher->getModes();
      std::tie(services,  serviceIndexesByUuid)   = params.cacheFetcher->getServices(params);
      std::tie(scenarios, scenarioIndexesByUuid)  = params.cacheFetcher->getScenarios(serviceIndexesByUuid, params);
      std::tie(stations,  stationIndexesByUuid)   = params.cacheFetcher->getStations(params);
      std::tie(nodes,     nodeIndexesByUuid)      = params.cacheFetcher->getNodes(stationIndexesByUuid, params);
               nodes                              = params.cacheFetcher->getNodeFootpaths(nodes, nodeIndexesByUuid, params);
      std::tie(agencies, agencyIndexesByUuid)     = params.cacheFetcher->getAgencies(params);
      std::tie(lines,    lineIndexesByUuid)       = params.cacheFetcher->getLines(agencyIndexesByUuid, modeIndexesByShortname, params);
      std::tie(paths,    pathIndexesByUuid)       = params.cacheFetcher->getPaths(lineIndexesByUuid, nodeIndexesByUuid, params);
      
      std::tie(trips, tripIndexesByUuid, blocks, blockIndexesByUuid, forwardConnections, reverseConnections) = params.cacheFetcher->getTripsAndConnections(agencyIndexesByUuid, lines, lineIndexesByUuid, pathIndexesByUuid, nodeIndexesByUuid, serviceIndexesByUuid, params);

      /*for (auto & node : nodes)
      {
        std::cout << node.toString() << std::endl;
        for (int transferableNodeIdx : node.transferableNodesIdx)
        {
          std::cout << "    " << transferableNodeIdx << " (" << nodes[transferableNodeIdx].uuid << ")" << std::endl;
        }
        std::cout << std::endl;
      }*/
      
      //if (params.updateOdTrips == 1) // only update od trips if set as parameter (1) when launching app, because this takes a long time. Call only if nodes and/or od trips were modified.
      //{
      //  std::tie(odTrips, odTripIndexesById, odTripFootpaths) = params.databaseFetcher->getOdTrips(params.projectShortname, nodes, params);
      //}
      //else
      //{
      //  std::tie(odTrips, odTripIndexesById)             = params.cacheFetcher->getOdTrips(params.projectShortname, nodes, params);
      //  odTripFootpaths                                  = params.cacheFetcher->getOdTripFootpaths(params.projectShortname, params);
      //}
      
      //std::tie(forwardConnections, reverseConnections)   = params.cacheFetcher->getConnections(params.projectShortname, nodeIndexesById, tripIndexesById);
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
