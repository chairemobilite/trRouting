#include "connection_scan_algorithm.hpp"

namespace TrRouting
{
  ConnectionScanAlgorithm::ConnectionScanAlgorithm(Parameters& theParams) : params(theParams)
  {
    algorithmCalculationTime = CalculationTime();
    setup();
  }
  
  ConnectionScanAlgorithm::ConnectionScanAlgorithm()
  {
    
  }
  
  void ConnectionScanAlgorithm::resetAccessEgressModes()
  {
    accessMode = params.accessMode;
    egressMode = params.egressMode;
    maxAccessWalkingTravelTimeFromOriginToFirstStopMinutes     = params.maxAccessWalkingTravelTimeFromOriginToFirstStopMinutes;
    maxAccessWalkingTravelTimeFromLastStopToDestinationMinutes = params.maxAccessWalkingTravelTimeFromLastStopToDestinationMinutes;
  }
  
  // call setup only once when starting the calculator. Use updateParams and refresh before each calculation.
  void ConnectionScanAlgorithm::setup()
  {
    calculationId = 1;
    resetAccessEgressModes();
    maxTimeValue = 9999; // that's almost 7 days. No travel time should take that long.
    //setParamsFromYaml("trRoutingConfig.yml"); // disable yml config for now
    if (params.databasePassword.size() > 0)
    {
      DbFetcher::setDbSetupStr("dbname=" + params.databaseName + " user=" + params.databaseUser + " hostaddr=" + params.databaseHost + " password=" + params.databasePassword + " port=" + params.databasePort + ""); // todo: add config to set this
    }
    else
    {
      DbFetcher::setDbSetupStr("dbname=" + params.databaseName + " user=" + params.databaseUser + " hostaddr=" + params.databaseHost + " port=" + params.databasePort + ""); // todo: add config to set this
    }
    DbFetcher::disconnect();
    pickUpTypes                          = DbFetcher::getPickUpTypes(params.applicationShortname);
    dropOffTypes                         = DbFetcher::getDropOffTypes(params.applicationShortname);
    transferDurationsByStopId            = DbFetcher::getTransferDurationsByStopId(params.applicationShortname, params.dataFetcher, params.maxTransferWalkingTravelTimeMinutes, params.transfersSqlWhereClause);
    pathStopSequencesById                = DbFetcher::getPathStopSequencesById(params.applicationShortname, params.dataFetcher);
    stopsById                            = DbFetcher::getStopsById(params.applicationShortname, params.dataFetcher, maxTimeValue);
    routesById                           = DbFetcher::getRoutesById(params.applicationShortname, params.dataFetcher);
    forwardConnectionsById               = DbFetcher::getConnectionsById(params.applicationShortname, params.dataFetcher, params.connectionsSqlWhereClause, params);
    reverseConnectionsById               = forwardConnectionsById;
    
    for(auto & connection : forwardConnectionsById)
    {
      connectionsByDepartureTime.emplace_back(&(connection.second));
    }
    
    // we need stable sort so we get the same order for connections with the same departure time, no matter which platform is used
    std::stable_sort(connectionsByDepartureTime.begin(), connectionsByDepartureTime.end(), [](Connection const* connectionA, Connection const* connectionB)
    {
      if (connectionA->departureFromOriginTimeMinuteOfDay < connectionB->departureFromOriginTimeMinuteOfDay)
      {
        return true;
      }
      else if (connectionA->departureFromOriginTimeMinuteOfDay > connectionB->departureFromOriginTimeMinuteOfDay)
      {
        return false;
      }
      if (connectionA->sequence < connectionB->sequence)
      {
        return true;
      }
      else if (connectionA->sequence > connectionB->sequence)
      {
        return false;
      }
      return false;
    });
    
    // copy connections to create a new vector sorted by reversed arrival time instead of departure time (for reverse calculations using arrival time as input)
    for(auto & connection : reverseConnectionsById)
    {
      std::swap(connection.second.nextConnectionId, connection.second.previousConnectionId);
      std::swap(connection.second.departureFromOriginTimeMinuteOfDay, connection.second.arrivalAtDestinationTimeMinuteOfDay);
      std::swap(connection.second.stopStartId, connection.second.stopEndId);
      std::swap(connection.second.canBoard, connection.second.canUnboard);
      std::swap(connection.second.pathStopSequenceStartId, connection.second.pathStopSequenceEndId);
      connection.second.departureFromOriginTimeMinuteOfDay  = maxTimeValue - connection.second.departureFromOriginTimeMinuteOfDay;
      connection.second.arrivalAtDestinationTimeMinuteOfDay = maxTimeValue - connection.second.arrivalAtDestinationTimeMinuteOfDay;
      connectionsByArrivalTime.emplace_back(&(connection.second));
    }
    std::stable_sort(connectionsByArrivalTime.begin(), connectionsByArrivalTime.end(), [](Connection const* connectionA, Connection const* connectionB)
    {
      if (connectionA->departureFromOriginTimeMinuteOfDay < connectionB->departureFromOriginTimeMinuteOfDay)
      {
        return true;
      }
      else if (connectionA->departureFromOriginTimeMinuteOfDay > connectionB->departureFromOriginTimeMinuteOfDay)
      {
        return false;
      }
      if (connectionA->sequence > connectionB->sequence) // here we need to reverse sequence!
      {
        return true;
      }
      else if (connectionA->sequence < connectionB->sequence)
      {
        return false;
      }
      return false;
    });
    
    connectionsByStartPathStopSequenceId = getConnectionsByStartPathStopSequenceId(connectionsByDepartureTime);
    connectionsByEndPathStopSequenceId   = getConnectionsByStartPathStopSequenceId(connectionsByArrivalTime); // Really: by start is correct because connections are reversed
    pathStopSequencesByStopId            = getPathStopSequencesByStopId(pathStopSequencesById);
  }
    
  // create a map of connection pointers by start path stop sequence id (key).
  std::map<unsigned long long, std::vector<Connection*> > ConnectionScanAlgorithm::getConnectionsByStartPathStopSequenceId(std::vector<Connection*> theConnectionsByDepartureTime)
  {
    
    std::cout << "Creating map of connections by starting path stop sequence id..." << std::endl;
    
    std::cout << std::fixed;
    std::cout << std::setprecision(2);
    
    std::map<unsigned long long, std::vector<Connection*> > connectionsByStartPathStopSequenceId;
    
    unsigned long long i = 0;
    
    for(auto & connection : theConnectionsByDepartureTime)
    {
      
      // add the path stop sequence id key if it does not exist:
      if (connectionsByStartPathStopSequenceId.find(connection->pathStopSequenceStartId) == connectionsByStartPathStopSequenceId.end())
      {
        std::vector<Connection*> connections;
        connectionsByStartPathStopSequenceId[connection->pathStopSequenceStartId] = connections;
      }
      connectionsByStartPathStopSequenceId[connection->pathStopSequenceStartId].emplace_back(connection);
      i++;
      if (i % 1000 == 0)
      {
        std::cout << ((((double) i) / theConnectionsByDepartureTime.size()) * 100) << "%\r"; // \r is used to stay on the same line
      }
    }
    
    return connectionsByStartPathStopSequenceId;
  }
  
  // create a map of connection pointers by end path stop sequence id (key).
  std::map<unsigned long long, std::vector<Connection*> > ConnectionScanAlgorithm::getConnectionsByEndPathStopSequenceId(std::vector<Connection*> theConnectionsByArrivalTime)
  {
    
    std::cout << "Creating map of connections by ending path stop sequence id..." << std::endl;
        
    std::cout << std::fixed;
    std::cout << std::setprecision(2);
    
    std::map<unsigned long long, std::vector<Connection*> > connectionsByEndPathStopSequenceId;
    
    unsigned long long i = 0;
    
    for(auto & connection : theConnectionsByArrivalTime) {
      
      // add the path stop sequence id key does not exist:
      if (connectionsByEndPathStopSequenceId.find(connection->pathStopSequenceEndId) == connectionsByEndPathStopSequenceId.end())
      {
        std::vector<Connection*> connections;
        connectionsByEndPathStopSequenceId[connection->pathStopSequenceEndId] = connections;
      }
      connectionsByEndPathStopSequenceId[connection->pathStopSequenceEndId].emplace_back(connection);
      i++;
      if (i % 1000 == 0)
      {
        std::cout << ((((double) i) / theConnectionsByArrivalTime.size()) * 100) << "%\r"; // \r is used to stay on the same line
      }
    }
    
    std::cout << std::endl;
    
    return connectionsByEndPathStopSequenceId;
  }
  
  std::map<unsigned long long, std::vector<unsigned long long> > ConnectionScanAlgorithm::getPathStopSequencesByStopId(std::map<unsigned long long,PathStopSequence> thePathStopSequencesById)
  {
    
    std::cout << "Creating map of path stop sequences by stop id..." << std::endl;
    
    std::cout << std::fixed;
    std::cout << std::setprecision(2);
    
    std::map<unsigned long long, std::vector<unsigned long long> > pathStopSequencesByStopId;
    
    unsigned long long i = 0;
    for(auto & pathStopSequence : thePathStopSequencesById) {
      // add the stop id if key does not exist:
      if (pathStopSequencesByStopId.find(pathStopSequence.second.stopId) == pathStopSequencesByStopId.end())
      {
        std::vector<unsigned long long> pathStopSequencesforStop;
        pathStopSequencesByStopId[pathStopSequence.second.stopId] = pathStopSequencesforStop;
      }
      pathStopSequencesByStopId[pathStopSequence.second.stopId].emplace_back(pathStopSequence.first);
      i++;
      if (i % 1000 == 0)
      {
        std::cout << ((((double) i) / thePathStopSequencesById.size()) * 100) << "%\r"; // \r is used to stay on the same line
      }
    }
    
    std::cout << std::endl;
    
    return pathStopSequencesByStopId;
  }
  
  // Call before each calculation
  void ConnectionScanAlgorithm::updateParams(Parameters& theParams)
  {
    params = theParams;
  }
  
