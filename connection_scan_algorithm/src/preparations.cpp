#include "spdlog/spdlog.h"
#include "calculator.hpp"
#include "parameters.hpp"
#include "data_fetcher.hpp"
#include "mode.hpp"

namespace TrRouting
{

  // TODO now that we have a "generic" data fetcher interface, we should remove the FromCache from these function name
  int Calculator::updateNodesFromCache(std::string customPath)
  {
    return dataFetcher.getNodes(nodes, customPath);
  }

  int Calculator::updateDataSourcesFromCache(std::string customPath)
  {
    return dataFetcher.getDataSources(dataSources, customPath);
  }
  /* TODO #167
  int Calculator::updateHouseholdsFromCache(std::string customPath)
  {
    return dataFetcher.getHouseholds(households, householdIndexesByUuid, dataSourceIndexesByUuid, nodeIndexesByUuid, customPath);
  }
  */
  int Calculator::updatePersonsFromCache(std::string customPath)
  {
    return dataFetcher.getPersons(persons, personIndexesByUuid, getDataSources(), customPath);
  }
  
  int Calculator::updateOdTripsFromCache(std::string customPath)
  {
    return dataFetcher.getOdTrips(odTrips, odTripIndexesByUuid, dataSources, personIndexesByUuid, getNodes(), customPath);
  }
  /* TODO #167
  int Calculator::updatePlacesFromCache(std::string customPath)
  {
    return dataFetcher.getPlaces(places, placeIndexesByUuid, dataSourceIndexesByUuid, nodeIndexesByUuid, customPath);
  }
  */ 
  int Calculator::updateAgenciesFromCache(std::string customPath)
  {
    return dataFetcher.getAgencies(agencies, customPath);
  }

  int Calculator::updateServicesFromCache(std::string customPath)
  {
    return dataFetcher.getServices(services, customPath);
  }

  int Calculator::updateLinesFromCache(std::string customPath)
  {
    return dataFetcher.getLines(lines, agencies, getModes(), customPath);
  }

  int Calculator::updatePathsFromCache(std::string customPath)
  {
    return dataFetcher.getPaths(paths, lines, getNodes(), customPath);
  }

  int Calculator::updateScenariosFromCache(std::string customPath)
  {
    return dataFetcher.getScenarios(scenarios, scenarioIndexesByUuid, services, lines, agencies, getNodes(), getModes(), customPath);
  }

  int Calculator::updateSchedulesFromCache(std::string customPath)
  {
    std::vector<std::shared_ptr<ConnectionTuple>> connections;
    int ret =  dataFetcher.getSchedules(
      trips,
      lines,
      paths,
      tripIndexesByUuid,
      services,
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
    
    modes = dataFetcher.getModes();

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
    /* TODO #167
    ret = updateHouseholdsFromCache();
    if (ret < 0)
    {
      return Calculator::DataStatus::DATA_READ_ERROR;
    }
    */
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
    /* TODO #167
    ret = updatePlacesFromCache();
    if (ret < 0)
    {
      return Calculator::DataStatus::DATA_READ_ERROR;
    }
    */
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
