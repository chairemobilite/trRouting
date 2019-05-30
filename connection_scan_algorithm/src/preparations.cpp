#include "calculator.hpp"

namespace TrRouting
{
  
  void Calculator::updateAgenciesFromCache(Parameters& params, std::string customPath)
  {
    params.cacheFetcher->getAgencies(agencies, agencyIndexesByUuid, params, customPath);
  }

  void Calculator::updateDataSourcesFromCache(Parameters& params, std::string customPath)
  {
    params.cacheFetcher->getDataSources(dataSources, dataSourceIndexesByUuid, params, customPath);
  }

  void Calculator::updateHouseholdsFromCache(Parameters& params, std::string customPath)
  {
    params.cacheFetcher->getHouseholds(households, householdIndexesByUuid, dataSourceIndexesByUuid, params, customPath);
  }

  void Calculator::updatePersonsFromCache(Parameters& params, std::string customPath)
  {
    params.cacheFetcher->getPersons(persons, personIndexesByUuid, dataSourceIndexesByUuid, householdIndexesByUuid, params, customPath);
  }

  void Calculator::prepare()
  {
    
    if (params.debugDisplay)
      std::cerr << "preparing nodes, routes, trips, connections and footpaths..." << std::endl;
    if (params.dataFetcherShortname == "cache")
    {
      std::tie(modes, modeIndexesByShortname) = params.cacheFetcher->getModes();
      updateDataSourcesFromCache(params);
      updateHouseholdsFromCache(params);
      updatePersonsFromCache(params);
      std::tie(odTrips,     odTripIndexesByUuid)     = params.cacheFetcher->getOdTrips(dataSourceIndexesByUuid, householdIndexesByUuid, personIndexesByUuid, params);
      //std::tie(places,      placeIndexesByUuid)      = params.cacheFetcher->getPlaces(dataSourceIndexesByUuid, params);
      std::tie(services,    serviceIndexesByUuid)    = params.cacheFetcher->getServices(params);
      //std::tie(stations,    stationIndexesByUuid)    = params.cacheFetcher->getStations(params);
      std::tie(nodes,       nodeIndexesByUuid)       = params.cacheFetcher->getNodes(stationIndexesByUuid, params);
               nodes                                 = params.cacheFetcher->getNodeFootpaths(nodes, nodeIndexesByUuid, params);
      //std::tie(agencies,    agencyIndexesByUuid)     = params.cacheFetcher->getAgencies(params);
      updateAgenciesFromCache(params);
      std::tie(lines,       lineIndexesByUuid)       = params.cacheFetcher->getLines(agencyIndexesByUuid, modeIndexesByShortname, params);
      std::tie(paths,       pathIndexesByUuid)       = params.cacheFetcher->getPaths(lineIndexesByUuid, nodeIndexesByUuid, params);
      std::tie(scenarios,   scenarioIndexesByUuid)   = params.cacheFetcher->getScenarios(serviceIndexesByUuid, lineIndexesByUuid, agencyIndexesByUuid, nodeIndexesByUuid, modeIndexesByShortname, params);
      
      std::tie(trips, tripIndexesByUuid, tripConnectionDepartureTimes, tripConnectionDemands, blocks, blockIndexesByUuid, forwardConnections, reverseConnections) = params.cacheFetcher->getTripsAndConnections(agencyIndexesByUuid, lines, lineIndexesByUuid, paths, pathIndexesByUuid, nodeIndexesByUuid, serviceIndexesByUuid, params);

      std::cout << forwardConnections.size() << " connections" << std::endl; 

      int benchmarkingStart = algorithmCalculationTime.getEpoch();

      int lastConnectionIndex = forwardConnections.size() - 1;

      forwardConnectionsIndexPerDepartureTimeHour = std::vector<int>(32, -1);
      reverseConnectionsIndexPerArrivalTimeHour   = std::vector<int>(32, lastConnectionIndex);
      
      int hour {0};
      int i = 0;
      for (auto & connection : forwardConnections)
      {
        while (std::get<connectionIndexes::TIME_DEP>(connection) >= hour * 3600 && forwardConnectionsIndexPerDepartureTimeHour[hour] == -1 && hour < 32)
        {
          forwardConnectionsIndexPerDepartureTimeHour[hour] = i;
          //std::cout << hour << ":" << i << ":" << std::get<connectionIndexes::TIME_DEP>(connection) << std::endl;
          hour++;
        }
        i++;
      }

      hour = 31;
      i = 0;
      for (auto & connection : reverseConnections)
      {
        while (std::get<connectionIndexes::TIME_ARR>(connection) <= hour * 3600 && reverseConnectionsIndexPerArrivalTimeHour[hour] == lastConnectionIndex && hour >= 0)
        {
          reverseConnectionsIndexPerArrivalTimeHour[hour] = i;
          //std::cout << hour << ":" << i << ":" << std::get<connectionIndexes::TIME_ARR>(connection) << std::endl;
          hour--;
        }
        i++;
      }

      for (int h = 0; h < 32; h++)
      {
        if (forwardConnectionsIndexPerDepartureTimeHour[h] == -1)
        {
          forwardConnectionsIndexPerDepartureTimeHour[h] = lastConnectionIndex;
        }
        //std::cout << h << ": " << forwardConnectionsIndexPerDepartureTimeHour[h] << std::endl;
      }
      /*for (int h = 0; h < 32; h++)
      {
        std::cout << h << ": " << reverseConnectionsIndexPerArrivalTimeHour[h] << std::endl;
      }*/

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
