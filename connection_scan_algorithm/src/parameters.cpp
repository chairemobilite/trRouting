#include "parameters.hpp"

namespace TrRouting
{
  
  bool Parameters::isCompleteForCalculation()
  {
    if (debugDisplay)
    {
      std::cout << "parameters: "            << std::endl;
      std::cout << " hasScenarioUuid: "      << (scenarioUuid.is_initialized()   ? "true" : "false") << std::endl;
      std::cout << " hasServices: "          << (onlyServicesIdx.size() >  0     ? "true" : "false") << std::endl;
      std::cout << " hasDataSourceUuid: "    << (dataSourceUuid.is_initialized() ? "true" : "false") << std::endl;
      std::cout << " hasOdTripUuid: "        << (odTripUuid.is_initialized()     ? "true" : "false") << std::endl;
      std::cout << " calculateAllOdTrips: "  << calculateAllOdTrips  << std::endl;
      std::cout << " calculateProfiles: "    << calculateProfiles    << std::endl;
      std::cout << " hasOrigin: "            << hasOrigin            << std::endl;
      std::cout << " hasDestination: "       << hasDestination       << std::endl;
      std::cout << " forwardCalculation: "   << forwardCalculation   << std::endl;
      std::cout << " departureTimeSeconds: " << departureTimeSeconds << std::endl;
      std::cout << " arrivalTimeSeconds: "   << arrivalTimeSeconds   << std::endl;
      std::cout << " returnAllNodesResult: " << returnAllNodesResult << std::endl;
    }
    if (!scenarioUuid.is_initialized() || onlyServicesIdx.size() == 0) // scenario and only services is mandatory
    {
      return false;
    }
    if (calculateAllOdTrips || odTripUuid.is_initialized())
    {
      return true;
    }
    else if (hasOrigin && returnAllNodesResult && departureTimeSeconds >= 0 && forwardCalculation)
    {
      return true;
    }
    else if (hasDestination && returnAllNodesResult && arrivalTimeSeconds >= 0 && !forwardCalculation)
    {
      return true;
    }
    else if (hasOrigin && hasDestination && departureTimeSeconds >= 0 && forwardCalculation)
    {
      return true;
    }
    else if (hasOrigin && hasDestination && arrivalTimeSeconds >= 0 && !forwardCalculation)
    {
      return true;
    }
    return false;
  }

  void Parameters::setDefaultValues()
  {
    odTripsPeriods.clear();
    odTripsGenders.clear();
    odTripsAgeGroups.clear();
    odTripsOccupations.clear();
    odTripsActivities.clear();
    odTripsModes.clear();

    onlyDataSourceIdx = -1;
    onlyServicesIdx.clear();
    exceptServicesIdx.clear();
    onlyLinesIdx.clear();
    exceptLinesIdx.clear();
    onlyModesIdx.clear();
    exceptModesIdx.clear();
    onlyAgenciesIdx.clear();
    exceptAgenciesIdx.clear();
    onlyNodesIdx.clear();
    exceptNodesIdx.clear();

    calculationName                        = "trRouting";
    batchNumber                            = 1;
    batchesCount                           = 1;
    alternatives                           = false;
    responseFormat                         = "json";
    saveResultToFile                       = false;
    odTripsSampleRatio                     = 1.0;
    scenarioUuid                           = boost::none;
    dataSourceUuid                         = boost::none;
    odTripUuid                             = boost::none;
    startingNodeUuid                       = boost::none;
    endingNodeUuid                         = boost::none;
    origin                                 = Point();
    destination                            = Point();
    hasOrigin                              = false;
    hasDestination                         = false;
    routingDateYear                        = 0;
    routingDateMonth                       = 0;
    routingDateDay                         = 0;
    originNodeIdx                          = -1;
    destinationNodeIdx                     = -1;
    departureTimeSeconds                   = -1;
    arrivalTimeSeconds                     = -1;
    calculateAllOdTrips                    = false;
    walkingSpeedMetersPerSecond            = 5/3.6; // 5 km/h
    drivingSpeedMetersPerSecond            = 90/3.6; // 90 km/h
    cyclingSpeedMetersPerSecond            = 25/3.6; // 25 km/h
    maxTotalTravelTimeSeconds              = MAX_INT;
    maxNumberOfTransfers                   = -1; // -1 means no limit
    minWaitingTimeSeconds                  = 5*60;
    maxAccessWalkingTravelTimeSeconds      = 20*60;
    maxEgressWalkingTravelTimeSeconds      = 20*60;
    maxTransferWalkingTravelTimeSeconds    = 20*60; // depends of transfer data provided
    maxTotalWalkingTravelTimeSeconds       = 60*60; // not used right now
    maxOnlyWalkingAccessTravelTimeRatio    = 1.5; // prefer walking only if it is faster than transit and total only walking travel time <= maxAccessWalkingTravelTimeSeconds * this ratio
    transferPenaltySeconds                 = 0; // not used right now
    accessMode                             = "walking";
    egressMode                             = "walking";
    noResultSecondMode                     = "driving";
    tryNextModeIfRoutingFails              = false;
    noResultNextAccessTimeSecondsIncrement = 5*60;
    maxNoResultNextAccessTimeSeconds       = 40*60;
    returnAllNodesResult                   = false;
    forwardCalculation                     = true;
    detailedResults                        = false;
    transferOnlyAtSameStation              = false;
    transferBetweenSameLine                = true;
    calculateByNumberOfTransfers           = false;
    maxAlternatives                        = 100;
    debugDisplay                           = serverDebugDisplay;
    alternativesMaxTravelTimeRatio         = 1.5;
    minAlternativeMaxTravelTimeSeconds     = 30*60;
    alternativesMaxAddedTravelTimeSeconds  = 30*60;
    odTripsSampleSize                      = -1;
    calculateProfiles                      = true;
    walkingSpeedFactor                     = 1.0; // all walking segments are weighted with this value. > 1.0 means faster walking, < 1.0 means slower walking
    seed                                   = std::chrono::system_clock::now().time_since_epoch().count();

  }

