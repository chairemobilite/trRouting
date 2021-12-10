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

  int Calculator::updateNodesFromCache(Parameters& params, std::string customPath)
  {
    return params.cacheFetcher->getNodes(nodes, nodeIndexesByUuid, stationIndexesByUuid, params, customPath);
  }

  int Calculator::updateDataSourcesFromCache(Parameters& params, std::string customPath)
  {
    return params.cacheFetcher->getDataSources(dataSources, dataSourceIndexesByUuid, params, customPath);
  }

  int Calculator::updateHouseholdsFromCache(Parameters& params, std::string customPath)
  {
    return params.cacheFetcher->getHouseholds(households, householdIndexesByUuid, dataSourceIndexesByUuid, nodeIndexesByUuid, params, customPath);
  }

  int Calculator::updatePersonsFromCache(Parameters& params, std::string customPath)
  {
    return params.cacheFetcher->getPersons(persons, personIndexesByUuid, dataSourceIndexesByUuid, householdIndexesByUuid, nodeIndexesByUuid, params, customPath);
  }
  
  int Calculator::updateOdTripsFromCache(Parameters& params, std::string customPath)
  {
    return params.cacheFetcher->getOdTrips(odTrips, odTripIndexesByUuid, dataSourceIndexesByUuid, householdIndexesByUuid, personIndexesByUuid, nodeIndexesByUuid, params, customPath);
  }

  int Calculator::updatePlacesFromCache(Parameters& params, std::string customPath)
  {
    return params.cacheFetcher->getPlaces(places, placeIndexesByUuid, dataSourceIndexesByUuid, nodeIndexesByUuid, params, customPath);
  }

  int Calculator::updateAgenciesFromCache(Parameters& params, std::string customPath)
  {
    return params.cacheFetcher->getAgencies(agencies, agencyIndexesByUuid, params, customPath);
  }

  int Calculator::updateServicesFromCache(Parameters& params, std::string customPath)
  {
    return params.cacheFetcher->getServices(services, serviceIndexesByUuid, params, customPath);
  }

  int Calculator::updateLinesFromCache(Parameters& params, std::string customPath)
  {
    return params.cacheFetcher->getLines(lines, lineIndexesByUuid, agencyIndexesByUuid, modeIndexesByShortname, params, customPath);
  }

  int Calculator::updatePathsFromCache(Parameters& params, std::string customPath)
  {
    return params.cacheFetcher->getPaths(paths, pathIndexesByUuid, lineIndexesByUuid, nodeIndexesByUuid, params, customPath);
  }

  int Calculator::updateScenariosFromCache(Parameters& params, std::string customPath)
  {
    return params.cacheFetcher->getScenarios(scenarios, scenarioIndexesByUuid, serviceIndexesByUuid, lineIndexesByUuid, agencyIndexesByUuid, nodeIndexesByUuid, modeIndexesByShortname, params, customPath);
  }

  int Calculator::updateNetworksFromCache(Parameters& params, std::string customPath)
  {
    return params.cacheFetcher->getNetworks(networks, networkIndexesByUuid, agencyIndexesByUuid, serviceIndexesByUuid, scenarioIndexesByUuid, params, customPath);
  }

  int Calculator::updateSchedulesFromCache(Parameters& params, std::string customPath)
  {
    std::vector<std::shared_ptr<std::tuple<int,int,int,int,int,short,short,int,int,int,short,short>>> connections;
    int ret =  params.cacheFetcher->getSchedules(
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
      connections,
      params,
      customPath
    );
    if (ret < 0)
    {
      return ret;
    }
    return setConnections(connections);
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
      ret = updateNodesFromCache(params);
      // Ignore missing nodes file
      if (ret < 0 && ret != -ENOENT)
      {
        return -1;
      }

      ret = updateDataSourcesFromCache(params);
      // Ignore missing data sources file
      if (ret < 0 && ret != -ENOENT)
      {
        return -1;
      }
      ret = updateHouseholdsFromCache(params);
      if (ret < 0)
      {
        return -1;
      }
      ret = updatePersonsFromCache(params);
      if (ret < 0)
      {
        return -1;
      }
      ret = updateOdTripsFromCache(params);
      if (ret < 0)
      {
        return -1;
      }
      ret = updatePlacesFromCache(params);
      if (ret < 0)
      {
        return -1;
      }

      ret = updateAgenciesFromCache(params);
      // Ignore missing file
      if (ret < 0 && ret != -ENOENT)
      {
        return -1;
      }
      ret = updateServicesFromCache(params);
      // Ignore missing file
      if (ret < 0 && ret != -ENOENT)
      {
        return -1;
      }
      
      ret = updateLinesFromCache(params);
      // Ignore missing file
      if (ret < 0 && ret != -ENOENT)
      {
        return -1;
      }
      ret = updatePathsFromCache(params);
      // Ignore missing file
      if (ret < 0 && ret != -ENOENT)
      {
        return -1;
      }
      ret = updateScenariosFromCache(params);
      // Ignore missing file
      if (ret < 0 && ret != -ENOENT)
      {
        return -1;
      }
      ret = updateNetworksFromCache(params);
      // Ignore missing file
      if (ret < 0 && ret != -ENOENT)
      {
        return -1;
      }
      ret = updateSchedulesFromCache(params);
      // Ignore missing file
      if (ret < 0 && ret != -ENOENT)
      {
        return -1;
      }
      
    }
    else if (params.dataFetcherShortname == "gtfs")
    {
      // not yet implemented
    }
    else if (params.dataFetcherShortname == "csv")
    {
      // not yet implemented
    }
    
    initializeCalculationData();
    std::cout << "preparing nodes tentative times, trips enter connections and journeys..." << std::endl;
    return ret;
  }
  
}
