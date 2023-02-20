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


namespace TrRouting {

  TransitData::TransitData(DataFetcher& fetcher) :
    dataFetcher(fetcher)
  {
    DataStatus loadStatus = loadAllData();
    if (loadStatus != DataStatus::READY) {
      //TODO For now, don't throw on error because Transit server expect to get the dataStatus
      //throw std::exception("Incomplete transit data");
      spdlog::error("TransitData loading had error, object will not be valid");
    }
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
    std::vector<std::shared_ptr<Connection>> connections;
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
    return generateForwardAndReverseConnections(connections);
  }

  const int CONNECTION_ITERATOR_CACHE_BEGIN_HOUR = 0;
  const int CONNECTION_ITERATOR_CACHE_END_HOUR = 32;

  std::vector<std::shared_ptr<Connection>>::const_iterator TransitData::getForwardConnectionsBeginAtDepartureHour(int hour) const
  {
    if (hour > CONNECTION_ITERATOR_CACHE_END_HOUR || hour  < CONNECTION_ITERATOR_CACHE_BEGIN_HOUR) {
      return forwardConnections.cend();
    }

    return forwardConnectionsBeginIteratorCache[hour];
  };

  std::vector<std::shared_ptr<Connection>>::const_iterator TransitData::getReverseConnectionsBeginAtArrivalHour(int hour) const
  {
    if (hour > CONNECTION_ITERATOR_CACHE_END_HOUR || hour  < CONNECTION_ITERATOR_CACHE_BEGIN_HOUR) {
      return reverseConnections.cend();
    }

    return reverseConnectionsBeginIteratorCache[hour];
  };

  // Create a cache with a begin iterator which match the connection closest to the specified hour
  void TransitData::generateConnectionsIteratorCache() {
    int currentHour = CONNECTION_ITERATOR_CACHE_BEGIN_HOUR;

    // Create first part of the forward cache
    // For each hour, we save the iterator matching the connection which as a departure time bigger
    // than this hour
    for (std::vector<std::shared_ptr<Connection>>::const_iterator ite = forwardConnections.cbegin();
         ite != forwardConnections.cend();
         ite++) {
      // We can have a gap of multiple hours between 2 connections, so the current iterator
      // can be valid for multiple hour slots
      while ((*ite)->getDepartureTime() >= currentHour * 3600) {
        forwardConnectionsBeginIteratorCache.push_back(ite);
        currentHour++;
      }
    }

    //Finish filling forward cache
    //We reached the end of the forwardConnections in the previously, so here we
    //fill the rest of the forward cache with end iterator
    for (;currentHour < CONNECTION_ITERATOR_CACHE_END_HOUR; currentHour++) {
      forwardConnectionsBeginIteratorCache.push_back(forwardConnections.cend());
    }

    // Create the reverse cache
    currentHour = CONNECTION_ITERATOR_CACHE_END_HOUR - 1;
    for (std::vector<std::shared_ptr<Connection>>::const_iterator ite = reverseConnections.cbegin();
         ite != reverseConnections.cend();
         ite++) {
      // Fill cache with same iterator if we have multi hour gap between connection
      while ((*ite)->getArrivalTime() <= currentHour * 3600 && currentHour > CONNECTION_ITERATOR_CACHE_BEGIN_HOUR) {
        reverseConnectionsBeginIteratorCache.insert(reverseConnectionsBeginIteratorCache.begin(),ite);
        currentHour--;
      }
    }
  }

  int TransitData::generateForwardAndReverseConnections(const std::vector<std::shared_ptr<Connection>> &connections)
  {

    // Copy the connections to both forward and reverse vectors
    forwardConnections.clear();
    reverseConnections.clear();
    for (size_t i=0; i<connections.size(); i++)
    {
      std::shared_ptr<Connection> reverseConnection = connections[i];
      forwardConnections.push_back(std::move(connections[i]));
      reverseConnections.push_back(std::move(reverseConnection));
    }
    forwardConnections.shrink_to_fit();
    reverseConnections.shrink_to_fit();

    try
    {
      spdlog::info("Sorting connections...");
      // Sort forward connections by departure time, trip id, sequence
      //TODO Maybe this could be handled by an operator in the connection class or at least some function that we could bind
      std::stable_sort(forwardConnections.begin(), forwardConnections.end(), [](const std::shared_ptr<Connection>& connectionA, const std::shared_ptr<Connection>& connectionB)
      {
        if (connectionA->getDepartureTime() < connectionB->getDepartureTime())
        {
          return true;
        }
        else if (connectionA->getDepartureTime() > connectionB->getDepartureTime())
        {
          return false;
        }
        //TODO We could do something  better than comparing uuud for trip. We just need something to have a stable sort
        if (connectionA->getTrip().uuid < connectionB->getTrip().uuid)
        {
          return true;
        }
        else if (connectionA->getTrip().uuid > connectionB->getTrip().uuid)
        {
          return false;
        }
        if (connectionA->getSequenceInTrip() < connectionB->getSequenceInTrip())
        {
          return true;
        }
        else if (connectionA->getSequenceInTrip() > connectionB->getSequenceInTrip())
        {
          return false;
        }
        return false;
      });
      // Sort reverse connection by arrival time, trip and sequence
      std::stable_sort(reverseConnections.begin(), reverseConnections.end(), [](const std::shared_ptr<Connection>& connectionA, const std::shared_ptr<Connection>& connectionB)
      {
        if (connectionA->getArrivalTime() > connectionB->getArrivalTime())
        {
          return true;
        }
        else if (connectionA->getArrivalTime() < connectionB->getArrivalTime())
        {
          return false;
        }
        if (connectionA->getTrip().uuid > connectionB->getTrip().uuid) // here we need to reverse sequence!
        {
          return true;
        }
        else if (connectionA->getTrip().uuid < connectionB->getTrip().uuid)
        {
          return false;
        }
        if (connectionA->getSequenceInTrip() > connectionB->getSequenceInTrip()) // here we need to reverse sequence!
        {
          return true;
        }
        else if (connectionA->getSequenceInTrip() < connectionB->getSequenceInTrip())
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
        Trip & trip = connection->getTripMutable();
        trip.forwardConnections.push_back(connection);
      }

      for(auto & connection : reverseConnections)
      {
        //TODO This require a trip to be modified, we might want to revisit how to create Connection and Trip (#201)
        Trip & trip = connection->getTripMutable();
        trip.reverseConnections.push_back(connection);
      }

      spdlog::debug("-- assign connections to trips -- {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);

      generateConnectionsIteratorCache();
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
  
}
