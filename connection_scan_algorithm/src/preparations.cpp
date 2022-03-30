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

  int Calculator::updateSchedulesFromCache(Parameters& params, std::string customPath)
  {
    std::vector<std::shared_ptr<ConnectionTuple>> connections;
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

  Calculator::DataStatus validateData(Calculator* calculator) {
    if (calculator->countAgencies() == 0)
    {
      return Calculator::DataStatus::NO_AGENCIES;
    }
    else if (calculator->countServices() == 0)
    {
      return Calculator::DataStatus::NO_SERVICES;
    }
    else if (calculator->countNodes() == 0)
    {
      return Calculator::DataStatus::NO_NODES;
    }
    else if (calculator->countLines() == 0)
    {
      return Calculator::DataStatus::NO_LINES;
    }
    else if (calculator->countPaths() == 0)
    {
      return Calculator::DataStatus::NO_PATHS;
    }
    else if (calculator->countScenarios() == 0)
    {
      return Calculator::DataStatus::NO_SCENARIOS;
    }
    else if (calculator->countConnections() == 0  || calculator->countTrips() == 0)
    {
      return Calculator::DataStatus::NO_SCHEDULES;
    }
    return Calculator::DataStatus::READY;
  }

  Calculator::DataStatus Calculator::prepare()
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
        return Calculator::DataStatus::DATA_READ_ERROR;
      }

      ret = updateDataSourcesFromCache(params);
      // Ignore missing data sources file
      if (ret < 0 && ret != -ENOENT)
      {
        return Calculator::DataStatus::DATA_READ_ERROR;
      }
      ret = updateHouseholdsFromCache(params);
      if (ret < 0)
      {
        return Calculator::DataStatus::DATA_READ_ERROR;
      }
      ret = updatePersonsFromCache(params);
      if (ret < 0)
      {
        return Calculator::DataStatus::DATA_READ_ERROR;
      }
      ret = updateOdTripsFromCache(params);
      if (ret < 0)
      {
        return Calculator::DataStatus::DATA_READ_ERROR;
      }
      ret = updatePlacesFromCache(params);
      if (ret < 0)
      {
        return Calculator::DataStatus::DATA_READ_ERROR;
      }

      ret = updateAgenciesFromCache(params);
      // Ignore missing file
      if (ret < 0 && ret != -ENOENT)
      {
        return Calculator::DataStatus::DATA_READ_ERROR;
      }
      ret = updateServicesFromCache(params);
      // Ignore missing file
      if (ret < 0 && ret != -ENOENT)
      {
        return Calculator::DataStatus::DATA_READ_ERROR;
      }
      
      ret = updateLinesFromCache(params);
      // Ignore missing file
      if (ret < 0 && ret != -ENOENT)
      {
        return Calculator::DataStatus::DATA_READ_ERROR;
      }
      ret = updatePathsFromCache(params);
      // Ignore missing file
      if (ret < 0 && ret != -ENOENT)
      {
        return Calculator::DataStatus::DATA_READ_ERROR;
      }
      ret = updateScenariosFromCache(params);
      // Ignore missing file
      if (ret < 0 && ret != -ENOENT)
      {
        return Calculator::DataStatus::DATA_READ_ERROR;
      }
      ret = updateSchedulesFromCache(params);
      // Ignore missing file
      if (ret < 0 && ret != -ENOENT)
      {
        return Calculator::DataStatus::DATA_READ_ERROR;
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
    return validateData(this);
  }
  
}
