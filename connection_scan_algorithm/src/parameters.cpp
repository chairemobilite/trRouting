#include <chrono>
#include <boost/uuid/string_generator.hpp>
#include <boost/algorithm/string.hpp>
#include "spdlog/spdlog.h"

#include "parameters.hpp"
#include "node.hpp"
#include "od_trip.hpp"

namespace TrRouting
{

  void Parameters::setDefaultValues()
  {
    odTripsPeriods.clear();
    odTripsGenders.clear();
    odTripsAgeGroups.clear();
    odTripsOccupations.clear();
    odTripsActivities.clear();
    odTripsModes.clear();

    accessNodesIdx.clear();
    accessNodeTravelTimesSeconds.clear();
    accessNodeDistancesMeters.clear();
    egressNodesIdx.clear();
    egressNodeTravelTimesSeconds.clear();
    egressNodeDistancesMeters.clear();

    onlyDataSourceIdx = -1;

    calculationName                        = "trRouting";
    batchNumber                            = 1;
    batchesCount                           = 1;
    saveResultToFile                       = false;
    odTripsSampleRatio                     = 1.0;
    dataSourceUuid.reset();
    odTripUuid.reset();
    startingNodeUuid.reset();
    endingNodeUuid.reset();
    routingDateYear                        = 0;
    routingDateMonth                       = 0;
    routingDateDay                         = 0;
    originNodeIdx                          = -1;
    destinationNodeIdx                     = -1;
    calculateAllOdTrips                    = false;
    walkingSpeedMetersPerSecond            = 5/3.6; // 5 km/h
    drivingSpeedMetersPerSecond            = 90/3.6; // 90 km/h
    cyclingSpeedMetersPerSecond            = 25/3.6; // 25 km/h
    maxNumberOfTransfers                   = -1; // -1 means no limit
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
    maxAlternatives                        = 200;
    debugDisplay                           = serverDebugDisplay;
    alternativesMaxTravelTimeRatio         = 1.75;
    minAlternativeMaxTravelTimeSeconds     = 30*60;
    alternativesMaxAddedTravelTimeSeconds  = 60*60;
    maxValidAlternatives                   = 50;
    odTripsSampleSize                      = -1;
    calculateProfiles                      = true;
    walkingSpeedFactor                     = 1.0; // all walking segments are weighted with this value. > 1.0 means faster walking, < 1.0 means slower walking
    seed                                   = std::chrono::system_clock::now().time_since_epoch().count();

  }