  void Parameters::update(std::vector<std::string> &parameters, std::map<boost::uuids::uuid, int> &scenarioIndexesByUuid, std::vector<std::unique_ptr<Scenario>> &scenarios, std::map<boost::uuids::uuid, int> &nodeIndexesByUuid, std::map<boost::uuids::uuid, int> &dataSourceIndexesByUuid)
  {
    
    setDefaultValues();

    boost::uuids::string_generator uuidGenerator;

    Scenario *         scenario;
    scenarioUuid   = boost::none;
    dataSourceUuid = boost::none;
    boost::uuids::uuid originNodeUuid;
    boost::uuids::uuid destinationNodeUuid;
    
    int periodIndex {0};
    int periodStartAtSeconds;
    int periodEndAtSeconds;
    std::vector<std::string> parameterWithValueVector;
    std::vector<std::string> latitudeLongitudeVector;
    std::vector<std::string> dateVector;
    std::vector<std::string> timeVector;
    std::vector<std::string> onlyServiceUuidsVector;
    std::vector<std::string> exceptServiceUuidsVector;
    std::vector<std::string> onlyLineUuidsVector;
    std::vector<std::string> exceptLineUuidsVector;
    std::vector<std::string> onlyModeShortnamesVector;
    std::vector<std::string> exceptModeShortnamesVector;
    std::vector<std::string> onlyAgencyUuidsVector;
    std::vector<std::string> exceptAgencyUuidsVector;
    std::vector<std::string> onlyNodeUuidsVector;
    std::vector<std::string> exceptNodeUuidsVector;
    std::vector<std::string> accessNodeUuidsVector;
    std::vector<std::string> accessNodeTravelTimesSecondsVector;
    std::vector<std::string> egressNodeUuidsVector;
    std::vector<std::string> egressNodeTravelTimesSecondsVector;
    std::vector<std::string> odTripsPeriodsVector;
    std::vector<std::string> odTripsGendersVector;
    std::vector<std::string> odTripsAgeGroupsVector;
    std::vector<std::string> odTripsOccupationsVector;
    std::vector<std::string> odTripsActivitiesVector;
    std::vector<std::string> odTripsModesVector;

    for(auto & parameterWithValue : parameters)
    {
      boost::split(parameterWithValueVector, parameterWithValue, boost::is_any_of("="));

      std::cout << " setting parameter " << parameterWithValueVector[0] << " with value " << parameterWithValueVector[1] << std::endl;

      // origin and destination:
      if (parameterWithValueVector[0] == "origin")
      {
        boost::split(latitudeLongitudeVector, parameterWithValueVector[1], boost::is_any_of(","));
        origin    = Point(std::stof(latitudeLongitudeVector[0]), std::stof(latitudeLongitudeVector[1]));
        hasOrigin = true;
        continue;
      }
      else if (parameterWithValueVector[0] == "destination")
      {
        boost::split(latitudeLongitudeVector, parameterWithValueVector[1], boost::is_any_of(","));
        destination    = Point(std::stof(latitudeLongitudeVector[0]), std::stof(latitudeLongitudeVector[1]));
        hasDestination = true;
        continue;
      }
      else if (parameterWithValueVector[0] == "starting_node_uuid"
               || parameterWithValueVector[0] == "start_node_uuid"
               || parameterWithValueVector[0] == "origin_node_uuid"
              )
      {
        originNodeUuid = uuidGenerator(parameterWithValueVector[1]);
        if (nodeIndexesByUuid.count(originNodeUuid) == 1)
        {
          originNodeIdx = nodeIndexesByUuid[originNodeUuid];
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "ending_node_uuid"
               || parameterWithValueVector[0] == "end_node_uuid"
               || parameterWithValueVector[0] == "destination_node_uuid"
              )
      {
        destinationNodeUuid = uuidGenerator(parameterWithValueVector[1]);
        if (nodeIndexesByUuid.count(destinationNodeUuid) == 1)
        {
          destinationNodeIdx = nodeIndexesByUuid[destinationNodeUuid];
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "return_all_nodes_results"
               || parameterWithValueVector[0] == "return_all_nodes_result"
               || parameterWithValueVector[0] == "all_nodes"
              )
      {
        if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1") { returnAllNodesResult = true; }
      }

      // times and date:
      else if (parameterWithValueVector[0] == "time"
               || parameterWithValueVector[0] == "departure"
               || parameterWithValueVector[0] == "departure_time"
               || parameterWithValueVector[0] == "start_time"
              )
      {
        boost::split(timeVector, parameterWithValueVector[1], boost::is_any_of(":"));
        departureTimeSeconds = std::stoi(timeVector[0]) * 3600 + std::stoi(timeVector[1]) * 60;
        continue;
      }
      else if (parameterWithValueVector[0] == "arrival_time"
               || parameterWithValueVector[0] == "arrival"
               || parameterWithValueVector[0] == "end_time"
              )
      {
        boost::split(timeVector, parameterWithValueVector[1], boost::is_any_of(":"));
        arrivalTimeSeconds = std::stoi(timeVector[0]) * 3600 + std::stoi(timeVector[1]) * 60;
        continue;
      }
      else if (parameterWithValueVector[0] == "departure_seconds"
               || parameterWithValueVector[0] == "departure_time_seconds"
               || parameterWithValueVector[0] == "start_time_seconds"
              )
      {
        departureTimeSeconds = std::stoi(parameterWithValueVector[1]);
        if (departureTimeSeconds < 0)
        {
          departureTimeSeconds = -1;
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "arrival_seconds"
               || parameterWithValueVector[0] == "arrival_time_seconds"
               || parameterWithValueVector[0] == "end_time_seconds"
              )
      {
        arrivalTimeSeconds = std::stoi(parameterWithValueVector[1]);
        if (arrivalTimeSeconds < 0)
        {
          arrivalTimeSeconds = -1;
        }
        continue;
      }

      // scenario:
      else if (parameterWithValueVector[0] == "scenario_uuid")
      {
        scenarioUuid = uuidGenerator(parameterWithValueVector[1]);
        if (scenarioIndexesByUuid.count(*scenarioUuid) == 1)
        {
          scenario          = scenarios[scenarioIndexesByUuid[*scenarioUuid]].get();
          onlyServicesIdx   = scenario->servicesIdx;
          onlyLinesIdx      = scenario->onlyLinesIdx;
          onlyAgenciesIdx   = scenario->onlyAgenciesIdx;
          onlyNodesIdx      = scenario->onlyNodesIdx;
          onlyModesIdx      = scenario->onlyModesIdx;
          exceptLinesIdx    = scenario->exceptLinesIdx;
          exceptAgenciesIdx = scenario->exceptAgenciesIdx;
          exceptNodesIdx    = scenario->exceptNodesIdx;
          exceptModesIdx    = scenario->exceptModesIdx;
        }
        continue;
      }

      else if (parameterWithValueVector[0] == "data_source_uuid")
      {
        dataSourceUuid    = uuidGenerator(parameterWithValueVector[1]);
        onlyDataSourceIdx = dataSourceIndexesByUuid[*dataSourceUuid];
        continue;
      }

      // forward/reverse:
      else if (parameterWithValueVector[0] == "reverse")
      {
        if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1") { forwardCalculation = false; }
        continue;
      }

      // min and max parameters:
      else if (parameterWithValueVector[0] == "min_waiting_time" || parameterWithValueVector[0] == "min_waiting_time_minutes")
      {
        minWaitingTimeSeconds = std::stoi(parameterWithValueVector[1]) * 60;
        if (minWaitingTimeSeconds < 0)
        {
          minWaitingTimeSeconds = 0;
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "min_waiting_time_seconds")
      {
        minWaitingTimeSeconds = std::stoi(parameterWithValueVector[1]);
        if (minWaitingTimeSeconds < 0)
        {
          minWaitingTimeSeconds = 0;
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "max_travel_time" || parameterWithValueVector[0] == "max_travel_time_minutes")
      {
        maxTotalTravelTimeSeconds = std::stoi(parameterWithValueVector[1]) * 60;
        if (maxTotalTravelTimeSeconds == 0)
        {
          maxTotalTravelTimeSeconds = MAX_INT;
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "max_travel_time_seconds")
      {
        maxTotalTravelTimeSeconds = std::stoi(parameterWithValueVector[1]);
        if (maxTotalTravelTimeSeconds == 0)
        {
          maxTotalTravelTimeSeconds = MAX_INT;
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "max_access_travel_time" || parameterWithValueVector[0] == "max_access_travel_time_minutes")
      {
        maxAccessWalkingTravelTimeSeconds = std::stoi(parameterWithValueVector[1]) * 60;
        if (maxAccessWalkingTravelTimeSeconds == 0)
        {
          maxAccessWalkingTravelTimeSeconds = MAX_INT;
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "max_access_travel_time_seconds")
      {
        maxAccessWalkingTravelTimeSeconds = std::stoi(parameterWithValueVector[1]);
        if (maxAccessWalkingTravelTimeSeconds == 0)
        {
          maxAccessWalkingTravelTimeSeconds = MAX_INT;
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "max_egress_travel_time" || parameterWithValueVector[0] == "max_egress_travel_time_minutes")
      {
        maxEgressWalkingTravelTimeSeconds = std::stoi(parameterWithValueVector[1]) * 60;
        if (maxEgressWalkingTravelTimeSeconds == 0)
        {
          maxEgressWalkingTravelTimeSeconds = MAX_INT;
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "max_egress_travel_time_seconds")
      {
        maxEgressWalkingTravelTimeSeconds = std::stoi(parameterWithValueVector[1]);
        if (maxEgressWalkingTravelTimeSeconds == 0)
        {
          maxEgressWalkingTravelTimeSeconds = MAX_INT;
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "max_transfer_travel_time" || parameterWithValueVector[0] == "max_transfer_travel_time_minutes")
      {
        maxTransferWalkingTravelTimeSeconds = std::stoi(parameterWithValueVector[1]) * 60;
        if (maxTransferWalkingTravelTimeSeconds == 0)
        {
          maxTransferWalkingTravelTimeSeconds = MAX_INT;
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "max_transfer_travel_time_seconds")
      {
        maxTransferWalkingTravelTimeSeconds = std::stoi(parameterWithValueVector[1]);
        if (maxTransferWalkingTravelTimeSeconds == 0)
        {
          maxTransferWalkingTravelTimeSeconds = MAX_INT;
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "max_only_walking_access_travel_time_ratio")
      {
        maxOnlyWalkingAccessTravelTimeRatio = std::stof(parameterWithValueVector[1]);
        continue;
      }

      // od trips:
      else if (parameterWithValueVector[0] == "od_trip_uuid")
      {
        odTripUuid = uuidGenerator(parameterWithValueVector[1]);
        continue;
      }
      else if (parameterWithValueVector[0] == "od_trips")
      {
        if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1") { calculateAllOdTrips = true; }
        continue;
      }
      else if (parameterWithValueVector[0] == "od_trips_sample_size" || parameterWithValueVector[0] == "sample_size" || parameterWithValueVector[0] == "sample")
      {
        odTripsSampleSize = std::stoi(parameterWithValueVector[1]);
        continue;
      }
      else if (parameterWithValueVector[0] == "od_trips_sample_ratio" || parameterWithValueVector[0] == "sample_ratio") // example: 0.5 means only 50% of od trips will be calculated (od trips will be shuffled before)
      {
        odTripsSampleRatio = std::stof(parameterWithValueVector[1]);
        continue;
      }
      else if (parameterWithValueVector[0] == "od_trips_periods")
      {
        boost::split(odTripsPeriodsVector, parameterWithValueVector[1], boost::is_any_of(","));
        for(std::string odTripsTime : odTripsPeriodsVector)
        {
          if (periodIndex % 2 == 0)
          {
            periodStartAtSeconds = std::stoi(odTripsTime);
          }
          else
          {
            periodEndAtSeconds = std::stoi(odTripsTime);
            odTripsPeriods.push_back(std::make_pair(periodStartAtSeconds, periodEndAtSeconds));
          }
          periodIndex++;
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "od_trips_genders")
      {
        boost::split(odTripsGendersVector, parameterWithValueVector[1], boost::is_any_of(","));
        for(std::string odTripsGender : odTripsGendersVector)
        {
          odTripsGenders.push_back(odTripsGender);
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "od_trips_age_groups")
      {
        boost::split(odTripsAgeGroupsVector, parameterWithValueVector[1], boost::is_any_of(","));
        for(std::string odTripsAgeGroup : odTripsAgeGroupsVector)
        {
          odTripsAgeGroups.push_back(odTripsAgeGroup);
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "od_trips_occupations")
      {
        boost::split(odTripsOccupationsVector, parameterWithValueVector[1], boost::is_any_of(","));
        for(std::string odTripsOccupation : odTripsOccupationsVector)
        {
          odTripsOccupations.push_back(odTripsOccupation);
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "od_trips_activities")
      {
        boost::split(odTripsActivitiesVector, parameterWithValueVector[1], boost::is_any_of(","));
        for(std::string odTripsActivity : odTripsActivitiesVector)
        {
          odTripsActivities.push_back(odTripsActivity);
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "od_trips_modes")
      {
        boost::split(odTripsModesVector, parameterWithValueVector[1], boost::is_any_of(","));
        for(std::string odTripsMode : odTripsModesVector)
        {
          odTripsModes.push_back(odTripsMode);
        }
        continue;
      }

      // alternatives:
      else if (parameterWithValueVector[0] == "alternatives" || parameterWithValueVector[0] == "alt")
      {
        if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1") { alternatives = true; }
        continue;
      }
       else if (parameterWithValueVector[0] == "max_alternatives" || parameterWithValueVector[0] == "max_alt")
      {
        maxAlternatives = std::stoi(parameterWithValueVector[1]);
        continue;
      }
      else if (parameterWithValueVector[0] == "alternatives_max_added_travel_time_minutes" || parameterWithValueVector[0] == "alt_max_added_travel_time")
      {
        alternativesMaxAddedTravelTimeSeconds = std::stoi(parameterWithValueVector[1]) * 60;
        continue;
      }
      else if (parameterWithValueVector[0] == "alternatives_max_added_travel_time_seconds" || parameterWithValueVector[0] == "alt_max_added_travel_time_seconds")
      {
        alternativesMaxAddedTravelTimeSeconds = std::stoi(parameterWithValueVector[1]);
        continue;
      }
      else if (parameterWithValueVector[0] == "alternatives_max_travel_time_ratio" || parameterWithValueVector[0] == "alt_max_ratio")
      {
        alternativesMaxTravelTimeRatio = std::stof(parameterWithValueVector[1]);
        continue;
      }
      else if (parameterWithValueVector[0] == "alternatives_min_max_travel_time_minutes" || parameterWithValueVector[0] == "alt_min_max_travel_time")
      {
        minAlternativeMaxTravelTimeSeconds = std::stoi(parameterWithValueVector[1]) * 60;
        continue;
      }
      else if (parameterWithValueVector[0] == "alternatives_min_max_travel_time_seconds" || parameterWithValueVector[0] == "alt_min_max_travel_time_seconds")
      {
        minAlternativeMaxTravelTimeSeconds = std::stoi(parameterWithValueVector[1]);
        continue;
      }
      
      // other:
      else if (parameterWithValueVector[0] == "transfer_penalty" || parameterWithValueVector[0] == "transfer_penalty_minutes")
      {
        transferPenaltySeconds = std::stoi(parameterWithValueVector[1]) * 60;
        continue;
      }
      else if (parameterWithValueVector[0] == "walking_speed_factor" || parameterWithValueVector[0] == "walk_factor") // > 1.0 means faster walking
      {
        walkingSpeedFactor = std::stof(parameterWithValueVector[1]);
        continue;
      }
      else if (parameterWithValueVector[0] == "transfer_only_at_same_station"
               || parameterWithValueVector[0] == "transfer_only_at_station"
              )
      {
        if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1") { transferOnlyAtSameStation = true; }
        continue;
      }
      else if (parameterWithValueVector[0] == "detailed"
               || parameterWithValueVector[0] == "detailed_results"
               || parameterWithValueVector[0] == "detailed_result"
              )
      {
        if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1") { detailedResults = true; }
        continue;
      }
      else if (parameterWithValueVector[0] == "profiles"
               || parameterWithValueVector[0] == "calculate_profiles"
              )
      {
        if (parameterWithValueVector[1] == "false" || parameterWithValueVector[1] == "0") { calculateProfiles = false; }
        continue;
      }
      else if (parameterWithValueVector[0] == "transfer_between_same_line"
               || parameterWithValueVector[0] == "allow_same_line_transfer"
               || parameterWithValueVector[0] == "transfers_between_same_line"
               || parameterWithValueVector[0] == "allow_same_line_transfers"
              )
      {
        if (parameterWithValueVector[1] == "false" || parameterWithValueVector[1] == "0") { transferBetweenSameLine = false; }
        continue;
      }
      else if (parameterWithValueVector[0] == "seed" || parameterWithValueVector[0] == "random_seed")
      {
        seed = std::stoi(parameterWithValueVector[1]);
        continue;
      }

      // not yet implemented
      else if (parameterWithValueVector[0] == "max_number_of_transfers" || parameterWithValueVector[0] == "max_transfers")
      {
        maxNumberOfTransfers = std::stoi(parameterWithValueVector[1]);
        continue;
      }
      // not yet implemented
      else if (parameterWithValueVector[0] == "calculate_by_number_of_transfers" || parameterWithValueVector[0] == "by_num_transfers")
      {
        if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1")
        { 
          calculateByNumberOfTransfers = true;
        }
        continue;
      }

      /*
      // not sure we want to keep this: or supply node indexes instead, to limit request size?
      else if (parameterWithValueVector[0] == "access_node_uuids")
      {
        boost::split(accessNodeUuidsVector, parameterWithValueVector[1], boost::is_any_of(","));
        boost::uuids::uuid accessNodeUuid;
        for(std::string accessNodeUuidStr : accessNodeUuidsVector)
        {
          accessNodeUuid = uuidGenerator(accessNodeUuidStr);
          if (calculator.nodeIndexesByUuid.count(accessNodeUuid) == 1)
          {
            accessNodesIdx.push_back(calculator.nodeIndexesByUuid[accessNodeUuid]);
          }
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "transfer_node_uuids")
      {
        boost::split(egressNodeUuidsVector, parameterWithValueVector[1], boost::is_any_of(","));
        boost::uuids::uuid egressNodeUuid;
        for(std::string egressNodeUuidStr : egressNodeUuidsVector)
        {
          egressNodeUuid = uuidGenerator(egressNodeUuidStr);
          if (calculator.nodeIndexesByUuid.count(egressNodeUuid) == 1)
          {
            calculator.params.egressNodesIdx.push_back(calculator.nodeIndexesByUuid[egressNodeUuid]);
          }
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "access_node_travel_times_seconds" || parameterWithValueVector[0] == "access_node_travel_times")
      {
        boost::split(accessNodeTravelTimesSecondsVector, parameterWithValueVector[1], boost::is_any_of(","));
        for(std::string accessNodeTravelTimeSeconds : accessNodeTravelTimesSecondsVector)
        {
          accessNodeTravelTimesSeconds.push_back(std::stoi(accessNodeTravelTimeSeconds));
        }
        calculator.params.accessNodeTravelTimesSeconds = accessNodeTravelTimesSeconds;
        continue;
      }
      else if (parameterWithValueVector[0] == "egress_node_travel_times_seconds" || parameterWithValueVector[0] == "egress_node_travel_times")
      {
        boost::split(egressNodeTravelTimesSecondsVector, parameterWithValueVector[1], boost::is_any_of(","));
        for(std::string egressNodeTravelTimeSeconds : egressNodeTravelTimesSecondsVector)
        {
          egressNodeTravelTimesSeconds.push_back(std::stoi(egressNodeTravelTimeSeconds));
        }
        calculator.params.egressNodeTravelTimesSeconds = egressNodeTravelTimesSeconds;
        continue;
      }
      */
      
      /*
      // we should use the scenario instead to simplify request:
      else if (parameterWithValueVector[0] == "only_service_uuids")
      {
        boost::split(onlyServiceUuidsVector, parameterWithValueVector[1], boost::is_any_of(","));
        boost::uuids::uuid onlyServiceUuid;
        for(std::string onlyServiceUuidStr : onlyServiceUuidsVector)
        {
          onlyServiceUuid = uuidGenerator(onlyServiceUuidStr);
          if (calculator.serviceIndexesByUuid.count(onlyServiceUuid) == 1)
          {
            calculator.params.onlyServicesIdx.push_back(calculator.serviceIndexesByUuid[onlyServiceUuid]);
          }
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "except_service_uuids")
      {
        boost::split(exceptServiceUuidsVector, parameterWithValueVector[1], boost::is_any_of(","));
        boost::uuids::uuid exceptServiceUuid;
        for(std::string exceptServiceUuidStr : exceptServiceUuidsVector)
        {
          exceptServiceUuid = uuidGenerator(exceptServiceUuidStr);
          if (calculator.serviceIndexesByUuid.count(exceptServiceUuid) == 1)
          {
            calculator.params.exceptServicesIdx.push_back(calculator.serviceIndexesByUuid[exceptServiceUuid]);
          }
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "only_node_uuids")
      {
        boost::split(onlyNodeUuidsVector, parameterWithValueVector[1], boost::is_any_of(","));
        boost::uuids::uuid onlyNodeUuid;
        for(std::string onlyNodeUuidStr : onlyNodeUuidsVector)
        {
          onlyNodeUuid = uuidGenerator(onlyNodeUuidStr);
          if (calculator.nodeIndexesByUuid.count(onlyNodeUuid) == 1)
          {
            calculator.params.onlyNodesIdx.push_back(calculator.nodeIndexesByUuid[onlyNodeUuid]);
          }
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "except_node_uuids")
      {
        boost::split(exceptNodeUuidsVector, parameterWithValueVector[1], boost::is_any_of(","));
        boost::uuids::uuid exceptNodeUuid;
        for(std::string exceptNodeUuidStr : exceptNodeUuidsVector)
        {
          exceptNodeUuid = uuidGenerator(exceptNodeUuidStr);
          if (calculator.nodeIndexesByUuid.count(exceptNodeUuid) == 1)
          {
            calculator.params.exceptNodesIdx.push_back(calculator.nodeIndexesByUuid[exceptNodeUuid]);
          }
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "only_line_uuids")
      {
        boost::split(onlyLineUuidsVector, parameterWithValueVector[1], boost::is_any_of(","));
        boost::uuids::uuid onlyLineUuid;
        for(std::string onlyLineUuidStr : onlyLineUuidsVector)
        {
          onlyLineUuid = uuidGenerator(onlyLineUuidStr);
          if (calculator.lineIndexesByUuid.count(onlyLineUuid) == 1)
          {
            calculator.params.onlyLinesIdx.push_back(calculator.lineIndexesByUuid[onlyLineUuid]);
          }
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "except_line_uuids")
      {
        boost::split(exceptLineUuidsVector, parameterWithValueVector[1], boost::is_any_of(","));
        boost::uuids::uuid exceptLineUuid;
        for(std::string exceptLineUuidStr : exceptLineUuidsVector)
        {
          exceptLineUuid = uuidGenerator(exceptLineUuidStr);
          if (calculator.lineIndexesByUuid.count(exceptLineUuid) == 1)
          {
            calculator.params.exceptLinesIdx.push_back(calculator.lineIndexesByUuid[exceptLineUuid]);
          }
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "only_modes")
      {
        boost::split(onlyModeShortnamesVector, parameterWithValueVector[1], boost::is_any_of(","));
        for(std::string onlyModeShortname : onlyModeShortnamesVector)
        {
          if (calculator.modeIndexesByShortname.count(onlyModeShortname) == 1)
          {
            calculator.params.onlyModesIdx.push_back(calculator.modeIndexesByShortname[onlyModeShortname]);
          }
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "except_modes")
      {
        boost::split(exceptModeShortnamesVector, parameterWithValueVector[1], boost::is_any_of(","));
        for(std::string exceptModeShortname : exceptModeShortnamesVector)
        {
          if (calculator.modeIndexesByShortname.count(exceptModeShortname) == 1)
          {
            calculator.params.exceptModesIdx.push_back(calculator.modeIndexesByShortname[exceptModeShortname]);
          }
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "only_agency_uuids")
      {
        boost::split(onlyAgencyUuidsVector, parameterWithValueVector[1], boost::is_any_of(","));
        boost::uuids::uuid onlyAgencyUuid;
        for(std::string onlyAgencyUuidStr : onlyAgencyUuidsVector)
        {
          onlyAgencyUuid = uuidGenerator(onlyAgencyUuidStr);
          if (calculator.agencyIndexesByUuid.count(onlyAgencyUuid) == 1)
          {
            calculator.params.onlyAgenciesIdx.push_back(calculator.agencyIndexesByUuid[onlyAgencyUuid]);
          }
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "except_agency_uuids")
      {
        boost::split(exceptAgencyUuidsVector, parameterWithValueVector[1], boost::is_any_of(","));
        boost::uuids::uuid exceptAgencyUuid;
        for(std::string exceptAgencyUuidStr : exceptAgencyUuidsVector)
        {
          exceptAgencyUuid = uuidGenerator(exceptAgencyUuidStr);
          if (calculator.agencyIndexesByUuid.count(exceptAgencyUuid) == 1)
          {
            calculator.params.onlyAgenciesIdx.push_back(calculator.agencyIndexesByUuid[exceptAgencyUuid]);
          }
        }
        continue;
      }
      */


      else if (parameterWithValueVector[0] == "date")
      {
        boost::split(dateVector, parameterWithValueVector[1], boost::is_any_of("/"));
        routingDateYear  = std::stoi(dateVector[0]);
        routingDateMonth = std::stoi(dateVector[1]);
        routingDateDay   = std::stoi(dateVector[2]);
        continue;
      }

      else if (parameterWithValueVector[0] == "calculation_name" || parameterWithValueVector[0] == "name")
      {
        calculationName = parameterWithValueVector[1];
        continue;
      }
      else if (parameterWithValueVector[0] == "file_format" || parameterWithValueVector[0] == "format" || parameterWithValueVector[0] == "response_format" || parameterWithValueVector[0] == "response")
      {
        responseFormat = parameterWithValueVector[1];
        continue;
      }
      else if (parameterWithValueVector[0] == "batch")
      {
        batchNumber = std::stoi(parameterWithValueVector[1]);
        continue;
      }
      else if (parameterWithValueVector[0] == "num_batches")
      {
        batchesCount = std::stoi(parameterWithValueVector[1]);
        continue;
      }
      else if (parameterWithValueVector[0] == "save_to_file" || parameterWithValueVector[0] == "save_file")
      {
        if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1") { saveResultToFile = true; }
        continue;
      }
      else if (parameterWithValueVector[0] == "debug")
      {
        if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1") { debugDisplay = true; }
        continue;
      }

    }

    if (departureTimeSeconds >= 0)
    {
      forwardCalculation = true;
    }
    else if (arrivalTimeSeconds >= 0 && departureTimeSeconds < 0)
    {
      forwardCalculation = false;
    }

  }


}