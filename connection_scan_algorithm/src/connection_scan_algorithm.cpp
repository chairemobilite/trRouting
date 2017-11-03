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
  
  // call setup only once when starting the calculator. Use updateParams before each calculation.
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
    connectionsByEndPathStopSequenceId   = getConnectionsByStartPathStopSequenceId(connectionsByArrivalTime); // really? by start? is this correct or we should replace by EndPathStopSequence?
    pathStopSequencesByStopId            = getPathStopSequencesByStopId(pathStopSequencesById);
  }
  
      
  
  // create a map of connection pointers by start path stop sequence id (key).
  std::map<unsigned long long, std::vector<Connection*> > ConnectionScanAlgorithm::getConnectionsByStartPathStopSequenceId(std::vector<Connection*> theConnectionsByDepartureTime)
  {
    
    std::cout << "Creating map of connections by starting path stop sequence id..." << std::endl;
    
    //algorithmCalculationTime.startStep();
    
    std::cout << std::fixed;
    std::cout << std::setprecision(2);
    
    std::map<unsigned long long, std::vector<Connection*> > connectionsByStartPathStopSequenceId;
    
    unsigned long long i = 0;
    
    for(auto & connection : theConnectionsByDepartureTime)
    {
      
      // add the path stop sequence id key does not exist:
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
    
    //algorithmCalculationTime.stopStep();
    //std::cout << "-- Creating map of connections by starting path stop sequence id -- " << algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
    
    return connectionsByStartPathStopSequenceId;
  }
  
  // create a map of connection pointers by end path stop sequence id (key).
  std::map<unsigned long long, std::vector<Connection*> > ConnectionScanAlgorithm::getConnectionsByEndPathStopSequenceId(std::vector<Connection*> theConnectionsByArrivalTime)
  {
    
    std::cout << "Creating map of connections by ending path stop sequence id..." << std::endl;
    
    //algorithmCalculationTime.startStep();
    
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
    
    //algorithmCalculationTime.stopStep();
    //std::cout << "-- Creating map of connections by ending path stop sequence id -- " << algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
    
    return connectionsByEndPathStopSequenceId;
  }
  
  std::map<unsigned long long, std::vector<unsigned long long> > ConnectionScanAlgorithm::getPathStopSequencesByStopId(std::map<unsigned long long,PathStopSequence> thePathStopSequencesById)
  {
    
    std::cout << "Creating map of path stop sequences by stop id..." << std::endl;
    
    //algorithmCalculationTime.startStep();
    
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
    
    //algorithmCalculationTime.stopStep();
    //std::cout << "-- Creating map of path stop sequences by stop id -- " << algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
    
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
    
    //algorithmCalculationTime.startStep();
    
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
    
    //algorithmCalculationTime.stopStep();
    //std::cout << "-- Resetting connections -- " << algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
    
  }
  
  std::string ConnectionScanAlgorithm::calculate(std::string tripIdentifier, const std::map<unsigned long long, int>& cachedNearestStopsIdsFromStartingPoint, const std::map<unsigned long long, int>& cachedNearestStopsIdsFromEndingPoint)
  {
    
    //algorithmCalculationTime.start();
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
        
      }
    }
    
    std::cerr << "-- calculation time -- " << algorithmCalculationTime.getDurationMillisecondsNoStop() << " ms\n";
    
    return "";
    
  }
  
  
}