  void ConnectionScanAlgorithm::refresh()
  {
    
    int totalNumberOfJourneySteps = 0;
    
    ++calculationId;
    journeyStepId = 1;
    
    if (params.forwardCalculation)
    {
      maxUnboardingTimeMinutes = maxTimeValue;
    }
    else
    {
      maxUnboardingTimeMinutes = maxTimeValue;
    }
    
    // refresh stops:
    for (auto & stop : stopsById)
    {
      if (params.forwardCalculation)
      {
        stop.second.arrivalTimeMinuteOfDay = maxTimeValue;
      }
      else
      {
        stop.second.arrivalTimeMinuteOfDay = maxTimeValue;
      }
      stop.second.canUnboardToDestination            = false;
      stop.second.numBoardings                       = 0;
      stop.second.totalInVehicleTravelTimeMinutes    = 0;
      stop.second.totalNotInVehicleTravelTimeMinutes = 0;
      stop.second.journeySteps.resize(0);
      stop.second.journeySteps.shrink_to_fit();
    }

    bool hasOnlyServices     = !params.onlyServiceIds.empty();
    bool hasOnlyRoutes       = !params.onlyRouteIds.empty();
    bool hasOnlyRouteTypes   = !params.onlyRouteTypeIds.empty();
    bool hasOnlyAgencies     = !params.onlyAgencyIds.empty();
    bool hasExceptServices   = !params.exceptServiceIds.empty();
    bool hasExceptRoutes     = !params.exceptRouteIds.empty();
    bool hasExceptRouteTypes = !params.exceptRouteTypeIds.empty();
    bool hasExceptAgencies   = !params.exceptAgencyIds.empty();
    
    // refresh connections:
    if (params.forwardCalculation)
    {
      for (auto & connection : forwardConnectionsById)
      {
        connection.second.reachable          = 0;
        connection.second.calculationEnabled = connection.second.enabled == 1 ? true : false;
        
        if (hasOnlyServices)
        {
          if (params.onlyServiceIds.find(connection.second.serviceId) == params.onlyServiceIds.end())
          {
            connection.second.calculationEnabled = false;
          }
        }
        
        if (hasOnlyRoutes)
        {
          if (connection.second.calculationEnabled && params.onlyRouteIds.find(connection.second.routeId) == params.onlyRouteIds.end())
          {
            connection.second.calculationEnabled = false;
          }
        }
        
        if (hasOnlyRouteTypes)
        {
          if (connection.second.calculationEnabled && params.onlyRouteTypeIds.find(connection.second.routeTypeId) == params.onlyRouteTypeIds.end())
          {
            connection.second.calculationEnabled = false;
          }
        }
        
        if (hasOnlyAgencies)
        {
          if (connection.second.calculationEnabled && params.onlyAgencyIds.find(connection.second.agencyId) == params.onlyAgencyIds.end())
          {
            connection.second.calculationEnabled = false;
          }
        }
        
        if (hasExceptServices)
        {
          if (connection.second.calculationEnabled && params.exceptServiceIds.find(connection.second.serviceId) != params.exceptServiceIds.end())
          {
            connection.second.calculationEnabled = false;
          }
        }
        
        if (hasExceptRoutes)
        {
          if (connection.second.calculationEnabled && params.exceptRouteIds.find(connection.second.routeId) != params.exceptRouteIds.end())
          {
            connection.second.calculationEnabled = false;
          }
        }
        
        if (hasExceptRouteTypes)
        {
          if (connection.second.calculationEnabled && params.exceptRouteTypeIds.find(connection.second.routeTypeId) != params.exceptRouteTypeIds.end())
          {
            connection.second.calculationEnabled = false;
          }
        }
        
        if (hasExceptAgencies)
        {
          if (connection.second.calculationEnabled && params.exceptAgencyIds.find(connection.second.agencyId) != params.exceptAgencyIds.end())
          {
            connection.second.calculationEnabled = false;
          }
        }
        
        connection.second.numBoardings                       = 0;
        connection.second.totalInVehicleTravelTimeMinutes    = 0;
        connection.second.totalNotInVehicleTravelTimeMinutes = 0;
        totalNumberOfJourneySteps += connection.second.journeySteps.size();
        connection.second.journeySteps.resize(0);
        connection.second.journeySteps.shrink_to_fit();
      }
    }
    else // reverse calculation
    {
      for (auto & connection : reverseConnectionsById)
      {
        connection.second.reachable          = 0;
        connection.second.calculationEnabled = connection.second.enabled == 1 ? true : false;
        
        if (hasOnlyServices)
        {
          if (params.onlyServiceIds.find(connection.second.serviceId) == params.onlyServiceIds.end())
          {
            connection.second.calculationEnabled = false;
          }
        }
        
        if (hasOnlyRoutes)
        {
          if (connection.second.calculationEnabled && params.onlyRouteIds.find(connection.second.routeId) == params.onlyRouteIds.end())
          {
            connection.second.calculationEnabled = false;
          }
        }
        
        if (hasOnlyRouteTypes)
        {
          if (connection.second.calculationEnabled && params.onlyRouteTypeIds.find(connection.second.routeTypeId) == params.onlyRouteTypeIds.end())
          {
            connection.second.calculationEnabled = false;
          }
        }
        
        if (hasOnlyAgencies)
        {
          if (connection.second.calculationEnabled && params.onlyAgencyIds.find(connection.second.agencyId) == params.onlyAgencyIds.end())
          {
            connection.second.calculationEnabled = false;
          }
        }
        
        if (hasExceptServices)
        {
          if (connection.second.calculationEnabled && params.exceptServiceIds.find(connection.second.serviceId) != params.exceptServiceIds.end())
          {
            connection.second.calculationEnabled = false;
          }
        }
        
        if (hasExceptRoutes)
        {
          if (connection.second.calculationEnabled && params.exceptRouteIds.find(connection.second.routeId) != params.exceptRouteIds.end())
          {
            connection.second.calculationEnabled = false;
          }
        }
        
        if (hasExceptRouteTypes)
        {
          if (connection.second.calculationEnabled && params.exceptRouteTypeIds.find(connection.second.routeTypeId) != params.exceptRouteTypeIds.end())
          {
            connection.second.calculationEnabled = false;
          }
        }
        
        if (hasExceptAgencies)
        {
          if (connection.second.calculationEnabled && params.exceptAgencyIds.find(connection.second.agencyId) != params.exceptAgencyIds.end())
          {
            connection.second.calculationEnabled = false;
          }
        }
        
        connection.second.numBoardings                       = 0;
        connection.second.totalInVehicleTravelTimeMinutes    = 0;
        connection.second.totalNotInVehicleTravelTimeMinutes = 0;
        connection.second.journeySteps.resize(0);
        connection.second.journeySteps.shrink_to_fit();
        
      }
    }
    
    std::cerr << "number of journey steps = " << totalNumberOfJourneySteps << std::endl;
    
    //params.accessMode = "walking";
    //params.egressMode = "walking";
    //params.newMaxAccessWalkingTravelTimeFromOriginToFirstStopMinutes     = params.maxAccessWalkingTravelTimeFromOriginToFirstStopMinutes;
    //params.newMaxAccessWalkingTravelTimeFromLastStopToDestinationMinutes = params.maxAccessWalkingTravelTimeFromLastStopToDestinationMinutes;
    
  }
  
