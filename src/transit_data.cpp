#include "transit_data.hpp"
#include "spdlog/spdlog.h"

#include "data_fetcher.hpp"
#include "calculation_time.hpp"
#include "mode.hpp"
#include "data_source.hpp"
#include "person.hpp"
#include "od_trip.hpp"
#include "agency.hpp"
#include "service.hpp"
#include "line.hpp"
#include "path.hpp"
#include "scenario.hpp"
#include "trip.hpp"
#include "connection.hpp"
#include "connection_cache.hpp"
#include "connection_set.hpp"


namespace TrRouting {

  TransitData::TransitData(DataFetcher& fetcher, bool cacheAllScenarios) :
    dataFetcher(fetcher)
  {
    DataStatus loadStatus = loadAllData();
    if (loadStatus != DataStatus::READY) {
      //TODO For now, don't throw on error because Transit server expect to get the dataStatus
      //throw std::exception("Incomplete transit data");
      spdlog::error("TransitData loading had error, object will not be valid");
    }
    if (cacheAllScenarios) {
      scenarioConnectionCache = new ScenarioConnectionCacheAll();
      spdlog::info("Will cache all connectionSets");
    } else {
      scenarioConnectionCache = new ScenarioConnectionCacheOne();
      spdlog::info("Will cache one connectionSet");
    }
  }

  TransitData::~TransitData() {
    delete scenarioConnectionCache;
  }

  DataStatus TransitData::getDataStatus() const {
    if (agencies.size() == 0)
    {
      return DataStatus::NO_AGENCIES;
    }
    else if (services.size() == 0)
    {
      return DataStatus::NO_SERVICES;
    }
    else if (nodes.size() == 0)
    {
      return DataStatus::NO_NODES;
    }
    else if (lines.size() == 0)
    {
      return DataStatus::NO_LINES;
    }
    else if (paths.size() == 0)
    {
      return DataStatus::NO_PATHS;
    }
    else if (scenarios.size() == 0)
    {
      return DataStatus::NO_SCENARIOS;
    }
    //TODO Add connection
    else if (/*calculator->countConnections() == 0  ||*/ trips.size() == 0)
    {
      return DataStatus::NO_SCHEDULES;
    }
    return DataStatus::READY;
  }

  int TransitData::updateNodes(std::string customPath)
  {
    return dataFetcher.getNodes(nodes, customPath);
  }

  int TransitData::updateDataSources(std::string customPath)
  {
    return dataFetcher.getDataSources(dataSources, customPath);
  }
  /* TODO #167
  int TransitData::updateHouseholds(std::string customPath)
  {
    return dataFetcher.getHouseholds(households, householdIndexesByUuid, dataSourceIndexesByUuid, nodeIndexesByUuid, customPath);
  }
  */
  int TransitData::updatePersons(std::string customPath)
  {
    return dataFetcher.getPersons(persons, getDataSources(), customPath);
  }
  
  int TransitData::updateOdTrips(std::string customPath)
  {
    return dataFetcher.getOdTrips(odTrips, dataSources, getPersons(), getNodes(), customPath);
  }
  /* TODO #167
  int TransitData::updatePlaces(std::string customPath)
  {
    return dataFetcher.getPlaces(places, placeIndexesByUuid, dataSourceIndexesByUuid, nodeIndexesByUuid, customPath);
  }
  */ 
  int TransitData::updateAgencies(std::string customPath)
  {
    return dataFetcher.getAgencies(agencies, customPath);
  }

  int TransitData::updateServices(std::string customPath)
  {
    return dataFetcher.getServices(services, customPath);
  }

  int TransitData::updateLines(std::string customPath)
  {
    return dataFetcher.getLines(lines, getAgencies(), getModes(), customPath);
  }

  int TransitData::updatePaths(std::string customPath)
  {
    return dataFetcher.getPaths(paths, getLines(), getNodes(), customPath);
  }

  int TransitData::updateScenarios(std::string customPath)
  {
    return dataFetcher.getScenarios(scenarios, getServices(), getLines(), getAgencies(), getNodes(), getModes(), customPath);
  }

  int TransitData::updateSchedules(std::string customPath)
  {
    int ret =  dataFetcher.getSchedules(
      trips,
      getLines(),
      paths,
      getServices(),
      connections,
      customPath
    );
    if (ret < 0)
    {
      return ret;
    }
    return generateForwardAndReverseConnections();
  }

  const int CONNECTION_ITERATOR_CACHE_BEGIN_HOUR = 0;
  const int CONNECTION_ITERATOR_CACHE_END_HOUR = 32;

