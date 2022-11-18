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
    
    loadAllData();
    if(loadAllData() != DataStatus::READY ) {
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

  //TODO This should probably take a reference to the connections object and later not use the std::move semantic
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