  std::string ConnectionScanAlgorithm::calculate(std::string tripIdentifier, const std::map<unsigned long long, int>& cachedNearestStopsIdsFromStartingPoint, const std::map<unsigned long long, int>& cachedNearestStopsIdsFromEndingPoint)
  {
    
    
      
    Connection * possibleConnectionPtr;
    std::map<std::string, long long> benchmarkTimes;
    std::vector<std::string> result;
    std::map<unsigned long long,std::vector<Connection*> > connectionsByPathStopSequenceId;
    std::map<unsigned long long,Connection>* connectionsById;
    std::vector<Connection*>* connections;
    int startTime;
    int forwardFlag {params.forwardCalculation};
    int reverseFlag {!params.forwardCalculation};
    
    // boost formatter for hours and minutes: add padding zeros
    boost::format padWithZeros("%02i");
    
    if(params.startingStopId != -1)
    {
      params.startingPoint = stopsById[params.startingStopId].point;
    }
    if(params.endingStopId != -1)
    {
      params.endingPoint = stopsById[params.endingStopId].point;
    }
    
    if (params.forwardCalculation)
    {
      startTime = params.departureTimeHour * 60 + params.departureTimeMinutes;
      connections     = &connectionsByDepartureTime;
      connectionsById = &forwardConnectionsById;
      connectionsByPathStopSequenceId = connectionsByStartPathStopSequenceId;
    }
    else
    {
      startTime = maxTimeValue - (params.arrivalTimeHour * 60 + params.arrivalTimeMinutes);
      connections     = &connectionsByArrivalTime;
      connectionsById = &reverseConnectionsById;
      connectionsByPathStopSequenceId = connectionsByEndPathStopSequenceId;
      std::swap(params.startingPoint, params.endingPoint);
    }
    
    int maxWalkingDurationAtEndingStops {0};
    int maxArrivalTimeAtEndingStops {maxTimeValue};
    int maxArrivalTime {maxTimeValue};
    if (params.maxTotalTravelTimeMinutes > 0)
    {
      maxArrivalTime = startTime + params.maxTotalTravelTimeMinutes;
    }
    if (maxArrivalTime > maxTimeValue)
    {
      maxArrivalTime = maxTimeValue;
    }
    
    bool foundMaxArrivalTime {false};
    
    std::map<unsigned long long, int> nearestStopsIdsFromStartingPoint;
    std::map<unsigned long long, int> nearestStopsIdsFromEndingPoint;
    std::pair<int,int>                walkingTravelTimeAndDistance{std::make_pair(-1,-1)};
    
      
      
    if(params.startingStopId == -1) // if starting point is not set
    {
      if (!cachedNearestStopsIdsFromStartingPoint.empty())
      {
        nearestStopsIdsFromStartingPoint = cachedNearestStopsIdsFromStartingPoint;
      }
      else
      {
        nearestStopsIdsFromStartingPoint = DbFetcher::getNearestStopsIds(params.applicationShortname, params.dataFetcher, params.startingPoint, stopsById, params, accessMode, maxAccessWalkingTravelTimeFromOriginToFirstStopMinutes);
      }
    }
    else // if starting stop is forced
    {
      nearestStopsIdsFromStartingPoint[params.startingStopId] = 0;
    }
    
      
      
    if(!params.returnAllStopsResult)
    {
      //if(params.endingStopId == -1)
      //{
      
      if (!cachedNearestStopsIdsFromEndingPoint.empty())
      {
        nearestStopsIdsFromEndingPoint = cachedNearestStopsIdsFromEndingPoint;
      }
      else
      {
        nearestStopsIdsFromEndingPoint = DbFetcher::getNearestStopsIds(params.applicationShortname, params.dataFetcher, params.endingPoint, stopsById, params, egressMode, maxAccessWalkingTravelTimeFromLastStopToDestinationMinutes);
      }
      
      //}
      //else
      //{
      //  nearestStopsIdsFromEndingPoint[params.endingStopId] = 0;
      //}
      for(auto & walkableStopFromEndingPoint : nearestStopsIdsFromEndingPoint)
      {
        
        // stop can be unboarded to reach destination
        stopsById[walkableStopFromEndingPoint.first].canUnboardToDestination = true;
        
        // set the maximum arrival time at ending stops to limit calculation time:
        if (maxWalkingDurationAtEndingStops < walkableStopFromEndingPoint.second)
        {
          maxWalkingDurationAtEndingStops = walkableStopFromEndingPoint.second;
        }
        
      }
      std::cout << "-- Max walking duration from ending stop to destination: " << maxWalkingDurationAtEndingStops << " minutes --" << std::endl;
      
      // calculate walking travel time for the whole trip:
      walkingTravelTimeAndDistance = DbFetcher::getTripTravelTimeAndDistance(params.startingPoint, params.endingPoint, "walking", params);
      
    }
    
      
      
    std::map<long long, int> accessedStartSequenceByTripIds;
    std::vector<std::pair<long long, int>> sortedNearestStopsIdsFromStartingPointPairs;
    
    for(auto & walkableStopFromStartingPoint : nearestStopsIdsFromStartingPoint)
    {
      sortedNearestStopsIdsFromStartingPointPairs.emplace_back(std::make_pair(walkableStopFromStartingPoint.first, walkableStopFromStartingPoint.second));
    }
      
      
      
      
    if (params.startingStopId == -1)
    {
      // sort stops by walking travel time, so we can minimize travel time for same trip boarding
      sort(sortedNearestStopsIdsFromStartingPointPairs.begin(), sortedNearestStopsIdsFromStartingPointPairs.end(), [=](std::pair<long long, int>& a, std::pair<long long, int>& b)
      {
        return a.second < b.second;
      });
    }
    
      
      
      
    for(auto & walkableStopFromStartingPoint : sortedNearestStopsIdsFromStartingPointPairs)
    {
      
      stopsById[walkableStopFromStartingPoint.first].arrivalTimeMinuteOfDay = walkableStopFromStartingPoint.second + startTime;
      SimplifiedJourneyStep newWalkJourneyStep;
      newWalkJourneyStep.id                      = ++journeyStepId;
      newWalkJourneyStep.action                  = WALK;
      newWalkJourneyStep.connectionId            = -1;
      newWalkJourneyStep.accessTimeMinutes       = walkableStopFromStartingPoint.second;
      newWalkJourneyStep.accessFromStopId        = -1;
      newWalkJourneyStep.accessFromTripId        = -1;
      newWalkJourneyStep.readyToBoardMinuteOfDay = walkableStopFromStartingPoint.second + startTime + forwardFlag * params.minWaitingTimeMinutes;
      stopsById[walkableStopFromStartingPoint.first].journeySteps.emplace_back(std::make_shared<SimplifiedJourneyStep>(newWalkJourneyStep));
      stopsById[walkableStopFromStartingPoint.first].totalNotInVehicleTravelTimeMinutes += walkableStopFromStartingPoint.second;
      
        
        
      for(auto & transferablePathStopSequenceId : pathStopSequencesByStopId[walkableStopFromStartingPoint.first])
      {
        
        for(auto & transferableConnection : connectionsByPathStopSequenceId[transferablePathStopSequenceId])
        {
          
          possibleConnectionPtr = transferableConnection;
          
          if (possibleConnectionPtr->calculationEnabled
            && possibleConnectionPtr->departureFromOriginTimeMinuteOfDay >= walkableStopFromStartingPoint.second + startTime + forwardFlag * params.minWaitingTimeMinutes
            && possibleConnectionPtr->canBoard
            //&& (!accessedStartSequenceByTripIds.count(possibleConnectionPtr->tripId) || (accessedStartSequenceByTripIds.count(possibleConnectionPtr->tripId) && accessedStartSequenceByTripIds[possibleConnectionPtr->tripId] > possibleConnectionPtr->sequence) ) // minimize walking access travel time
          )
          {
            //accessedStartSequenceByTripIds[possibleConnectionPtr->tripId] = possibleConnectionPtr->sequence;
            possibleConnectionPtr->reachable    = calculationId;
            possibleConnectionPtr->numBoardings = stopsById[walkableStopFromStartingPoint.first].numBoardings;
            possibleConnectionPtr->totalInVehicleTravelTimeMinutes    = stopsById[walkableStopFromStartingPoint.first].totalInVehicleTravelTimeMinutes;
            possibleConnectionPtr->totalNotInVehicleTravelTimeMinutes = stopsById[walkableStopFromStartingPoint.first].totalNotInVehicleTravelTimeMinutes;
            possibleConnectionPtr->journeySteps = stopsById[walkableStopFromStartingPoint.first].journeySteps;
            
            SimplifiedJourneyStep newBoardJourneyStep;
            newBoardJourneyStep.id                      = ++journeyStepId;
            newBoardJourneyStep.action                  = BOARD;
            newBoardJourneyStep.connectionId            = possibleConnectionPtr->id;
            newBoardJourneyStep.accessTimeMinutes       = -1;
            newBoardJourneyStep.accessFromStopId        = -1;
            newBoardJourneyStep.accessFromTripId        = -1;
            newBoardJourneyStep.readyToBoardMinuteOfDay = -1;
            possibleConnectionPtr->numBoardings += 1;
            possibleConnectionPtr->journeySteps.emplace_back(std::make_shared<SimplifiedJourneyStep>(newBoardJourneyStep));
            break;
          }
        }
      }
    }
    
    sortedNearestStopsIdsFromStartingPointPairs.empty();
    accessedStartSequenceByTripIds.clear();
    
    // main loop:
    for(auto & connection : *connections)
    {
      
      if (connection->reachable == calculationId && connection->calculationEnabled) // select reachable connections
      {
        
        PathStopSequence pathStopSequence;
        
        // stop calculating if max arrival time is reached
        if (connection->departureFromOriginTimeMinuteOfDay > maxArrivalTime)
        {
          break;
        }
        
        // stop calculating if max arrival time at ending stop is reached
        if(foundMaxArrivalTime)
        {
          if (connection->departureFromOriginTimeMinuteOfDay > maxArrivalTimeAtEndingStops)
          {
            std::cout << "breaking because max arrival time found\n";
            break;
          }
        } else if(!params.returnAllStopsResult) {
          
          // as soon as we reach one ending stop, limit the calculation to arrival time at this stop 
          // + max walking duration to reach destination from all ending stops
          if(stopsById[connection->stopEndId].canUnboardToDestination)
          {
            maxArrivalTimeAtEndingStops = connection->arrivalAtDestinationTimeMinuteOfDay + maxWalkingDurationAtEndingStops;
            foundMaxArrivalTime         = true;
            std::cout << "Found max arrival time at ending stops: " << maxArrivalTimeAtEndingStops << std::endl;
          }
        }
        
        SimplifiedJourneyStep newRideJourneyStep;
        newRideJourneyStep.id                      = ++journeyStepId;
        newRideJourneyStep.action                  = RIDE;
        newRideJourneyStep.connectionId            = connection->id;
        newRideJourneyStep.accessTimeMinutes       = -1;
        newRideJourneyStep.accessFromStopId        = -1;
        newRideJourneyStep.accessFromTripId        = -1;
        newRideJourneyStep.readyToBoardMinuteOfDay = -1;
        connection->journeySteps.emplace_back(std::make_shared<SimplifiedJourneyStep>(newRideJourneyStep));
        
        if (connection->nextConnectionId != -1)
        {
          
          possibleConnectionPtr = &(*connectionsById)[connection->nextConnectionId];
          
          if(possibleConnectionPtr->calculationEnabled)
          {
            if (possibleConnectionPtr->reachable == calculationId) // we already rode that connection. Make sure this time, we can ride it with less boardings:
            {
              if (possibleConnectionPtr->numBoardings >= connection->numBoardings)
              {
                if (possibleConnectionPtr->numBoardings == connection->numBoardings)
                {
                  if (possibleConnectionPtr->totalNotInVehicleTravelTimeMinutes > connection->totalNotInVehicleTravelTimeMinutes)
                  {
                    possibleConnectionPtr->numBoardings = connection->numBoardings;
                    possibleConnectionPtr->totalInVehicleTravelTimeMinutes    = connection->totalInVehicleTravelTimeMinutes + possibleConnectionPtr->arrivalAtDestinationTimeMinuteOfDay - connection->arrivalAtDestinationTimeMinuteOfDay; // yes, arrival two times to keep dwell time into account
                    possibleConnectionPtr->totalNotInVehicleTravelTimeMinutes = connection->totalNotInVehicleTravelTimeMinutes;
                    possibleConnectionPtr->journeySteps = connection->journeySteps;
                  }
                  //else
                  //{
                  //  
                  //}
                }
                else if (possibleConnectionPtr->numBoardings > connection->numBoardings)
                {
                  possibleConnectionPtr->numBoardings = connection->numBoardings;
                  possibleConnectionPtr->totalInVehicleTravelTimeMinutes    = connection->totalInVehicleTravelTimeMinutes + possibleConnectionPtr->arrivalAtDestinationTimeMinuteOfDay - connection->arrivalAtDestinationTimeMinuteOfDay; // yes, arrival two times to keep dwell time into account
                  possibleConnectionPtr->totalNotInVehicleTravelTimeMinutes = connection->totalNotInVehicleTravelTimeMinutes;
                  possibleConnectionPtr->journeySteps = connection->journeySteps;
                }
              }
            }
            else
            {
              possibleConnectionPtr->reachable    = calculationId;
              possibleConnectionPtr->numBoardings = connection->numBoardings;
              possibleConnectionPtr->totalInVehicleTravelTimeMinutes    = connection->totalInVehicleTravelTimeMinutes + possibleConnectionPtr->arrivalAtDestinationTimeMinuteOfDay - connection->arrivalAtDestinationTimeMinuteOfDay; // yes, arrival two times to keep dwell time into account
              possibleConnectionPtr->totalNotInVehicleTravelTimeMinutes = connection->totalNotInVehicleTravelTimeMinutes;
              possibleConnectionPtr->journeySteps = connection->journeySteps;
            }
          }
        }
        
        if( connection->arrivalAtDestinationTimeMinuteOfDay <= stopsById[connection->stopEndId].arrivalTimeMinuteOfDay && connection->canUnboard )
        {
         
          stopsById[connection->stopEndId].arrivalTimeMinuteOfDay = connection->arrivalAtDestinationTimeMinuteOfDay;
          stopsById[connection->stopEndId].numBoardings = connection->numBoardings;
          stopsById[connection->stopEndId].totalInVehicleTravelTimeMinutes    = connection->totalInVehicleTravelTimeMinutes;
          stopsById[connection->stopEndId].totalNotInVehicleTravelTimeMinutes = connection->totalNotInVehicleTravelTimeMinutes;
          stopsById[connection->stopEndId].journeySteps = connection->journeySteps;
          
          SimplifiedJourneyStep newUnboardJourneyStep;
          newUnboardJourneyStep.id                      = ++journeyStepId;
          newUnboardJourneyStep.action                  = UNBOARD;
          newUnboardJourneyStep.connectionId            = connection->id;
          newUnboardJourneyStep.accessTimeMinutes       = -1;
          newUnboardJourneyStep.accessFromStopId        = -1;
          newUnboardJourneyStep.accessFromTripId        = -1;
          newUnboardJourneyStep.readyToBoardMinuteOfDay = connection->arrivalAtDestinationTimeMinuteOfDay + params.minWaitingTimeMinutes;
          
          stopsById[connection->stopEndId].journeySteps.emplace_back(std::make_shared<SimplifiedJourneyStep>(newUnboardJourneyStep));
          
          // get transferable stops:
          // transfer only if max number of transfers has not been reached: BUT THAT IS WRONG: we could get to that connection with less boardings. There is still no way to really set a maximum number of transfers in csa for now...
          if (params.maxNumberOfTransfers == -1 || (params.maxNumberOfTransfers >= 0 && params.maxNumberOfTransfers > connection->numBoardings - 1))
          {
            
            for(auto & transferableStopId : transferDurationsByStopId[connection->stopEndId])
            {
              
              if (transferableStopId.second <= params.maxTransferWalkingTravelTimeMinutes
                && stopsById[transferableStopId.first].arrivalTimeMinuteOfDay >= transferableStopId.second + connection->arrivalAtDestinationTimeMinuteOfDay
                && (stopsById[connection->stopEndId].journeySteps.back())->action != WALK
              ) // ignore stops too far to walk or already accessed before
              {
                
                if (
                    stopsById[transferableStopId.first].arrivalTimeMinuteOfDay == transferableStopId.second + connection->arrivalAtDestinationTimeMinuteOfDay
                    &&
                    (stopsById[connection->stopEndId].journeySteps.back())->accessTimeMinutes < transferableStopId.second
                    &&
                    (stopsById[connection->stopEndId].journeySteps.back())->accessFromTripId == connection->tripId
                  )
                {
                  
                  continue; // ignore transfer if walking time would be greater (but with the same arrival time)
                }
                
                stopsById[transferableStopId.first].arrivalTimeMinuteOfDay = transferableStopId.second + connection->arrivalAtDestinationTimeMinuteOfDay;
                
                
                
                if(transferableStopId.first != connection->stopEndId)
                {
                  
                  stopsById[transferableStopId.first].numBoardings = stopsById[connection->stopEndId].numBoardings;
                  stopsById[transferableStopId.first].totalInVehicleTravelTimeMinutes    = stopsById[connection->stopEndId].totalInVehicleTravelTimeMinutes;
                  stopsById[transferableStopId.first].totalNotInVehicleTravelTimeMinutes = stopsById[connection->stopEndId].totalNotInVehicleTravelTimeMinutes;
                  stopsById[transferableStopId.first].journeySteps = stopsById[connection->stopEndId].journeySteps;
                  
                  SimplifiedJourneyStep newWalkJourneyStep;
                  
                  newWalkJourneyStep.id                      = ++journeyStepId;
                  newWalkJourneyStep.action                  = WALK;
                  newWalkJourneyStep.connectionId            = -1;
                  newWalkJourneyStep.accessTimeMinutes       = transferableStopId.second;
                  newWalkJourneyStep.accessFromStopId        = connection->stopEndId;
                  newWalkJourneyStep.accessFromTripId        = connection->tripId;
                  newWalkJourneyStep.readyToBoardMinuteOfDay = transferableStopId.second + params.minWaitingTimeMinutes + connection->arrivalAtDestinationTimeMinuteOfDay;
                  stopsById[transferableStopId.first].journeySteps.emplace_back(std::make_shared<SimplifiedJourneyStep>(newWalkJourneyStep));
                  stopsById[transferableStopId.first].totalNotInVehicleTravelTimeMinutes += transferableStopId.second;
                  
                }
                
                for(auto & transferablePathStopSequenceId : pathStopSequencesByStopId[transferableStopId.first])
                {
                  
                  if (!params.transferBetweenSameRoute)
                  {
                    pathStopSequence = pathStopSequencesById[transferablePathStopSequenceId];
                    if (pathStopSequence.routeId == connection->routeId)
                    {
                      continue;
                    }
                  }
                  if (params.transferOnlyAtSameStation)
                  {
                    if(stopsById[transferableStopId.first].stationId != stopsById[connection->stopEndId].stationId)
                    {
                      continue;
                    }
                  }
                  
                  for(auto & transferableConnection : connectionsByPathStopSequenceId[transferablePathStopSequenceId])
                  {
                    
                    possibleConnectionPtr = transferableConnection;
                    
                    if (
                       possibleConnectionPtr->calculationEnabled
                       && possibleConnectionPtr->departureFromOriginTimeMinuteOfDay >= transferableStopId.second + connection->arrivalAtDestinationTimeMinuteOfDay + params.minWaitingTimeMinutes 
                       && possibleConnectionPtr->canBoard
                       && possibleConnectionPtr->tripId != connection->tripId
                    ) // connection must be departing later, you should be able to board and you must not board the same trip again
                    {
                      
                      possibleConnectionPtr->reachable    = calculationId;
                      possibleConnectionPtr->numBoardings = stopsById[transferableStopId.first].numBoardings;
                      possibleConnectionPtr->totalInVehicleTravelTimeMinutes    = stopsById[transferableStopId.first].totalInVehicleTravelTimeMinutes;
                      possibleConnectionPtr->totalNotInVehicleTravelTimeMinutes = stopsById[transferableStopId.first].totalNotInVehicleTravelTimeMinutes;
                      possibleConnectionPtr->journeySteps = stopsById[transferableStopId.first].journeySteps;
                      
                      SimplifiedJourneyStep newBoardJourneyStep;
                      
                      newBoardJourneyStep.id                      = ++journeyStepId;
                      newBoardJourneyStep.action                  = BOARD;
                      newBoardJourneyStep.connectionId            = possibleConnectionPtr->id;
                      newBoardJourneyStep.accessTimeMinutes       = -1;
                      newBoardJourneyStep.accessFromStopId        = -1;
                      newBoardJourneyStep.accessFromTripId        = -1;
                      newBoardJourneyStep.readyToBoardMinuteOfDay = -1;
                      possibleConnectionPtr->numBoardings += 1;
                      possibleConnectionPtr->journeySteps.emplace_back(std::make_shared<SimplifiedJourneyStep>(newBoardJourneyStep));
                      
                      break;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
    
    std::string jsonResult = "";
    std::string jsonResultWithNumberOfTransfers = "";
    
    std::map<int, std::string> enumMap;
    enumMap[0] = "Walk";
    enumMap[1] = "Cycle";
    enumMap[2] = "Drive";
    enumMap[3] = "Ride";
    enumMap[4] = "Board";
    enumMap[5] = "Unboard";
    
    // return all stops result:
    if (params.returnAllStopsResult)
    {
      
      jsonResult += "{\n  \"stops\": \n  [";
      int stopArrivalTime;
      int travelTimeMinutes;
      Connection connection;
      
      std::string routeIdsStr;
      std::string tripIdsStr;
      std::string routeIdStopPairIdsStr;
      std::string transfersRoutePairIdsStr;
      long long transferFromRouteId;
      int numRoutes;
      
      int  numberOfTransfers;
      int  numberOfBoardings;
      int  firstBoardingMinuteOfDay;
      int  segmentInVehicleTimeMinutes;
      int  totalInVehicleTimeMinutes;
      int  totalWalkingTimeMinutes;
      int  transferWalkingTimeMinutes;
      int  totalWaitingTimeMinutes;
      int  transferWaitingTimeMinutes;
      int  accessWalkingTimeMinutes;
      int  firstWaitingTimeMinutes;
      int  egressWalkingTimeMinutes;
      int  lastReadyToBoardAtMinuteOfDay;
      bool accessedFirstStop;
      int  waitingTimeMinutes;
      unsigned long long countReachableStops {0};
      //SimplifiedJourneyStep journeyStep;
      
      for (auto & stop : stopsById)
      {
        
        if (params.startingStopId != -1 && stop.first == params.startingStopId)
        {
          continue; // ignore travel time to same stop
        }

        if (params.forwardCalculation)
        {
          stopArrivalTime   = stop.second.arrivalTimeMinuteOfDay;
          travelTimeMinutes = stopArrivalTime - startTime;
        }
        else
        {
          stopArrivalTime = maxTimeValue - stop.second.arrivalTimeMinuteOfDay;
          travelTimeMinutes = maxTimeValue - startTime - stopArrivalTime;
          
        }
        
        if (params.maxTotalTravelTimeMinutes > 0 && travelTimeMinutes > params.maxTotalTravelTimeMinutes)
        {
          continue; // ignore if travel time is too long
        }
        
        if (params.detailedResults)
        {
          
          routeIdsStr                   = "[";
          tripIdsStr                    = "[";
          routeIdStopPairIdsStr         = "[";
          transfersRoutePairIdsStr      = "[";
          transferFromRouteId           = -1;
          numRoutes                     = 0;
          
          numberOfTransfers             = -1;
          numberOfBoardings             = 0;
          firstBoardingMinuteOfDay      = -1;
          segmentInVehicleTimeMinutes   = 0;
          totalInVehicleTimeMinutes     = 0;
          totalWalkingTimeMinutes       = 0;
          transferWalkingTimeMinutes    = 0;
          totalWaitingTimeMinutes       = 0;
          transferWaitingTimeMinutes    = 0;
          accessWalkingTimeMinutes      = 0;
          firstWaitingTimeMinutes       = 0;
          egressWalkingTimeMinutes      = 0;
          lastReadyToBoardAtMinuteOfDay = -1;
          accessedFirstStop             = false;
          waitingTimeMinutes            = 0;
          
          //std::cout << stop.second.name << ": " << stopArrivalTime << std::endl;
          if (stopArrivalTime < maxTimeValue && stop.second.journeySteps.size() > 0)
          {
            
            countReachableStops++;
            
            int numberOfTransfers{-1};
            for(auto & journeyStep : stop.second.journeySteps)
            {
              
              if(journeyStep->connectionId != -1)
              {
                
                if(enumMap[journeyStep->action] == "Ride")
                {
                  connection = (*connectionsById)[journeyStep->connectionId];
                  routeIdStopPairIdsStr += "{ \"routeId\": " + std::to_string(connection.routeId) + ", \"stopIdsPair\": [" + std::to_string(connection.stopStartId) + "," + std::to_string(connection.stopEndId) +"]},";
                  segmentInVehicleTimeMinutes += connection.arrivalAtDestinationTimeMinuteOfDay - connection.departureFromOriginTimeMinuteOfDay;
                  totalInVehicleTimeMinutes   += connection.arrivalAtDestinationTimeMinuteOfDay - connection.departureFromOriginTimeMinuteOfDay;
                }
                else if(enumMap[journeyStep->action] == "Unboard")
                {
                  connection = (*connectionsById)[journeyStep->connectionId];
                  transferFromRouteId            = connection.routeId;
                  lastReadyToBoardAtMinuteOfDay  = journeyStep->readyToBoardMinuteOfDay;
                }
                else if(enumMap[journeyStep->action] == "Walk")
                {
                  if(!accessedFirstStop) // first walking
                  {
                    accessWalkingTimeMinutes = journeyStep->accessTimeMinutes;
                  }
                  
                  if(journeyStep->accessFromStopId != -1)
                  {
                    
                    if(journeyStep->readyToBoardMinuteOfDay != -1)
                    {
                      transferWalkingTimeMinutes += journeyStep->accessTimeMinutes;
                    }
                    else
                    {
                      egressWalkingTimeMinutes = journeyStep->accessTimeMinutes;
                    }
                  }
                  
                  totalWalkingTimeMinutes       += journeyStep->accessTimeMinutes;
                  lastReadyToBoardAtMinuteOfDay  = journeyStep->readyToBoardMinuteOfDay;
                
                }
                else if(enumMap[journeyStep->action] == "Board")
                {
                  connection = (*connectionsById)[journeyStep->connectionId];
                  numRoutes++;
                  numberOfTransfers++;
                  routeIdsStr              += std::to_string(connection.routeId) + ",";
                  tripIdsStr               += std::to_string(connection.tripId)  + ",";
                  segmentInVehicleTimeMinutes = 0;
                  
                  if(transferFromRouteId > 0)
                  {
                    transfersRoutePairIdsStr += "[" + std::to_string(transferFromRouteId) + "," + std::to_string(connection.routeId) + "],";
                  }
                  
                  if (firstBoardingMinuteOfDay == -1) // first boarding
                  {
                    firstBoardingMinuteOfDay = connection.departureFromOriginTimeMinuteOfDay;
                  }
                  
                  if (accessedFirstStop && lastReadyToBoardAtMinuteOfDay != -1)
                  {
                    waitingTimeMinutes = connection.departureFromOriginTimeMinuteOfDay - lastReadyToBoardAtMinuteOfDay + params.minWaitingTimeMinutes;
                  }
                  else if (!accessedFirstStop && lastReadyToBoardAtMinuteOfDay != -1) 
                  {
                    waitingTimeMinutes = connection.departureFromOriginTimeMinuteOfDay - lastReadyToBoardAtMinuteOfDay + params.minWaitingTimeMinutes;
                    firstWaitingTimeMinutes  = waitingTimeMinutes;
                    totalWaitingTimeMinutes += waitingTimeMinutes;
                  }
                  
                  accessedFirstStop = true;

                }
                
              }
            }
            
            // remove last comma:
            if (numRoutes >= 1)
            {
              routeIdsStr.pop_back();
              routeIdStopPairIdsStr.pop_back();
              tripIdsStr.pop_back();
            }
            if (numberOfTransfers >= 1)
            {
              transfersRoutePairIdsStr.pop_back();
            }
            routeIdsStr              += "]";
            tripIdsStr               += "]";
            routeIdStopPairIdsStr    += "]";
            transfersRoutePairIdsStr += "]";
            
            jsonResult += "\n    { \"id\": " + std::to_string(stop.first) + ", \"code\": \"" + stop.second.code + "\", \"name\": \"" + stop.second.name + "\", \"stopCoordinates\": [" + std::to_string(stop.second.point.latitude) + "," + std::to_string(stop.second.point.longitude) + "], \"" + (params.forwardCalculation ? "arrivalTime" : "departureTime") + "\":"
            + " \"" + boost::str(padWithZeros % (stopArrivalTime / 60)) + ":" + boost::str(padWithZeros % (stopArrivalTime % 60)) + "\",\n"
            + " \"totalTravelTimeSeconds\": "                      + std::to_string(travelTimeMinutes * 60) + ",\n"
            + " \"totalInVehicleTimeSeconds\": "                   + std::to_string(totalInVehicleTimeMinutes * 60) + ",\n"
            + " \"totalNonTransitTravelTimeSeconds\": "            + std::to_string(totalWalkingTimeMinutes * 60) + ",\n"
            + " \"transferWaitingTimeSeconds\": "                  + std::to_string(transferWaitingTimeMinutes * 60) + ",\n"
            + " \"transferWalkingTimeSeconds\": "                  + std::to_string(transferWalkingTimeMinutes * 60) + ",\n"
            + " \"numberOfBoardings\": "                           + std::to_string(stop.second.numBoardings) + ",\n"
            + " \"numberOfTransfers\": "                           + std::to_string(numberOfTransfers) + ",\n"
            + " \"accessTravelTimeSeconds\": "                     + std::to_string(accessWalkingTimeMinutes * 60) + ",\n"
            + " \"egressTravelTimeSeconds\": "                     + std::to_string(egressWalkingTimeMinutes * 60) + ",\n"
            + " \"firstWaitingTimeSeconds\": "                     + std::to_string(firstWaitingTimeMinutes * 60) + ",\n"
            + " \"totalWaitingTimeSeconds\": "                     + std::to_string(totalWaitingTimeMinutes * 60) + ",\n"
            + " \"legs\": "                                        + routeIdStopPairIdsStr + ",\n"
            + " \"transfersRouteIds\": "                           + transfersRoutePairIdsStr + ",\n"
            + " \"routeIds\": "                                    + routeIdsStr + ",\n"
            + " \"tripIds\": "                                     + tripIdsStr  + " },";
            
          }
          else // routing failed
          {
            //jsonResult += "\n    { \"id\": " + std::to_string(stop.first) + ", \"code\": \"" + stop.second.code + "\", \"name\": \"" + stop.second.name + "\", \"arrival_time\":"
            //+ " \"unreachable\","
            //+ " \"travel_time_minutes\": -1,"
            //+ " \"travel_time_seconds\": -1 },";
          }
        
        }
        else // no details results
        {
          
          if (stopArrivalTime < maxTimeValue && stop.second.journeySteps.size() > 0)
          {
            
            countReachableStops++;
            
            jsonResult += "\n    { \"id\": " + std::to_string(stop.first) + ", \"" + (params.forwardCalculation ? "arrivalTime" : "departureTime") + "\":"
            + " \"" + boost::str(padWithZeros % (stopArrivalTime / 60)) + ":" + boost::str(padWithZeros % (stopArrivalTime % 60)) + "\","
            + " \"totalTravelTimeSeconds\": " + std::to_string(travelTimeMinutes * 60) + ","
            + " \"numberOfTransfers\": "      + std::to_string(stop.second.numBoardings - 1) + "},";
          
          }
          else // routing failed
          {
            //jsonResult += "\n    { \"id\": " + std::to_string(stop.first) + ", \"code\": \"" + stop.second.code + "\", \"name\": \"" + stop.second.name + "\", \"arrival_time\":"
            //+ " \"unreachable\","
            //+ " \"travel_time_minutes\": -1,"
            //+ " \"travel_time_seconds\": -1 },";
          }
        }
        
      }

      jsonResult.pop_back(); // remove trailing comma
      jsonResult += "\n  ],\n";
      
      jsonResult += "  \"numberOfReachableStops\": "  + std::to_string(countReachableStops) + ",\n";
      jsonResult += "  \"percentOfReachableStops\": " + std::to_string(round(10000 * (float)countReachableStops / (float)(stopsById.size()))/100.0) + ",\n";
      
      algorithmCalculationTime.stop();
      jsonResult += "  \"calculatedInMilliseconds\": " + std::to_string(algorithmCalculationTime.getDurationMillisecondsNoStop()) + "\n";
      jsonResult += "}";
      
      std::cerr << "-- calculation time -- " << algorithmCalculationTime.getDurationMillisecondsNoStop() << " ms\n";

      
      return jsonResult;
      
    }
    
    Connection connection;
    //SimplifiedJourneyStep journeyStep;
    Stop stopStart;
    Stop stopEnd;
    Route route;
    
    std::cout << std::fixed;
    std::cout << std::setprecision(6);
    
    long long minTravelTimeStopId {-1};
    int minArrivalTime {maxTimeValue};
    int totalOnlyWalkingTimeMinutes {-1};
    
    // get total walking travel time if available:
    if (walkingTravelTimeAndDistance.first >= 0)
    {
      totalOnlyWalkingTimeMinutes = walkingTravelTimeAndDistance.first;
    }
    
    bool atLeastOneRoutingFound {false};
    int accessTimeMinutes {-1};
    std::vector<std::shared_ptr<SimplifiedJourneyStep> > destinationJourneySteps;
    Stop destinationStop;
    bool foundDestinationStop {false};
    
    for(auto & walkableStopFromEndingPoint : nearestStopsIdsFromEndingPoint)
    {
      
      if(stopsById[walkableStopFromEndingPoint.first].journeySteps.size() > 0 && (stopsById[walkableStopFromEndingPoint.first].journeySteps.back())->action == WALK)
      {
        continue;
        //stopsById[walkableStopFromEndingPoint.first].journeySteps = stopsById[walkableStopFromEndingPoint.first].previousJourneySteps;
      }
      
      if(minArrivalTime > walkableStopFromEndingPoint.second + stopsById[walkableStopFromEndingPoint.first].arrivalTimeMinuteOfDay + reverseFlag * params.minWaitingTimeMinutes)
      {
        minArrivalTime      = walkableStopFromEndingPoint.second + stopsById[walkableStopFromEndingPoint.first].arrivalTimeMinuteOfDay + reverseFlag * params.minWaitingTimeMinutes;
        minTravelTimeStopId = walkableStopFromEndingPoint.first;
        accessTimeMinutes   = walkableStopFromEndingPoint.second;
      }
    }
    
    if (minTravelTimeStopId != -1)
    {
      
      destinationStop         = stopsById[minTravelTimeStopId];
      destinationJourneySteps = destinationStop.journeySteps;
      foundDestinationStop = true;
      
      SimplifiedJourneyStep newWalkJourneyStep;
      newWalkJourneyStep.id                      = ++journeyStepId;
      newWalkJourneyStep.action                  = WALK;
      newWalkJourneyStep.connectionId            = -1;
      newWalkJourneyStep.accessTimeMinutes       = accessTimeMinutes;
      newWalkJourneyStep.accessFromStopId        = minTravelTimeStopId;
      newWalkJourneyStep.accessFromTripId        = -1;
      newWalkJourneyStep.readyToBoardMinuteOfDay = -1;
      destinationJourneySteps.emplace_back(std::make_shared<SimplifiedJourneyStep>(newWalkJourneyStep));
    }
    
    bool foundResult = minArrivalTime < maxTimeValue && (params.maxTotalTravelTimeMinutes <= 0 || (params.maxTotalTravelTimeMinutes > 0 && (minArrivalTime - startTime) <= params.maxTotalTravelTimeMinutes));
    int  numberOfTransfers {-1};
    
    // save results to json:
    
    // try another mode and farther access/egress time if routing failed:
    if(params.tryNextModeIfRoutingFails && maxAccessWalkingTravelTimeFromOriginToFirstStopMinutes <= params.maxNoResultNextAccessTimeMinutes && minArrivalTime >= maxTimeValue) // routing failed
    {
      
      std::cout << "No routing found! " << std::endl;
      
      if(accessMode == params.noResultSecondMode)
      {
        maxAccessWalkingTravelTimeFromOriginToFirstStopMinutes     += params.noResultNextAccessTimeMinutesIncrement;
        maxAccessWalkingTravelTimeFromLastStopToDestinationMinutes += params.noResultNextAccessTimeMinutesIncrement;
      }
      accessMode = params.noResultSecondMode;
      egressMode = params.noResultSecondMode;
      
      std::cout << "access mode: " << accessMode << std::endl;
      std::cout << "new access travel time: " << maxAccessWalkingTravelTimeFromOriginToFirstStopMinutes << std::endl;
      std::cout << "new egress travel time: " << maxAccessWalkingTravelTimeFromLastStopToDestinationMinutes << std::endl;
      
      refresh();
      jsonResult = calculate(tripIdentifier);
    }
    else
    {
      
      jsonResult += "{\n";
      
      std::cerr << "walkingTravelTimeMinutes: " << walkingTravelTimeAndDistance.first << std::endl; 
      
      // return single walking step if it is faster by walking:
      if ( totalOnlyWalkingTimeMinutes >= 0 && (double)totalOnlyWalkingTimeMinutes <= maxAccessWalkingTravelTimeFromOriginToFirstStopMinutes * params.maxOnlyWalkingAccessTravelTimeRatio && totalOnlyWalkingTimeMinutes <= minArrivalTime - startTime)
      {
        
        atLeastOneRoutingFound = true;
        minArrivalTime = startTime + totalOnlyWalkingTimeMinutes;
        
        jsonResult += "  \"origin\": [" + std::to_string(params.startingPoint.latitude) + "," + std::to_string(params.startingPoint.longitude) + "],\n";
        jsonResult += "  \"destination\": [" + std::to_string(params.endingPoint.latitude) + "," + std::to_string(params.endingPoint.longitude) + "],\n";
        jsonResult += "  \"date\": \"" + std::to_string(params.routingDateYear) + "/" + boost::str(padWithZeros % (params.routingDateMonth)) + "/" + boost::str(padWithZeros % (params.routingDateDay)) + "\",\n";
      
        jsonResult += "  \"steps\":\n  [\n";
        jsonResult += "\n    {\n";
        jsonResult += "      \"action\": \"walking\",\n";
        
        jsonResult += "      \"travelTimeMinutes\": " + std::to_string(totalOnlyWalkingTimeMinutes) + ",\n";
        jsonResult += "      \"travelTimeSeconds\": " + std::to_string(totalOnlyWalkingTimeMinutes * 60) + ",\n";
        jsonResult += "      \"distanceMeters\": "    + std::to_string(walkingTravelTimeAndDistance.second) + "\n";
        
        jsonResult += "    }";
        jsonResult += "\n  ],\n";
        
        if (params.forwardCalculation)
        {
          jsonResult += "  \"departureTime\": \"" + boost::str(padWithZeros % (startTime / 60))      + ":" + boost::str(padWithZeros % (startTime % 60))      + "\",\n";
          jsonResult += "  \"arrivalTime\": \""   + boost::str(padWithZeros % (minArrivalTime / 60)) + ":" + boost::str(padWithZeros % (minArrivalTime % 60)) + "\",\n";
        }
        else
        {
          jsonResult += "  \"arrivalTime\": \"" + boost::str(padWithZeros % ((maxTimeValue - startTime) / 60))      + ":" + boost::str(padWithZeros % ((maxTimeValue - startTime) % 60))      + "\",\n";
          jsonResult += "  \"departureTime\": \""   + boost::str(padWithZeros % ((maxTimeValue - minArrivalTime) / 60)) + ":" + boost::str(padWithZeros % ((maxTimeValue - minArrivalTime) % 60)) + "\",\n";
        }
        jsonResult += "  \"totalWalkingTimeMinutesIfWalkingOnly\": "        + std::to_string(totalOnlyWalkingTimeMinutes) + ",\n";
        jsonResult += "  \"totalTravelTimeMinutes\": "                      + std::to_string(totalOnlyWalkingTimeMinutes) + ",\n";
        jsonResult += "  \"totalTravelTimeSeconds\": "                      + std::to_string(totalOnlyWalkingTimeMinutes) + ",\n";
        jsonResult += "  \"totalInVehicleTimeMinutes\": 0,\n";
        jsonResult += "  \"totalInVehicleTimeSeconds\": 0,\n";
        jsonResult += "  \"totalNonTransitTravelTimeMinutes\": "            + std::to_string(totalOnlyWalkingTimeMinutes) + ",\n";
        jsonResult += "  \"totalNonTransitTravelTimeSeconds\": "            + std::to_string(totalOnlyWalkingTimeMinutes * 60) + ",\n";
        jsonResult += "  \"numberOfBoardings\": 0,\n";
        jsonResult += "  \"numberOfTransfers\": 0,\n";
        jsonResult += "  \"maxNumberOfTransfers\": "                        + std::to_string(params.maxNumberOfTransfers) + ",\n";
        jsonResult += "  \"transferWalkingTimeMinutes\": 0,\n";
        jsonResult += "  \"transferWalkingTimeSeconds\": 0,\n";
        jsonResult += "  \"accessTravelTimeMinutes\": "                     + std::to_string(totalOnlyWalkingTimeMinutes) + ",\n";
        jsonResult += "  \"accessTravelTimeSeconds\": "                     + std::to_string(totalOnlyWalkingTimeMinutes * 60) + ",\n";
        jsonResult += "  \"egressTravelTimeMinutes\": 0,\n";
        jsonResult += "  \"egressTravelTimeSeconds\": 0,\n";
        jsonResult += "  \"transferWaitingTimeMinutes\": 0,\n";
        jsonResult += "  \"transferWaitingTimeSeconds\": 0,\n";
        jsonResult += "  \"firstWaitingTimeMinutes\": 0,\n";
        jsonResult += "  \"firstWaitingTimeSeconds\": 0,\n";
        jsonResult += "  \"totalWaitingTimeMinutes\": 0,\n";
        jsonResult += "  \"totalWaitingTimeSeconds\": 0,\n";
        jsonResult += "  \"minimumWaitingTimeBeforeEachBoardingMinutes\": " + std::to_string(params.minWaitingTimeMinutes) + ",\n";
        jsonResult += "  \"minimumWaitingTimeBeforeEachBoardingSeconds\": " + std::to_string(params.minWaitingTimeMinutes * 60) + ",\n";
        jsonResult += "  \"departureTimeToMinimizeFirstWaitingTime\": \""   + boost::str(padWithZeros % ((maxTimeValue - minArrivalTime) / 60)) + ":" + boost::str(padWithZeros % ((maxTimeValue - minArrivalTime) % 60)) + "\",\n";
        jsonResult += "  \"minimizedTotalTravelTimeMinutes\": "             + std::to_string(totalOnlyWalkingTimeMinutes) + ",\n";
        jsonResult += "  \"minimizedTotalTravelTimeSeconds\": "             + std::to_string((totalOnlyWalkingTimeMinutes) * 60) + ",\n";
        foundResult = true;
      }
      else if (foundResult)
      {
        atLeastOneRoutingFound = true;
        jsonResult += "  \"origin\": [" + std::to_string(params.startingPoint.latitude) + "," + std::to_string(params.startingPoint.longitude) + "],\n";
        jsonResult += "  \"destination\": [" + std::to_string(params.endingPoint.latitude) + "," + std::to_string(params.endingPoint.longitude) + "],\n";
        jsonResult += "  \"date\": \"" + std::to_string(params.routingDateYear) + "/" + boost::str(padWithZeros % (params.routingDateMonth)) + "/" + boost::str(padWithZeros % (params.routingDateDay)) + "\",\n";
      
        jsonResult += "  \"steps\":\n  [\n";
        
        int  numberOfBoardings {0};
        int  firstBoardingMinuteOfDay {-1};
        int  segmentInVehicleTimeMinutes {0};
        int  totalInVehicleTimeMinutes {0};
        int  totalWalkingTimeMinutes {0};
        int  transferWalkingTimeMinutes {0};
        int  totalWaitingTimeMinutes {0};
        int  transferWaitingTimeMinutes {0};
        int  accessWalkingTimeMinutes {0};
        int  firstWaitingTimeMinutes {0};
        int  egressWalkingTimeMinutes {0};
        int  lastReadyToBoardAtMinuteOfDay {-1};
        bool accessedFirstStop {false};
        int  waitingTimeMinutes {0};
        
        if(foundDestinationStop && destinationStop.arrivalTimeMinuteOfDay > 0 && destinationStop.arrivalTimeMinuteOfDay < maxTimeValue)
        {
          for(auto & journeyStep : destinationJourneySteps)
          {
            
            if(enumMap[journeyStep->action] == "Ride")
            {
              connection = (*connectionsById)[journeyStep->connectionId];
              segmentInVehicleTimeMinutes += connection.arrivalAtDestinationTimeMinuteOfDay - connection.departureFromOriginTimeMinuteOfDay;
              totalInVehicleTimeMinutes   += connection.arrivalAtDestinationTimeMinuteOfDay - connection.departureFromOriginTimeMinuteOfDay;
            }
            
            if(journeyStep->connectionId != -1 && enumMap[journeyStep->action] == "Board")
            {
              
              connection = (*connectionsById)[journeyStep->connectionId];
              stopStart  = stopsById[connection.stopStartId];
              route      = routesById[connection.routeId];
              
              numberOfTransfers++;
              segmentInVehicleTimeMinutes = 0;
              
              if (firstBoardingMinuteOfDay == -1) // first boarding
              {
                firstBoardingMinuteOfDay = connection.departureFromOriginTimeMinuteOfDay;
              }
              
              if (accessedFirstStop && lastReadyToBoardAtMinuteOfDay != -1)
              {
                waitingTimeMinutes = params.minWaitingTimeMinutes + connection.departureFromOriginTimeMinuteOfDay - lastReadyToBoardAtMinuteOfDay;
                transferWaitingTimeMinutes += waitingTimeMinutes;
                totalWaitingTimeMinutes += waitingTimeMinutes;
              }
              else if (!accessedFirstStop && lastReadyToBoardAtMinuteOfDay != -1) 
              {
                waitingTimeMinutes = params.minWaitingTimeMinutes + connection.departureFromOriginTimeMinuteOfDay - lastReadyToBoardAtMinuteOfDay;
                firstWaitingTimeMinutes = waitingTimeMinutes;
                totalWaitingTimeMinutes += waitingTimeMinutes;
              }
              
              accessedFirstStop = true;
    
              jsonResult += "\n    {\n";
              
              jsonResult += "      \"action\": \"board\",\n";
              
              jsonResult += "      \"agencyAcronym\": \"" + route.agencyAcronym + "\",\n";
              jsonResult += "      \"agencyName\": \"" + route.agencyName + "\",\n";
              jsonResult += "      \"agencyId\": \"" + std::to_string(route.agencyId) + "\",\n";
              jsonResult += "      \"routeShortname\": \"" + route.shortname + "\",\n";
              jsonResult += "      \"routeLongname\": \"" + route.longname + "\",\n";
              jsonResult += "      \"routeId\": \"" + std::to_string(route.id) + "\",\n";
              jsonResult += "      \"routeTypeName\": \"" + route.routeTypeName + "\",\n";
              jsonResult += "      \"routeTypeId\": \"" + std::to_string(route.routeTypeId) + "\",\n";
              
              jsonResult += "      \"stopName\": \"" + stopStart.name + "\",\n";
              jsonResult += "      \"stopCode\": \"" + stopStart.code + "\",\n";
              jsonResult += "      \"stopNumBoardings\": \"" + std::to_string(stopStart.numBoardings) + "\",\n";
              jsonResult += "      \"stopId\": \"" + std::to_string(stopStart.id) + "\",\n";

              jsonResult += "      \"pathStopSequenceId\": \"" + std::to_string(connection.pathStopSequenceStartId) + "\",\n";
              jsonResult += "      \"stopCoordinates\": [" + std::to_string(stopStart.point.latitude) + "," + std::to_string(stopStart.point.longitude) + "],\n";
              
              if (params.forwardCalculation)
              {
                jsonResult += "      \"departureTime\": \"" + boost::str(padWithZeros % (connection.departureFromOriginTimeMinuteOfDay / 60)) + ":" + boost::str(padWithZeros % (connection.departureFromOriginTimeMinuteOfDay % 60)) + "\",\n";
              }
              else
              {
                jsonResult += "      \"arrivalTime\": \"" + boost::str(padWithZeros % ((maxTimeValue - connection.departureFromOriginTimeMinuteOfDay) / 60)) + ":" + boost::str(padWithZeros % ((maxTimeValue - connection.departureFromOriginTimeMinuteOfDay) % 60)) + "\",\n";
              }
              
              jsonResult += "      \"waitingTimeMinutes\":"  + std::to_string(waitingTimeMinutes) + ",\n";
              jsonResult += "      \"waitingTimeSeconds\":"  + std::to_string(waitingTimeMinutes * 60) + "\n";
              
              jsonResult += "    },";
              
            }
            else if(journeyStep->connectionId != -1 && enumMap[journeyStep->action] == "Unboard")
            {
              connection = (*connectionsById)[journeyStep->connectionId];
              stopEnd    = stopsById[connection.stopEndId];
              route      = routesById[connection.routeId];
              
              lastReadyToBoardAtMinuteOfDay = journeyStep->readyToBoardMinuteOfDay;
              
              numberOfBoardings = stopEnd.numBoardings;
              
              jsonResult += "\n    {\n";
              
              jsonResult += "      \"action\": \"unboard\",\n";
              jsonResult += "      \"agencyAcronym\": \"" + route.agencyAcronym + "\",\n";
              jsonResult += "      \"agencyName\": \"" + route.agencyName + "\",\n";
              jsonResult += "      \"agencyId\": \"" + std::to_string(route.agencyId) + "\",\n";
              jsonResult += "      \"routeShortname\": \"" + route.shortname + "\",\n";
              jsonResult += "      \"routeLongname\": \"" + route.longname + "\",\n";
              jsonResult += "      \"routeId\": \"" + std::to_string(route.id) + "\",\n";
              jsonResult += "      \"routeTypeName\": \"" + route.routeTypeName + "\",\n";
              jsonResult += "      \"routeTypeId\": \"" + std::to_string(route.routeTypeId) + "\",\n";
    
              jsonResult += "      \"stopName\": \"" + stopEnd.name + "\",\n";
              jsonResult += "      \"stopCode\": \"" + stopEnd.code + "\",\n";
              jsonResult += "      \"stopNumBoardings\": \"" + std::to_string(stopEnd.numBoardings) + "\",\n";
              jsonResult += "      \"stopId\": \"" + std::to_string(stopEnd.id) + "\",\n";
              
              jsonResult += "      \"pathStopSequenceId\": \"" + std::to_string(connection.pathStopSequenceEndId) + "\",\n";
              jsonResult += "      \"stopCoordinates\": [" + std::to_string(stopEnd.point.latitude) + "," + std::to_string(stopEnd.point.longitude) + "],\n";
              
              if (params.forwardCalculation)
              {
                jsonResult += "      \"arrivalTime\": \"" + boost::str(padWithZeros % (connection.arrivalAtDestinationTimeMinuteOfDay / 60)) + ":" + boost::str(padWithZeros % (connection.arrivalAtDestinationTimeMinuteOfDay % 60)) + "\",\n";
              }
              else
              {
                jsonResult += "      \"departureTime\": \"" + boost::str(padWithZeros % ((maxTimeValue - connection.arrivalAtDestinationTimeMinuteOfDay) / 60)) + ":" + boost::str(padWithZeros % ((maxTimeValue - connection.arrivalAtDestinationTimeMinuteOfDay) % 60)) + "\",\n";
              }
              
              jsonResult += "      \"segmentInVehicleTimeMinutes\":"  + std::to_string(segmentInVehicleTimeMinutes) + ",\n";
              jsonResult += "      \"segmentInVehicleTimeSeconds\":"  + std::to_string(segmentInVehicleTimeMinutes * 60) + "\n";
              
              jsonResult += "    },";
            }
            else if(enumMap[journeyStep->action] == "Walk")
            {
              jsonResult += "\n    {\n";
              
              if(!accessedFirstStop) // first walking
              {
                accessWalkingTimeMinutes = journeyStep->accessTimeMinutes;
                jsonResult += "      \"action\": \"" + accessMode + "\",\n";
              }
              
              if(journeyStep->accessFromStopId != -1)
              {
                stopStart = stopsById[journeyStep->accessFromStopId];
                
                if(journeyStep->readyToBoardMinuteOfDay != -1)
                {
                  transferWalkingTimeMinutes += journeyStep->accessTimeMinutes;
                  jsonResult += "      \"action\": \"walking\",\n";
                }
                else
                {
                  egressWalkingTimeMinutes = journeyStep->accessTimeMinutes;
                  jsonResult += "      \"action\": \"" + params.egressMode + "\",\n";
                }
              }
              
              totalWalkingTimeMinutes += journeyStep->accessTimeMinutes;
              
              lastReadyToBoardAtMinuteOfDay = journeyStep->readyToBoardMinuteOfDay;
              
              jsonResult += "      \"travelTimeMinutes\": " + std::to_string(journeyStep->accessTimeMinutes) + ",\n";
              jsonResult += "      \"travelTimeSeconds\": " + std::to_string(journeyStep->accessTimeMinutes * 60) + "\n";
    
              jsonResult += "    },";
    
            }
            
          }
        }
        jsonResult.pop_back(); // remove trailing comma
        jsonResult += "\n  ],\n";
        
        if (params.forwardCalculation)
        {
          jsonResult += "  \"departureTime\": \"" + boost::str(padWithZeros % (startTime / 60))      + ":" + boost::str(padWithZeros % (startTime % 60))      + "\",\n";
          jsonResult += "  \"arrivalTime\": \""   + boost::str(padWithZeros % (minArrivalTime / 60)) + ":" + boost::str(padWithZeros % (minArrivalTime % 60)) + "\",\n";
        }
        else
        {
          jsonResult += "  \"arrivalTime\": \"" + boost::str(padWithZeros % ((maxTimeValue - startTime) / 60))      + ":" + boost::str(padWithZeros % ((maxTimeValue - startTime) % 60))      + "\",\n";
          jsonResult += "  \"departureTime\": \""   + boost::str(padWithZeros % ((maxTimeValue - minArrivalTime) / 60)) + ":" + boost::str(padWithZeros % ((maxTimeValue - minArrivalTime) % 60)) + "\",\n";
        }
        jsonResult += "  \"totalWalkingTimeMinutesIfWalkingOnly\": "        + std::to_string(totalOnlyWalkingTimeMinutes) + ",\n";
        jsonResult += "  \"totalTravelTimeMinutes\": "                      + std::to_string(minArrivalTime - startTime) + ",\n";
        jsonResult += "  \"totalTravelTimeSeconds\": "                      + std::to_string((minArrivalTime - startTime) * 60) + ",\n";
        jsonResult += "  \"totalInVehicleTimeMinutes\": "                   + std::to_string(totalInVehicleTimeMinutes) + ",\n";
        jsonResult += "  \"totalInVehicleTimeSeconds\": "                   + std::to_string(totalInVehicleTimeMinutes * 60) + ",\n";
        jsonResult += "  \"totalNonTransitTravelTimeMinutes\": "            + std::to_string(totalWalkingTimeMinutes) + ",\n";
        jsonResult += "  \"totalNonTransitTravelTimeSeconds\": "            + std::to_string(totalWalkingTimeMinutes * 60) + ",\n";
        jsonResult += "  \"numberOfBoardings\": "                           + std::to_string(numberOfBoardings) + ",\n";
        jsonResult += "  \"numberOfTransfers\": "                           + std::to_string(numberOfTransfers) + ",\n";
        jsonResult += "  \"maxNumberOfTransfers\": "                        + std::to_string(params.maxNumberOfTransfers) + ",\n";
        jsonResult += "  \"transferWalkingTimeMinutes\": "                  + std::to_string(transferWalkingTimeMinutes) + ",\n";
        jsonResult += "  \"transferWalkingTimeSeconds\": "                  + std::to_string(transferWalkingTimeMinutes * 60) + ",\n";
        jsonResult += "  \"accessTravelTimeMinutes\": "                     + std::to_string(accessWalkingTimeMinutes) + ",\n";
        jsonResult += "  \"accessTravelTimeSeconds\": "                     + std::to_string(accessWalkingTimeMinutes * 60) + ",\n";
        jsonResult += "  \"egressTravelTimeMinutes\": "                     + std::to_string(egressWalkingTimeMinutes) + ",\n";
        jsonResult += "  \"egressTravelTimeSeconds\": "                     + std::to_string(egressWalkingTimeMinutes * 60) + ",\n";
        jsonResult += "  \"transferWaitingTimeMinutes\": "                  + std::to_string(transferWaitingTimeMinutes) + ",\n";
        jsonResult += "  \"transferWaitingTimeSeconds\": "                  + std::to_string(transferWaitingTimeMinutes * 60) + ",\n";
        jsonResult += "  \"firstWaitingTimeMinutes\": "                     + std::to_string(firstWaitingTimeMinutes) + ",\n";
        jsonResult += "  \"firstWaitingTimeSeconds\": "                     + std::to_string(firstWaitingTimeMinutes * 60) + ",\n";
        jsonResult += "  \"totalWaitingTimeMinutes\": "                     + std::to_string(totalWaitingTimeMinutes) + ",\n";
        jsonResult += "  \"totalWaitingTimeSeconds\": "                     + std::to_string(totalWaitingTimeMinutes * 60) + ",\n";
        jsonResult += "  \"minimumWaitingTimeBeforeEachBoardingMinutes\": " + std::to_string(params.minWaitingTimeMinutes) + ",\n";
        jsonResult += "  \"minimumWaitingTimeBeforeEachBoardingSeconds\": " + std::to_string(params.minWaitingTimeMinutes * 60) + ",\n";
        
        if (firstBoardingMinuteOfDay != -1 && accessWalkingTimeMinutes >= 0 && params.forwardCalculation)
        {
          int minStartTimeMinuteOfDay = (firstBoardingMinuteOfDay - accessWalkingTimeMinutes - params.minWaitingTimeMinutes);
          jsonResult += "  \"departureTimeToMinimizeFirstWaitingTime\": \"" + boost::str(padWithZeros % (minStartTimeMinuteOfDay / 60)) + ":" + boost::str(padWithZeros % (minStartTimeMinuteOfDay % 60)) + "\",\n";
          jsonResult += "  \"minimizedTotalTravelTimeMinutes\": " + std::to_string((minArrivalTime - (firstBoardingMinuteOfDay - accessWalkingTimeMinutes - params.minWaitingTimeMinutes))) + ",\n";
          jsonResult += "  \"minimizedTotalTravelTimeSeconds\": " + std::to_string(((minArrivalTime - (firstBoardingMinuteOfDay - accessWalkingTimeMinutes - params.minWaitingTimeMinutes))) * 60 ) + ",\n";
          //else
          //{
          //  jsonResult += "  \"arrivalTimeToMinimizeTravelTime\": \"" + boost::str(padWithZeros % ((maxTimeValue - minStartTimeMinuteOfDay) / 60)) + ":" + boost::str(padWithZeros % ((maxTimeValue - minStartTimeMinuteOfDay) % 60)) + "\",\n";
          //  jsonResult += "  \"minimizedTotalTravelTimeMinutes\": " + std::to_string((minArrivalTime - (firstBoardingMinuteOfDay - firstWalkingDuration - params.minWaitingTimeMinutes))) + ",\n";
          //  jsonResult += "  \"minimizedTotalTravelTimeSeconds\": " + std::to_string(((minArrivalTime - (firstBoardingMinuteOfDay - firstWalkingDuration - params.minWaitingTimeMinutes))) * 60 ) + ",\n";
          //}
        }
        
      }
      
      if (numberOfTransfers == -1) // set number of transfers to 0 if failed or only using walking
      {
        numberOfTransfers = 0;
      }
      
      if (atLeastOneRoutingFound)
      {
        jsonResult += "  \"status\": \"success\",\n";
      }
      else
      {
        jsonResult += "  \"status\": \"no_routing_found\",\n";
        jsonResult += "  \"totalWalkingTimeMinutesIfWalkingOnly\": " + std::to_string(totalOnlyWalkingTimeMinutes) + ",\n";
      }
      
      if (params.calculateByNumberOfTransfers) // calculate by number of transfers and return result for each maximum number of transfers down to 0
      {
        
        //std::cerr << "calculating by number of transfers" << std::endl;
        params.calculateByNumberOfTransfers = false; // make sure we do not enter a nested loop
        
        if (foundResult)
        {
          
          std::cerr << "num transfers = " << numberOfTransfers << std::endl;
          jsonResult += "  \"calculatedInMilliseconds\": " + std::to_string(algorithmCalculationTime.getDurationMillisecondsNoStop()) + "\n";
          
          //int currentTotalCalculationTime = algorithmCalculationTime.getDurationMillisecondsNoStop();
          
          jsonResultWithNumberOfTransfers += "{\n \"" + std::to_string(numberOfTransfers) + "\": \n" + jsonResult + "\n}";
          
          int minRoutingNumberOfTransfers {numberOfTransfers};
          int fastestRoutingNumberOfTransfers {numberOfTransfers};
          //int numTransfersToMinimizeTravelTimeWithTransferPenalty {numberOfTransfers}; // not yet implemented. We need to find a way to return travel time value alongside the json result!
          
          if (numberOfTransfers > 0)
          {
            for (int maxNumberOfTransfersI = numberOfTransfers - 1; maxNumberOfTransfersI >= 0; maxNumberOfTransfersI--)
            {
              std::string newJsonResult;
              params.maxNumberOfTransfers = maxNumberOfTransfersI;
              std::cerr << "num transfers = " << params.maxNumberOfTransfers << std::endl;
              refresh();
              newJsonResult = calculate(tripIdentifier, nearestStopsIdsFromStartingPoint, nearestStopsIdsFromEndingPoint);
              //algorithmCalculationTime.stop();
              std::string noRouteFoundString = "no_routing_found";
              std::size_t found = newJsonResult.find(noRouteFoundString);
              jsonResultWithNumberOfTransfers +=   ",\"" + std::to_string(maxNumberOfTransfersI) + "\": \n " + newJsonResult;
              
              //currentTotalCalculationTime = algorithmCalculationTime.getDurationMillisecondsNoStop();
              if (found != std::string::npos) // if not found (routing failed)
              {
                break;
              }
              else
              {
                minRoutingNumberOfTransfers = maxNumberOfTransfersI;
              }
            }
            
          }
          
          //std::cerr << "-- calculation time -- " << algorithmCalculationTime.getDurationMillisecondsNoStop() << " ms\n";
          
          jsonResultWithNumberOfTransfers += ",\n  \"status\": \"success\",\n";
          jsonResultWithNumberOfTransfers += "  \"totalWalkingTimeMinutesIfWalkingOnly\": " + std::to_string(totalOnlyWalkingTimeMinutes)                              + ",\n";
          jsonResultWithNumberOfTransfers += "  \"minimumNumberOfTransfers\": "             + std::to_string(minRoutingNumberOfTransfers)                              + ",\n";
          jsonResultWithNumberOfTransfers += "  \"fastestRoutingNumberOfTransfers\": "      + std::to_string(fastestRoutingNumberOfTransfers)                          + ",\n";
          jsonResultWithNumberOfTransfers += "  \"calculatedInMilliseconds\": "             + std::to_string(algorithmCalculationTime.getDurationMillisecondsNoStop()) + "\n";
          jsonResultWithNumberOfTransfers += "}";
          algorithmCalculationTime.stop();
          
          return jsonResultWithNumberOfTransfers;
        }
      }
      
      algorithmCalculationTime.stop();
      std::cerr << "-- calculation time -- " << algorithmCalculationTime.getDurationMillisecondsNoStop() << " ms\n";
      jsonResult += "  \"calculatedInMilliseconds\": " + std::to_string(algorithmCalculationTime.getDurationMillisecondsNoStop()) + "\n";
      
      jsonResult += "}";
    
    }
    
    return jsonResult;
    
  }
  
  
}
