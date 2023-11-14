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

    onlyDataSource.reset();

    batchNumber                            = 1;
    batchesCount                           = 1;
    odTripsSampleRatio                     = 1.0;
    odTripUuid.reset();
    odTripsSampleSize                      = -1;
    calculateProfiles                      = true;
    seed                                   = std::chrono::system_clock::now().time_since_epoch().count();

  }

  RouteParameters Parameters::update(std::vector<std::string> &parameters,
    const std::map<boost::uuids::uuid, Scenario> &scenarios,
    const std::map<boost::uuids::uuid, OdTrip> &odTrips,
    const std::map<boost::uuids::uuid, Node> &nodes,
    const std::map<boost::uuids::uuid, DataSource> &dataSources)
  {

    setDefaultValues();

    boost::uuids::string_generator uuidGenerator;
    // Vector to contain parameters required for creating the RouteParameters object
    std::vector<std::pair<std::string, std::string>> newParametersWithValues;

    boost::uuids::uuid originNodeUuid;
    boost::uuids::uuid destinationNodeUuid;

    int periodIndex {0};
    int periodStartAtSeconds = 0;
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
        //TODO Replace map.count()/at() combos with map.find(). (Do this elsewhere)
        if (nodes.count(originNodeUuid) == 1)
        {
          const Node &originNode = nodes.at(originNodeUuid);
          newParametersWithValues.push_back(std::make_pair("origin", std::to_string(originNode.point.get()->longitude) + ',' + std::to_string(originNode.point.get()->latitude)));
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "ending_node_uuid"
               || parameterWithValueVector[0] == "end_node_uuid"
               || parameterWithValueVector[0] == "destination_node_uuid"
              )
      {
        destinationNodeUuid = uuidGenerator(parameterWithValueVector[1]);
        if (nodes.count(destinationNodeUuid) == 1)
        {
          const Node & destinationNode = nodes.at(destinationNodeUuid);
          newParametersWithValues.push_back(std::make_pair("destination", std::to_string(destinationNode.point.get()->longitude) + ',' + std::to_string(destinationNode.point.get()->latitude)));
        }
        continue;
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
        boost::uuids::uuid dataSourceUuid    = uuidGenerator(parameterWithValueVector[1]);
        // This will throw an exception if specified data_source_uuid is not valid
        onlyDataSource = dataSources.at(dataSourceUuid);
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

      // od trips:
      else if (parameterWithValueVector[0] == "od_trip_uuid")
      {
        // TODO: Use a new endpoint for od trip uuid. Now we get its od and add them to parameters
        // TODO maybe we should not use the global variable here
        odTripUuid = uuidGenerator(parameterWithValueVector[1]);

        if (odTrips.count(odTripUuid.value()) == 1)
        {
          const OdTrip & odTrip = odTrips.at(odTripUuid.value());
          spdlog::info("od trip uuid {} dts {}", to_string(odTrip.uuid), odTrip.departureTimeSeconds);

          newParametersWithValues.push_back(std::make_pair("origin", std::to_string(odTrip.origin.get()->latitude) + ',' + std::to_string(odTrip.origin.get()->longitude)));
          newParametersWithValues.push_back(std::make_pair("destination", std::to_string(odTrip.destination.get()->latitude) + ',' + std::to_string(odTrip.destination.get()->longitude)));
          // TODO Add parameter for the departure_time? It was not in the original code path
        }
        continue;
      }
      else if (parameterWithValueVector[0] == "od_trips")
      {
        if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1")
        {
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

      // other:
      else if (parameterWithValueVector[0] == "profiles"
               || parameterWithValueVector[0] == "calculate_profiles"
              )
      {
        if (parameterWithValueVector[1] == "false" || parameterWithValueVector[1] == "0") { calculateProfiles = false; }
        continue;
      }
      else if (parameterWithValueVector[0] == "seed" || parameterWithValueVector[0] == "random_seed")
      {
        seed = std::stoi(parameterWithValueVector[1]);
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
    }

    return RouteParameters::createRouteODParameter(newParametersWithValues, scenarios);

  }


}
