#include "calculator.hpp"

namespace TrRouting
{

  /*void Calculator::updateStationsFromCache(Parameters& params, std::string customPath)
  {
    params.cacheFetcher->getStations(stations, stationIndexesByUuid, params, customPath);
  }*/

  /*void Calculator::updateStopsFromCache(Parameters& params, std::string customPath)
  {
    params.cacheFetcher->getStops(stops, stopIndexesByUuid, nodeIndexesByUuid, params, customPath);
  }*/


  void Calculator::updateNodesFromCache(Parameters& params, std::string customPath)
  {
    params.cacheFetcher->getNodes(nodes, nodeIndexesByUuid, stationIndexesByUuid, params, customPath);

    nodesTentativeTime.clear();
    nodesTentativeTime.shrink_to_fit();
    nodesTentativeTime.resize(nodes.size());
    nodesReverseTentativeTime.clear();
    nodesReverseTentativeTime.shrink_to_fit();
    nodesReverseTentativeTime.resize(nodes.size());
    nodesAccessTravelTime.clear();
    nodesAccessTravelTime.shrink_to_fit();
    nodesAccessTravelTime.resize(nodes.size());
    nodesAccessDistance.clear();
    nodesAccessDistance.shrink_to_fit();
    nodesAccessDistance.resize(nodes.size());
    nodesEgressTravelTime.clear();
    nodesEgressTravelTime.shrink_to_fit();
    nodesEgressTravelTime.resize(nodes.size());
    nodesEgressDistance.clear();
    nodesEgressDistance.shrink_to_fit();
    nodesEgressDistance.resize(nodes.size());
    forwardJourneysSteps.clear();
    forwardJourneysSteps.shrink_to_fit();
    forwardJourneysSteps.resize(nodes.size());
    forwardEgressJourneysSteps.clear();
    forwardEgressJourneysSteps.shrink_to_fit();
    forwardEgressJourneysSteps.resize(nodes.size());
    reverseJourneysSteps.clear();
    reverseJourneysSteps.shrink_to_fit();
    reverseJourneysSteps.resize(nodes.size());
    reverseAccessJourneysSteps.clear();
    reverseAccessJourneysSteps.shrink_to_fit();
    reverseAccessJourneysSteps.resize(nodes.size());
  }

  void Calculator::updateDataSourcesFromCache(Parameters& params, std::string customPath)
  {
    params.cacheFetcher->getDataSources(dataSources, dataSourceIndexesByUuid, params, customPath);
  }

  void Calculator::updateHouseholdsFromCache(Parameters& params, std::string customPath)
  {
    params.cacheFetcher->getHouseholds(households, householdIndexesByUuid, dataSourceIndexesByUuid, nodeIndexesByUuid, params, customPath);
  }

  void Calculator::updatePersonsFromCache(Parameters& params, std::string customPath)
  {
    params.cacheFetcher->getPersons(persons, personIndexesByUuid, dataSourceIndexesByUuid, householdIndexesByUuid, nodeIndexesByUuid, params, customPath);
  }
  
  void Calculator::updateOdTripsFromCache(Parameters& params, std::string customPath)
  {
    params.cacheFetcher->getOdTrips(odTrips, odTripIndexesByUuid, dataSourceIndexesByUuid, householdIndexesByUuid, personIndexesByUuid, nodeIndexesByUuid, params, customPath);
  }

  void Calculator::updatePlacesFromCache(Parameters& params, std::string customPath)
  {
    params.cacheFetcher->getPlaces(places, placeIndexesByUuid, dataSourceIndexesByUuid, nodeIndexesByUuid, params, customPath);
  }

  void Calculator::updateAgenciesFromCache(Parameters& params, std::string customPath)
  {
    params.cacheFetcher->getAgencies(agencies, agencyIndexesByUuid, params, customPath);
  }

  void Calculator::updateServicesFromCache(Parameters& params, std::string customPath)
  {
    params.cacheFetcher->getServices(services, serviceIndexesByUuid, params, customPath);
  }

  

  void Calculator::updateLinesFromCache(Parameters& params, std::string customPath)
  {
    params.cacheFetcher->getLines(lines, lineIndexesByUuid, agencyIndexesByUuid, modeIndexesByShortname, params, customPath);
  }

  void Calculator::updatePathsFromCache(Parameters& params, std::string customPath)
  {
    params.cacheFetcher->getPaths(paths, pathIndexesByUuid, lineIndexesByUuid, nodeIndexesByUuid, params, customPath);
  }