  RouteParameters Parameters::update(std::vector<std::string> &parameters,
    std::map<boost::uuids::uuid, int> &scenarioIndexesByUuid,
    std::vector<std::unique_ptr<Scenario>> &scenarios,
    std::map<boost::uuids::uuid, int> &odTripIndexesByUuid,
    std::vector<std::unique_ptr<OdTrip>> &odTrips,
    std::map<boost::uuids::uuid, int> &nodeIndexesByUuid,
    std::vector<std::unique_ptr<Node>> &nodes,
    std::map<boost::uuids::uuid, int> &dataSourceIndexesByUuid)
  {

    setDefaultValues();

    boost::uuids::string_generator uuidGenerator;
    // Vector to contain parameters required for creating the RouteParameters object
    std::vector<std::pair<std::string, std::string>> newParametersWithValues;

    Scenario *         scenario;
    dataSourceUuid.reset();
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
    std::vector<std::string> accessNodeDistancesMetersVector;
    std::vector<std::string> egressNodeUuidsVector;
    std::vector<std::string> egressNodeTravelTimesSecondsVector;
    std::vector<std::string> egressNodeDistancesMetersVector;
    std::vector<std::string> odTripsPeriodsVector;
    std::vector<std::string> odTripsGendersVector;
    std::vector<std::string> odTripsAgeGroupsVector;
    std::vector<std::string> odTripsOccupationsVector;
    std::vector<std::string> odTripsActivitiesVector;
    std::vector<std::string> odTripsModesVector;

    for(auto & parameterWithValue : parameters)
    {
      boost::split(parameterWithValueVector, parameterWithValue, boost::is_any_of("="));

      spdlog::info(" setting parameter {} with value {}", parameterWithValueVector[0], parameterWithValueVector[1]);

      // origin and destination:
      if (parameterWithValueVector[0] == "origin")
      {
        boost::split(latitudeLongitudeVector, parameterWithValueVector[1], boost::is_any_of(","));
        if (latitudeLongitudeVector.size() == 2)
        {
          newParametersWithValues.push_back(std::make_pair("origin", latitudeLongitudeVector[1] + "," + latitudeLongitudeVector[0]));
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "destination")
      {
        boost::split(latitudeLongitudeVector, parameterWithValueVector[1], boost::is_any_of(","));
        if (latitudeLongitudeVector.size() == 2)
        {
          newParametersWithValues.push_back(std::make_pair("destination", latitudeLongitudeVector[1] + "," + latitudeLongitudeVector[0]));
        }
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
          Node *originNode = nodes[nodeIndexesByUuid[originNodeUuid]].get();
          newParametersWithValues.push_back(std::make_pair("origin", std::to_string(originNode->point.get()->longitude) + ',' + std::to_string(originNode->point.get()->latitude)));
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
          Node *destinationNode = nodes[nodeIndexesByUuid[destinationNodeUuid]].get();
          newParametersWithValues.push_back(std::make_pair("destination", std::to_string(destinationNode->point.get()->longitude) + ',' + std::to_string(destinationNode->point.get()->latitude)));
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "return_all_nodes_results"
               || parameterWithValueVector[0] == "return_all_nodes_result"
               || parameterWithValueVector[0] == "all_nodes"
              )
      {
        if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1")
        {
          returnAllNodesResult = true;
          // This parameter requires only origin or destination, but the routeParameters class requires both,so we fake one so the parameter creation will work
          // TODO Eventually move this parameter to its own endpoint
          newParametersWithValues.insert(newParametersWithValues.begin(), std::make_pair("origin", "0,0"));
          newParametersWithValues.insert(newParametersWithValues.begin(), std::make_pair("destination", "1,1"));
        }
      }

      // times and date:
      else if (parameterWithValueVector[0] == "time"
               || parameterWithValueVector[0] == "departure"
               || parameterWithValueVector[0] == "departure_time"
               || parameterWithValueVector[0] == "start_time"
              )
      {
        boost::split(timeVector, parameterWithValueVector[1], boost::is_any_of(":"));
        newParametersWithValues.push_back(std::make_pair("time_of_trip", std::to_string(std::stoi(timeVector[0]) * 3600 + std::stoi(timeVector[1]) * 60)));
        continue;
      }
      else if (parameterWithValueVector[0] == "arrival_time"
               || parameterWithValueVector[0] == "arrival"
               || parameterWithValueVector[0] == "end_time"
              )
      {
        boost::split(timeVector, parameterWithValueVector[1], boost::is_any_of(":"));
        newParametersWithValues.push_back(std::make_pair("time_of_trip", std::to_string(std::stoi(timeVector[0]) * 3600 + std::stoi(timeVector[1]) * 60)));
        newParametersWithValues.push_back(std::make_pair("time_type", "1"));
        continue;
      }
      else if (parameterWithValueVector[0] == "departure_seconds"
               || parameterWithValueVector[0] == "departure_time_seconds"
               || parameterWithValueVector[0] == "start_time_seconds"
              )
      {
        newParametersWithValues.push_back(std::make_pair("time_of_trip", parameterWithValueVector[1]));
        continue;
      }
      else if (parameterWithValueVector[0] == "arrival_seconds"
               || parameterWithValueVector[0] == "arrival_time_seconds"
               || parameterWithValueVector[0] == "end_time_seconds"
              )
      {
        newParametersWithValues.push_back(std::make_pair("time_of_trip", parameterWithValueVector[1]));
        newParametersWithValues.push_back(std::make_pair("time_type", "1"));
        continue;
      }

      // scenario:
      else if (parameterWithValueVector[0] == "scenario_uuid")
      {
        newParametersWithValues.push_back(std::make_pair("scenario_id", parameterWithValueVector[1]));
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
        newParametersWithValues.push_back(std::make_pair("min_waiting_time", std::to_string(std::stoi(parameterWithValueVector[1]) * 60)));
        continue;
      }
      else if (parameterWithValueVector[0] == "min_waiting_time_seconds")
      {
        newParametersWithValues.push_back(std::make_pair("min_waiting_time", parameterWithValueVector[1]));
        continue;
      }
      else if (parameterWithValueVector[0] == "max_travel_time" || parameterWithValueVector[0] == "max_travel_time_minutes")
      {
        newParametersWithValues.push_back(std::make_pair("max_travel_time", std::to_string(std::stoi(parameterWithValueVector[1]) * 60)));
        continue;
      }
      else if (parameterWithValueVector[0] == "max_travel_time_seconds")
      {
        newParametersWithValues.push_back(std::make_pair("max_travel_time", parameterWithValueVector[1]));
        continue;
      }
      else if (parameterWithValueVector[0] == "max_access_travel_time" || parameterWithValueVector[0] == "max_access_travel_time_minutes")
      {
        newParametersWithValues.push_back(std::make_pair("max_access_travel_time", std::to_string(std::stoi(parameterWithValueVector[1]) * 60)));
        continue;
      }
      else if (parameterWithValueVector[0] == "max_access_travel_time_seconds")
      {
        newParametersWithValues.push_back(std::make_pair("max_access_travel_time", parameterWithValueVector[1]));
        continue;
      }
      else if (parameterWithValueVector[0] == "max_egress_travel_time" || parameterWithValueVector[0] == "max_egress_travel_time_minutes")
      {
        newParametersWithValues.push_back(std::make_pair("max_egress_travel_time", std::to_string(std::stoi(parameterWithValueVector[1]) * 60)));
        continue;
      }
      else if (parameterWithValueVector[0] == "max_egress_travel_time_seconds")
      {
        newParametersWithValues.push_back(std::make_pair("max_egress_travel_time", parameterWithValueVector[1]));
        continue;
      }
      else if (parameterWithValueVector[0] == "max_transfer_travel_time" || parameterWithValueVector[0] == "max_transfer_travel_time_minutes")
      {
        newParametersWithValues.push_back(std::make_pair("max_transfer_travel_time", std::to_string(std::stoi(parameterWithValueVector[1]) * 60)));
        continue;
      }
      else if (parameterWithValueVector[0] == "max_transfer_travel_time_seconds")
      {
        newParametersWithValues.push_back(std::make_pair("max_transfer_travel_time", parameterWithValueVector[1]));
        continue;
      }
      else if (parameterWithValueVector[0] == "max_first_waiting_time_seconds")
      {
        newParametersWithValues.push_back(std::make_pair("max_first_waiting_time", parameterWithValueVector[1]));
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
        // TODO: Use a new endpoint for od trip uuid. Now we get its od and add them to parameters
        boost::uuids::uuid odTripUuid = uuidGenerator(parameterWithValueVector[1]);

        if (odTripIndexesByUuid.count(odTripUuid) == 1)
        {
          OdTrip *odTrip = odTrips[odTripIndexesByUuid[odTripUuid]].get();
          spdlog::info("od trip uuid {} dts {}", to_string(odTrip->uuid), odTrip->departureTimeSeconds);

          newParametersWithValues.push_back(std::make_pair("origin", std::to_string(odTrip->origin.get()->latitude) + ',' + std::to_string(odTrip->origin.get()->longitude)));
          newParametersWithValues.push_back(std::make_pair("destination", std::to_string(odTrip->destination.get()->latitude) + ',' + std::to_string(odTrip->destination.get()->longitude)));
          // TODO Add parameter for the departure_time? It was not in the original code path
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "od_trips")
      {
        if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1")
        {
          calculateAllOdTrips = true;
          // This parameter does not require origin/destination/departure_time parameters, so we fake one so the parameter creation will work
          // TODO Eventually move this parameter to its own endpoint
          newParametersWithValues.push_back(std::make_pair("origin", "0,0"));
          newParametersWithValues.push_back(std::make_pair("destination", "1,1"));
          newParametersWithValues.push_back(std::make_pair("time_of_trip", "0"));
        }
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
        newParametersWithValues.push_back(std::make_pair("alternatives", parameterWithValueVector[1]));
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
        // not used right now
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

      // not sure we want to keep this: or supply node indexes instead, to limit request size?
      else if (parameterWithValueVector[0] == "access_node_uuids")
      {
        boost::split(accessNodeUuidsVector, parameterWithValueVector[1], boost::is_any_of(","));
        boost::uuids::uuid accessNodeUuid;
        for(std::string accessNodeUuidStr : accessNodeUuidsVector)
        {
          accessNodeUuid = uuidGenerator(accessNodeUuidStr);
          if (nodeIndexesByUuid.count(accessNodeUuid) == 1)
          {
            accessNodesIdx.push_back(nodeIndexesByUuid[accessNodeUuid]);
          }
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "egress_node_uuids")
      {
        boost::split(egressNodeUuidsVector, parameterWithValueVector[1], boost::is_any_of(","));
        boost::uuids::uuid egressNodeUuid;
        for(std::string egressNodeUuidStr : egressNodeUuidsVector)
        {
          egressNodeUuid = uuidGenerator(egressNodeUuidStr);
          if (nodeIndexesByUuid.count(egressNodeUuid) == 1)
          {
            egressNodesIdx.push_back(nodeIndexesByUuid[egressNodeUuid]);
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
        accessNodeTravelTimesSeconds = accessNodeTravelTimesSeconds;
        continue;
      }
      else if (parameterWithValueVector[0] == "egress_node_travel_times_seconds" || parameterWithValueVector[0] == "egress_node_travel_times")
      {
        boost::split(egressNodeTravelTimesSecondsVector, parameterWithValueVector[1], boost::is_any_of(","));
        for(std::string egressNodeTravelTimeSeconds : egressNodeTravelTimesSecondsVector)
        {
          egressNodeTravelTimesSeconds.push_back(std::stoi(egressNodeTravelTimeSeconds));
        }
        egressNodeTravelTimesSeconds = egressNodeTravelTimesSeconds;
        continue;
      }

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

    return RouteParameters::createRouteODParameter(newParametersWithValues, scenarioIndexesByUuid, scenarios);

  }


}
