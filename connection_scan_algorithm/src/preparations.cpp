#include "spdlog/spdlog.h"
#include "calculator.hpp"
#include "parameters.hpp"
#include "data_fetcher.hpp"
#include "mode.hpp"

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
  // TODO now that we have a "generic" data fetcher interface, we should remove the FromCache from these function name
  int Calculator::updateNodesFromCache(std::string customPath)
  {
    return dataFetcher.getNodes(nodes, nodeIndexesByUuid, stationIndexesByUuid, customPath);
  }

  int Calculator::updateDataSourcesFromCache(std::string customPath)
  {
    return dataFetcher.getDataSources(dataSources, dataSourceIndexesByUuid, customPath);
  }

  int Calculator::updateHouseholdsFromCache(std::string customPath)
  {
    return dataFetcher.getHouseholds(households, householdIndexesByUuid, dataSourceIndexesByUuid, nodeIndexesByUuid, customPath);
  }

  int Calculator::updatePersonsFromCache(std::string customPath)
  {
    return dataFetcher.getPersons(persons, personIndexesByUuid, dataSourceIndexesByUuid, householdIndexesByUuid, nodeIndexesByUuid, customPath);
  }
  
  int Calculator::updateOdTripsFromCache(std::string customPath)
  {
    return dataFetcher.getOdTrips(odTrips, odTripIndexesByUuid, dataSourceIndexesByUuid, householdIndexesByUuid, personIndexesByUuid, nodeIndexesByUuid, customPath);
  }

  int Calculator::updatePlacesFromCache(std::string customPath)
  {
    return dataFetcher.getPlaces(places, placeIndexesByUuid, dataSourceIndexesByUuid, nodeIndexesByUuid, customPath);
  }

  int Calculator::updateAgenciesFromCache(std::string customPath)
  {
    return dataFetcher.getAgencies(agencies, agencyIndexesByUuid, customPath);
  }

  int Calculator::updateServicesFromCache(std::string customPath)
  {
    return dataFetcher.getServices(services, serviceIndexesByUuid, customPath);
  }

  int Calculator::updateLinesFromCache(std::string customPath)
  {
    return dataFetcher.getLines(lines, lineIndexesByUuid, agencyIndexesByUuid, modeIndexesByShortname, customPath);
  }

  int Calculator::updatePathsFromCache(std::string customPath)
  {
    return dataFetcher.getPaths(paths, pathIndexesByUuid, lineIndexesByUuid, nodeIndexesByUuid, customPath);
  }

  int Calculator::updateScenariosFromCache(std::string customPath)
  {
    return dataFetcher.getScenarios(scenarios, scenarioIndexesByUuid, serviceIndexesByUuid, lineIndexesByUuid, agencyIndexesByUuid, nodeIndexesByUuid, modeIndexesByShortname, customPath);
  }

  int Calculator::updateSchedulesFromCache(std::string customPath)
  {
    std::vector<std::shared_ptr<ConnectionTuple>> connections;
    int ret =  dataFetcher.getSchedules(
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

    spdlog::debug("preparing nodes, routes, trips, connections and footpaths...");
    
    std::tie(modes, modeIndexesByShortname) = dataFetcher.getModes();

    //updateStationsFromCache();
    //updateStopsFromCache();
    ret = updateNodesFromCache();
    // Ignore missing nodes file
    if (ret < 0 && ret != -ENOENT)
    {
      return Calculator::DataStatus::DATA_READ_ERROR;
    }
    ret = updateDataSourcesFromCache();
    // Ignore missing data sources file
    if (ret < 0 && ret != -ENOENT)
    {
      return Calculator::DataStatus::DATA_READ_ERROR;
    }
    ret = updateHouseholdsFromCache();
    if (ret < 0)
    {
      return Calculator::DataStatus::DATA_READ_ERROR;
    }
    ret = updatePersonsFromCache();
    if (ret < 0)
    {
      return Calculator::DataStatus::DATA_READ_ERROR;
    }
    ret = updateOdTripsFromCache();
    if (ret < 0)
    {
      return Calculator::DataStatus::DATA_READ_ERROR;
    }
    ret = updatePlacesFromCache();
    if (ret < 0)
    {
      return Calculator::DataStatus::DATA_READ_ERROR;
    }
    
    ret = updateAgenciesFromCache();
    // Ignore missing file
    if (ret < 0 && ret != -ENOENT)
    {
      return Calculator::DataStatus::DATA_READ_ERROR;
    }
    ret = updateServicesFromCache();
    // Ignore missing file
    if (ret < 0 && ret != -ENOENT)
    {
      return Calculator::DataStatus::DATA_READ_ERROR;
    }
    ret = updateLinesFromCache();
    // Ignore missing file
    if (ret < 0 && ret != -ENOENT)
    {
      return Calculator::DataStatus::DATA_READ_ERROR;
    }
    ret = updatePathsFromCache();
    // Ignore missing file
    if (ret < 0 && ret != -ENOENT)
    {
      return Calculator::DataStatus::DATA_READ_ERROR;
    }
    ret = updateScenariosFromCache();
    // Ignore missing file
    if (ret < 0 && ret != -ENOENT)
    {
      return Calculator::DataStatus::DATA_READ_ERROR;
    }
    ret = updateSchedulesFromCache();
    // Ignore missing file
    if (ret < 0 && ret != -ENOENT)
    {
      return Calculator::DataStatus::DATA_READ_ERROR;
    }

    initializeCalculationData();
    spdlog::info("preparing nodes tentative times, trips enter connections and journeys...");
    return validateData(this);
  }
  
}