  void Calculator::updateScenariosFromCache(Parameters& params, std::string customPath)
  {
    params.cacheFetcher->getScenarios(scenarios, scenarioIndexesByUuid, serviceIndexesByUuid, lineIndexesByUuid, agencyIndexesByUuid, nodeIndexesByUuid, modeIndexesByShortname, params, customPath);
  }

  void Calculator::updateNetworksFromCache(Parameters& params, std::string customPath)
  {
    params.cacheFetcher->getNetworks(networks, networkIndexesByUuid, agencyIndexesByUuid, serviceIndexesByUuid, scenarioIndexesByUuid, params, customPath);
  }

  void Calculator::updateSchedulesFromCache(Parameters& params, std::string customPath)
  {
    params.cacheFetcher->getSchedules(
      trips,
      lines,
      paths,
      tripIndexesByUuid,
      serviceIndexesByUuid,
      lineIndexesByUuid,
      pathIndexesByUuid,
      agencyIndexesByUuid,
      nodeIndexesByUuid,
      modeIndexesByShortname,
      tripConnectionDepartureTimes,
      tripConnectionDemands,
      forwardConnections,
      reverseConnections,
      params,
      customPath
    );


    tripsEnabled.clear();
    tripsEnabled.shrink_to_fit();
    tripsEnabled.resize(trips.size());
    tripsUsable.clear();
    tripsUsable.shrink_to_fit();
    tripsUsable.resize(trips.size());
    tripsEnterConnection.clear();
    tripsEnterConnection.shrink_to_fit();
    tripsEnterConnection.resize(trips.size());
    tripsExitConnection.clear();
    tripsExitConnection.shrink_to_fit();
    tripsExitConnection.resize(trips.size());
    tripsEnterConnectionTransferTravelTime.clear();
    tripsEnterConnectionTransferTravelTime.shrink_to_fit();
    tripsEnterConnectionTransferTravelTime.resize(trips.size());
    tripsExitConnectionTransferTravelTime.clear();
    tripsExitConnectionTransferTravelTime.shrink_to_fit();
    tripsExitConnectionTransferTravelTime.resize(trips.size());

    std::cout << forwardConnections.size() << " connections" << std::endl; 

    //int benchmarkingStart = algorithmCalculationTime.getEpoch();

    int lastConnectionIndex = forwardConnections.size() - 1;

    forwardConnectionsIndexPerDepartureTimeHour = std::vector<int>(32, -1);
    reverseConnectionsIndexPerArrivalTimeHour   = std::vector<int>(32, lastConnectionIndex);
    
    int hour {0};
    int i = 0;
    for (auto & connection : forwardConnections)
    {
      while (std::get<connectionIndexes::TIME_DEP>(*connection) >= hour * 3600 && forwardConnectionsIndexPerDepartureTimeHour[hour] == -1 && hour < 32)
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
      while (std::get<connectionIndexes::TIME_ARR>(*connection) <= hour * 3600 && reverseConnectionsIndexPerArrivalTimeHour[hour] == lastConnectionIndex && hour >= 0)
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

    /*for (auto & node : nodes)
    {
      std::cout << node.toString() << std::endl;
      for (int transferableNodeIdx : node.transferableNodesIdx)
      {
        std::cout << "    " << transferableNodeIdx << " (" << nodes[transferableNodeIdx].get()->uuid << ")" << std::endl;
      }
      std::cout << std::endl;
    }*/

  }

  int Calculator::prepare()
  {
    int ret = 0;
    if (params.debugDisplay)
      std::cerr << "preparing nodes, routes, trips, connections and footpaths..." << std::endl;
    if (params.dataFetcherShortname == "cache")
    {
      std::tie(modes, modeIndexesByShortname) = params.cacheFetcher->getModes();

      //updateStationsFromCache(params);
      //updateStopsFromCache(params);
      updateNodesFromCache(params);

      updateDataSourcesFromCache(params);
      updateHouseholdsFromCache(params);
      updatePersonsFromCache(params);
      updateOdTripsFromCache(params);
      updatePlacesFromCache(params);

      updateAgenciesFromCache(params);
      updateServicesFromCache(params);
      
      updateLinesFromCache(params);
      updatePathsFromCache(params);
      updateScenariosFromCache(params);
      updateNetworksFromCache(params);
      updateSchedulesFromCache(params);      
      
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
    return ret;
  }
  
}