  int TransitData::generateForwardAndReverseConnections()
  {

    // Copy the connections to both forward and reverse vectors
    forwardConnections.clear();
    reverseConnections.clear();
    for (size_t i=0; i<connections.size(); i++)
    {
      forwardConnections.push_back(connections[i]);
      reverseConnections.push_back(connections[i]);
    }
    forwardConnections.shrink_to_fit();
    reverseConnections.shrink_to_fit();

    try
    {
      spdlog::info("Sorting connections...");
      // Sort forward connections by departure time, trip id, sequence
      //TODO Maybe this could be handled by an operator in the connection class or at least some function that we could bind
      std::stable_sort(forwardConnections.begin(), forwardConnections.end(), [](const std::reference_wrapper<const Connection>& connectionA, const std::reference_wrapper<const Connection>& connectionB)
      {
        if (connectionA.get().getDepartureTime() < connectionB.get().getDepartureTime())
        {
          return true;
        }
        else if (connectionA.get().getDepartureTime() > connectionB.get().getDepartureTime())
        {
          return false;
        }
        //TODO We could do something  better than comparing uuud for trip. We just need something to have a stable sort
        if (connectionA.get().getTrip().uuid < connectionB.get().getTrip().uuid)
        {
          return true;
        }
        else if (connectionA.get().getTrip().uuid > connectionB.get().getTrip().uuid)
        {
          return false;
        }
        if (connectionA.get().getSequenceInTrip() < connectionB.get().getSequenceInTrip())
        {
          return true;
        }
        else if (connectionA.get().getSequenceInTrip() > connectionB.get().getSequenceInTrip())
        {
          return false;
        }
        return false;
      });
      // Sort reverse connection by arrival time, trip and sequence
      std::stable_sort(reverseConnections.begin(), reverseConnections.end(), [](const std::reference_wrapper<const Connection>& connectionA, const std::reference_wrapper<const Connection>& connectionB)
      {
        if (connectionA.get().getArrivalTime() > connectionB.get().getArrivalTime())
        {
          return true;
        }
        else if (connectionA.get().getArrivalTime() < connectionB.get().getArrivalTime())
        {
          return false;
        }
        if (connectionA.get().getTrip().uuid > connectionB.get().getTrip().uuid) // here we need to reverse sequence!
        {
          return true;
        }
        else if (connectionA.get().getTrip().uuid < connectionB.get().getTrip().uuid)
        {
          return false;
        }
        if (connectionA.get().getSequenceInTrip() > connectionB.get().getSequenceInTrip()) // here we need to reverse sequence!
        {
          return true;
        }
        else if (connectionA.get().getSequenceInTrip() < connectionB.get().getSequenceInTrip())
        {
          return false;
        }
        return false;
      });

      CalculationTime algorithmCalculationTime = CalculationTime();
      algorithmCalculationTime.start();
      long long       calculationTime;
      calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

      // assign connections to trips:
      for(auto & connection : forwardConnections)
      {
        //TODO This require a trip to be modified, we might want to revisit how to create Connection and Trip (#201)
        Trip & trip = connection.get().getTripMutable();
        trip.forwardConnections.push_back(connection);
      }

      for(auto & connection : reverseConnections)
      {
        //TODO This require a trip to be modified, we might want to revisit how to create Connection and Trip (#201)
        Trip & trip = connection.get().getTripMutable();
        trip.reverseConnections.push_back(connection);
      }

      spdlog::debug("-- assign connections to trips -- {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);

      return 0;
    }
    catch (const std::exception& ex)
    {
      spdlog::error("-- Error assigning connections to trips -- {} ", ex.what());
      return -EINVAL;
    }
  }
  
  DataStatus TransitData::loadAllData() {
    int ret = 0;

    spdlog::debug("preparing nodes, routes, trips, connections and footpaths...");
    
    modes = dataFetcher.getModes();

    ret = updateNodes();
    // Ignore missing nodes file
    if (ret < 0 && ret != -ENOENT)
    {
      return DataStatus::DATA_READ_ERROR;
    }
    ret = updateDataSources();
    // Ignore missing data sources file
    if (ret < 0 && ret != -ENOENT)
    {
      return DataStatus::DATA_READ_ERROR;
    }
    /* TODO #167
    ret = updateHouseholds();
    if (ret < 0)
    {
      return DataStatus::DATA_READ_ERROR;
    }
    */
    ret = updatePersons();
    if (ret < 0)
    {
      return DataStatus::DATA_READ_ERROR;
    }
    ret = updateOdTrips();
    if (ret < 0)
    {
      return DataStatus::DATA_READ_ERROR;
    }
    /* TODO #167
    ret = updatePlaces();
    if (ret < 0)
    {
      return DataStatus::DATA_READ_ERROR;
    }
    */
    ret = updateAgencies();
    // Ignore missing file
    if (ret < 0 && ret != -ENOENT)
    {
      return DataStatus::DATA_READ_ERROR;
    }
    ret = updateServices();
    // Ignore missing file
    if (ret < 0 && ret != -ENOENT)
    {
      return DataStatus::DATA_READ_ERROR;
    }
    ret = updateLines();
    // Ignore missing file
    if (ret < 0 && ret != -ENOENT)
    {
      return DataStatus::DATA_READ_ERROR;
    }
    ret = updatePaths();
    // Ignore missing file
    if (ret < 0 && ret != -ENOENT)
    {
      return DataStatus::DATA_READ_ERROR;
    }
    ret = updateScenarios();
    // Ignore missing file
    if (ret < 0 && ret != -ENOENT)
    {
      return DataStatus::DATA_READ_ERROR;
    }
    ret = updateSchedules();
    // Ignore missing file
    if (ret < 0 && ret != -ENOENT)
    {
      return DataStatus::DATA_READ_ERROR;
    }

    return getDataStatus();
  }
  
  std::shared_ptr<ConnectionSet> TransitData::getConnectionsForScenario(const Scenario & scenario) const {
    std::optional<std::shared_ptr<ConnectionSet>> optCurrentCache = scenarioConnectionCache->get(scenario.uuid);
    if (optCurrentCache.has_value()) {
      return optCurrentCache.value();
    }

    spdlog::debug("Computing connection cache for scenario {}...", boost::uuids::to_string(scenario.uuid));
    // Create the cache for scenario
    // Get the list of enabled trips
    std::unordered_map<Trip::uid_t, bool> tripsEnabled;
    std::vector<std::reference_wrapper<const Trip>> trips; 
    for (auto & tripIte : getTrips())
    {
      const Trip & trip = tripIte.second;
      bool enabled = true;

      if (enabled && scenario.servicesList.size() > 0)
      {
        if (std::find(scenario.servicesList.begin(), scenario.servicesList.end(), trip.service) == scenario.servicesList.end())
        {
          enabled = false;
        }
      }

      if (enabled && scenario.onlyLines.size() > 0)
      {
        if (std::find(scenario.onlyLines.begin(), scenario.onlyLines.end(), trip.line) == scenario.onlyLines.end())
        {
          enabled = false;
        }
      }

      if (enabled && scenario.onlyModes.size() > 0)
      {
        if (std::find(scenario.onlyModes.begin(), scenario.onlyModes.end(), trip.mode) == scenario.onlyModes.end())
        {
          enabled = false;
        }
      }

      if (enabled  && scenario.onlyNodes.size() > 0)
      {
        // FIXME: This is not right, it should look for a node, not the mode
        // FIXME2: Commented out, since mode is now typed, it won't match
        /*if (std::find(scenario.onlyNodesIdx()->begin(), scenario.onlyNodesIdx()->end(), trip->modeIdx) == scenario.onlyNodesIdx()->end())
          {
          enabled = -1;
          }(*/
      }

      if (enabled && scenario.onlyAgencies.size() > 0)
      {
        if (std::find(scenario.onlyAgencies.begin(), scenario.onlyAgencies.end(), trip.agency) == scenario.onlyAgencies.end())
        {
          enabled = false;
        }
      }

      if (enabled && scenario.exceptLines.size() > 0)
      {
        if (std::find(scenario.exceptLines.begin(), scenario.exceptLines.end(), trip.line) != scenario.exceptLines.end())
        {
          enabled = false;
        }
      }

      if (enabled && scenario.exceptNodes.size() > 0)
      {
        // FIXME: This is not right, it should look for a node, not the mode
        // FIXME2: Commented out, since mode is now typed, it won't match
        /*
          if (std::find(scenario.exceptNodesIdx()->begin(), scenario.exceptNodesIdx()->end(), trip.modeIdx) != scenario.exceptNodesIdx()->end())
          {
          enabled = false;
          }*/
      }

      if (enabled && scenario.exceptModes.size() > 0)
      {
        if (std::find(scenario.exceptModes.begin(), scenario.exceptModes.end(), trip.mode) != scenario.exceptModes.end())
        {
          enabled = false;
        }
      }

      if (enabled && scenario.exceptAgencies.size() > 0)
      {
        if (std::find(scenario.exceptAgencies.begin(), scenario.exceptAgencies.end(), trip.agency) != scenario.exceptAgencies.end())
        {
          enabled = false;
        }
      }
      tripsEnabled[trip.uid] = enabled;
      if (enabled) {
        trips.push_back(trip);
      }
    }

    // Keep only the connections that are active for enabled trips
    std::vector<std::reference_wrapper<const Connection>> scenarioForwardConnections;
    std::vector<std::reference_wrapper<const Connection>> scenarioReverseConnections; 
    auto forwardLastConnection = forwardConnections.end(); // cache last connection for loop
    for(auto connection = forwardConnections.begin(); connection != forwardLastConnection; ++connection)
    {
      const Trip & trip = (*connection).get().getTrip();

      // enabled trips only here:
      if (tripsEnabled[trip.uid]) {
        scenarioForwardConnections.push_back(*connection);
      }
    }

    auto reverseLastConnection = reverseConnections.end(); // cache last connection for loop
    for(auto connection = reverseConnections.begin(); connection != reverseLastConnection; ++connection)
    {
      const Trip & trip = (*connection).get().getTrip();

      // enabled trips only here:
      if (tripsEnabled[trip.uid]) {
        scenarioReverseConnections.push_back(*connection);
      }
    }

    std::shared_ptr<ConnectionSet> currentCache = std::make_shared<ConnectionSet>(trips, scenarioForwardConnections, scenarioReverseConnections);
    scenarioConnectionCache->set(scenario.uuid, currentCache);
    return currentCache;
    
  }
}
