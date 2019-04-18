#ifndef TR_PARAMETERS
#define TR_PARAMETERS

#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <map>
#include <math.h>
#include <osrm/osrm.hpp>
#include <boost/optional.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/string_generator.hpp>

#include "point.hpp"
#include "toolbox.hpp"
#include "od_trip.hpp"

namespace TrRouting
{
  
  class DatabaseFetcher;
  class CacheFetcher;
  class GtfsFetcher;
  class CsvFetcher;

  struct Parameters {

    std::string projectShortname;
    std::string dataFetcherShortname; // cache, csv or gtfs, only cache is implemented for now
    std::string cacheDirectoryPath;
    std::string calculationName;
    std::string responseFormat;
    
    CacheFetcher* cacheFetcher;
    GtfsFetcher*  gtfsFetcher;
    CsvFetcher*   csvFetcher;
    
    int batchNumber;
    int batchesCount;
    int odTripsSampleSize;

    int routingDateYear;   // not implemented, use onlyServicesIdx or exceptServicesIdx for now
    int routingDateMonth;  // not implemented, use onlyServicesIdx or exceptServicesIdx for now
    int routingDateDay;    // not implemented, use onlyServicesIdx or exceptServicesIdx for now
    std::vector<int> onlyServicesIdx;
    std::vector<int> exceptServicesIdx;
    std::vector<int> onlyLinesIdx;
    std::vector<int> exceptLinesIdx;
    std::vector<int> onlyModesIdx;
    std::vector<int> exceptModesIdx;
    std::vector<int> onlyAgenciesIdx;
    std::vector<int> exceptAgenciesIdx;
    std::vector<int> onlyNodesIdx;
    std::vector<int> exceptNodesIdx;
    std::vector<int> accessNodesIdx;
    std::vector<int> accessNodeTravelTimesSeconds;
    std::vector<int> egressNodesIdx;
    std::vector<int> egressNodeTravelTimesSeconds;
    
    std::vector<std::pair<int,int>> odTripsPeriods; // pair: start_at_seconds, end_at_seconds
    std::vector<std::string>        odTripsGenders;
    std::vector<std::string>        odTripsAgeGroups;
    std::vector<std::string>        odTripsOccupations;
    std::vector<std::string>        odTripsActivities;
    std::vector<std::string>        odTripsModes;

    int departureTimeSeconds;
    int arrivalTimeSeconds;
    int maxTotalTravelTimeSeconds;
    int maxNumberOfTransfers;
    int minWaitingTimeSeconds;
    int transferPenaltySeconds;
    int maxAccessWalkingDistanceMeters;
    int maxAccessWalkingTravelTimeSeconds;
    int maxEgressWalkingTravelTimeSeconds;
    int maxTransferWalkingTravelTimeSeconds;
    int maxTotalWalkingTravelTimeSeconds;
    float maxOnlyWalkingAccessTravelTimeRatio;
    float walkingSpeedFactor;
    float walkingSpeedMetersPerSecond;
    float drivingSpeedMetersPerSecond;
    float cyclingSpeedMetersPerSecond;
    
    Point origin;
    Point destination;
    int originNodeIdx;
    int destinationNodeIdx;
    OdTrip* odTrip;
    bool calculateAllOdTrips;
    bool saveResultToFile;
    boost::optional<boost::uuids::uuid> scenarioUuid;
    boost::optional<boost::uuids::uuid> odTripUuid;
    boost::optional<boost::uuids::uuid> startingNodeUuid
    boost::optional<boost::uuids::uuid> endingNodeUuid;

    std::string osrmWalkingPort;
    std::string osrmCyclingPort;
    std::string osrmDrivingPort;
    std::string osrmWalkingHost;
    std::string osrmCyclingHost;
    std::string osrmDrivingHost;
    std::string osrmWalkingFilePath; // path to walking .osrm file
    std::string osrmCyclingFilePath; // path to cycling .osrm file
    std::string osrmDrivingFilePath; // path to driving .osrm file
    bool osrmWalkingUseLib;
    bool osrmCyclingUseLib;
    bool osrmDrivingUseLib;
    boost::optional<osrm::OSRM> osrmWalkingRouter;
    boost::optional<osrm::OSRM> osrmCyclingRouter;
    boost::optional<osrm::OSRM> osrmDrivingRouter;
    
    std::string accessMode;
    std::string egressMode;
    bool debugDisplay; // display performance and debug info when set to true
    bool tryNextModeIfRoutingFails;
    std::string noResultSecondMode;
    int noResultNextAccessTimeSecondsIncrement;
    int maxNoResultNextAccessTimeSeconds;
    int maxAlternatives; // number of alternatives to calculate before returning results (when alternatives parameter is set to true)
    float alternativesMaxTravelTimeRatio; // travel time of fastest route is multiplied by this ratio to find plausible alternative with a max travel time.
    float minAlternativeMaxTravelTimeSeconds; // if multiplying max travel time ratio with max travel time is too small, keep max travel time to this minimum.
    int   alternativesMaxAddedTravelTimeSeconds; // how many seconds to add to fastest travel time to limit alternatives travel time.
    
    bool returnAllNodesResult;         // keep results for all nodes (used in creating accessibility map)
    bool forwardCalculation;           // forward calculation: default. if false: reverse calculation, will ride connections backward (useful when setting the arrival time)
    bool detailedResults;              // return detailed results when using results for all nodes
    bool transferOnlyAtSameStation;    // will transfer only between nodes/stops having the same station_id (better performance, but make sure your stations are well designed and specified)
    bool transferBetweenSameLine;      // allow transfers between the same line
    bool calculateByNumberOfTransfers; // calculate first the fastest route, then calculate with decreasing number of transfers until no route is found, return results for each number of transfers.
    bool alternatives;                 // calculate alternatives or not

    void setDefaultValues()
    {

      odTripsPeriods.clear();
      odTripsGenders.clear();
      odTripsAgeGroups.clear();
      odTripsOccupations.clear();
      odTripsActivities.clear();
      odTripsModes.clear();

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

      cacheDirectoryPath                     = "cache/";
      calculationName                        = "trRouting";
      batchNumber                            = 1;
      batchesCount                           = 1;
      alternatives                           = false;
      responseFormat                         = "json";
      saveResultToFile                       = false;
      odTrip                                 = NULL;
      scenarioUuid                           = boost::none;
      odTripUuid                             = boost::none;
      startingNodeUuid                       = boost::none;
      endingNodeUuid                         = boost::none;

      origin                                 = Point();
      destination                            = Point();
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
      debugDisplay                           = false;
      alternativesMaxTravelTimeRatio         = 1.5;
      minAlternativeMaxTravelTimeSeconds     = 30*60;
      alternativesMaxAddedTravelTimeSeconds  = 30*60;
      odTripsSampleSize                      = -1;
      walkingSpeedFactor                     = 1.0; // all walking segments are weighted with this value. > 1.0 means faster walking, < 1.0 means slower walking
    }

    bool isCompleteForCalculation()
    {
      if (calculateAllOdTrips || odTripUuid.is_initialized())
      {
        return true;
      }
      else if (origin && returnAllNodesResult && departureTimeSeconds && forwardCalculation)
      {
        return true;
      }
      else if (destination && returnAllNodesResult && arrivalTimeSeconds && !forwardCalculation)
      {
        return true;
      }
      else if (origin && destination && departureTimeSeconds && forwardCalculation)
      {
        return true;
      }
      else if (origin && destination && arrivalTimeSeconds && !forwardCalculation)
      {
        return true;
      }
      return false;
    }

    void update(std::vector<std::string> &parameters, std::map<boost::uuids::uuid, int> &scenarioIndexesByUuid, std::vector<Scenario> &scenarios, std::map<boost::uuids::uuid, int> &nodeIndexesByUuid)
    {

      boost::uuids::string_generator uuidGenerator;

      Scenario           scenario;
      boost::uuids::uuid scenarioUuid;
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
        boost::split(parameterWithValueVector, parameters, boost::is_any_of("="));

        // origin and destination:
        if (parameterWithValueVector[0] == "origin")
        {
          boost::split(latitudeLongitudeVector, parameterWithValueVector[1], boost::is_any_of(","));
          origin = Point(std::stof(latitudeLongitudeVector[0]), std::stof(latitudeLongitudeVector[1]));
          break;
        }
        else if (parameterWithValueVector[0] == "destination")
        {
          boost::split(latitudeLongitudeVector, parameterWithValueVector[1], boost::is_any_of(","));
          destination = Point(std::stof(latitudeLongitudeVector[0]), std::stof(latitudeLongitudeVector[1]));
          break;
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
          break;
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
          break;
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
          break;
        }
        else if (parameterWithValueVector[0] == "arrival_time"
                 || parameterWithValueVector[0] == "arrival"
                 || parameterWithValueVector[0] == "end_time"
                )
        {
          boost::split(timeVector, parameterWithValueVector[1], boost::is_any_of(":"));
          arrivalTimeSeconds = std::stoi(timeVector[0]) * 3600 + std::stoi(timeVector[1]) * 60;
          break;
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
          break;
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
          break;
        }

        // scenario:
        else if (parameterWithValueVector[0] == "scenario_uuid")
        {
          scenarioUuid = uuidGenerator(parameterWithValueVector[1]);
          if (scenarioIndexesByUuid.count(scenarioUuid) == 1)
          {
            scenario          = scenarios[scenarioIndexesByUuid[scenarioUuid]];
            onlyServicesIdx   = scenario.servicesIdx;
            onlyLinesIdx      = scenario.onlyLinesIdx;
            onlyAgenciesIdx   = scenario.onlyAgenciesIdx;
            onlyNodesIdx      = scenario.onlyNodesIdx;
            onlyModesIdx      = scenario.onlyModesIdx;
            exceptLinesIdx    = scenario.exceptLinesIdx;
            exceptAgenciesIdx = scenario.exceptAgenciesIdx;
            exceptNodesIdx    = scenario.exceptNodesIdx;
            exceptModesIdx    = scenario.exceptModesIdx;
          }
          break;
        }

        // forward/reverse:
        else if (parameterWithValueVector[0] == "reverse")
        {
          if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1") { forwardCalculation = false; }
          break;
        }

        // min and max parameters:
        else if (parameterWithValueVector[0] == "min_waiting_time" || parameterWithValueVector[0] == "min_waiting_time_minutes")
        {
          minWaitingTimeSeconds = std::stoi(parameterWithValueVector[1]) * 60;
          if (minWaitingTimeSeconds < 0)
          {
            minWaitingTimeSeconds = 0;
          }
          break;
        }
        else if (parameterWithValueVector[0] == "min_waiting_time_seconds")
        {
          minWaitingTimeSeconds = std::stoi(parameterWithValueVector[1]);
          if (minWaitingTimeSeconds < 0)
          {
            minWaitingTimeSeconds = 0;
          }
          break;
        }
        else if (parameterWithValueVector[0] == "max_travel_time" || parameterWithValueVector[0] == "max_travel_time_minutes")
        {
          maxTotalTravelTimeSeconds = std::stoi(parameterWithValueVector[1]) * 60;
          if (maxTotalTravelTimeSeconds == 0)
          {
            maxTotalTravelTimeSeconds = MAX_INT;
          }
          break;
        }
        else if (parameterWithValueVector[0] == "max_travel_time_seconds")
        {
          maxTotalTravelTimeSeconds = std::stoi(parameterWithValueVector[1]);
          if (maxTotalTravelTimeSeconds == 0)
          {
            maxTotalTravelTimeSeconds = MAX_INT;
          }
          break;
        }
        else if (parameterWithValueVector[0] == "max_access_travel_time" || parameterWithValueVector[0] == "max_access_travel_time_minutes")
        {
          maxAccessWalkingTravelTimeSeconds = std::stoi(parameterWithValueVector[1]) * 60;
          if (maxAccessWalkingTravelTimeSeconds == 0)
          {
            maxAccessWalkingTravelTimeSeconds = MAX_INT;
          }
          break;
        }
        else if (parameterWithValueVector[0] == "max_access_travel_time_seconds")
        {
          maxAccessWalkingTravelTimeSeconds = std::stoi(parameterWithValueVector[1]);
          if (maxAccessWalkingTravelTimeSeconds == 0)
          {
            maxAccessWalkingTravelTimeSeconds = MAX_INT;
          }
          break;
        }
        else if (parameterWithValueVector[0] == "max_egress_travel_time" || parameterWithValueVector[0] == "max_egress_travel_time_minutes")
        {
          maxEgressWalkingTravelTimeSeconds = std::stoi(parameterWithValueVector[1]) * 60;
          if (maxEgressWalkingTravelTimeSeconds == 0)
          {
            maxEgressWalkingTravelTimeSeconds = MAX_INT;
          }
          break;
        }
        else if (parameterWithValueVector[0] == "max_egress_travel_time_seconds")
        {
          maxEgressWalkingTravelTimeSeconds = std::stoi(parameterWithValueVector[1]);
          if (maxEgressWalkingTravelTimeSeconds == 0)
          {
            maxEgressWalkingTravelTimeSeconds = MAX_INT;
          }
          break;
        }
        else if (parameterWithValueVector[0] == "max_transfer_travel_time" || parameterWithValueVector[0] == "max_transfer_travel_time_minutes")
        {
          maxTransferWalkingTravelTimeSeconds = std::stoi(parameterWithValueVector[1]) * 60;
          if (maxTransferWalkingTravelTimeSeconds == 0)
          {
            maxTransferWalkingTravelTimeSeconds = MAX_INT;
          }
          break;
        }
        else if (parameterWithValueVector[0] == "max_transfer_travel_time_seconds")
        {
          maxTransferWalkingTravelTimeSeconds = std::stoi(parameterWithValueVector[1]);
          if (maxTransferWalkingTravelTimeSeconds == 0)
          {
            maxTransferWalkingTravelTimeSeconds = MAX_INT;
          }
          break;
        }
        else if (parameterWithValueVector[0] == "max_only_walking_access_travel_time_ratio")
        {
          maxOnlyWalkingAccessTravelTimeRatio = std::stof(parameterWithValueVector[1]);
          break;
        }

        // od trips:
        else if (parameterWithValueVector[0] == "od_trip_uuid")
        {
          odTripUuid = uuidGenerator(parameterWithValueVector[1]);
          break;
        }
        else if (parameterWithValueVector[0] == "od_trips")
        {
          if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1") { calculateAllOdTrips = true; }
          break;
        }
        else if (parameterWithValueVector[0] == "od_trips_sample_size" || parameterWithValueVector[0] == "sample_size" || parameterWithValueVector[0] == "sample")
        {
          odTripsSampleSize = std::stoi(parameterWithValueVector[1]);
          break;
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
          break;
        }
        else if (parameterWithValueVector[0] == "od_trips_genders")
        {
          boost::split(odTripsGendersVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string odTripsGender : odTripsGendersVector)
          {
            odTripsGenders.push_back(odTripsGender);
          }
          break;
        }
        else if (parameterWithValueVector[0] == "od_trips_age_groups")
        {
          boost::split(odTripsAgeGroupsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string odTripsAgeGroup : odTripsAgeGroupsVector)
          {
            odTripsAgeGroups.push_back(odTripsAgeGroup);
          }
          break;
        }
        else if (parameterWithValueVector[0] == "od_trips_occupations")
        {
          boost::split(odTripsOccupationsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string odTripsOccupation : odTripsOccupationsVector)
          {
            odTripsOccupations.push_back(odTripsOccupation);
          }
          break;
        }
        else if (parameterWithValueVector[0] == "od_trips_activities")
        {
          boost::split(odTripsActivitiesVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string odTripsActivity : odTripsActivitiesVector)
          {
            odTripsActivities.push_back(odTripsActivity);
          }
          break;
        }
        else if (parameterWithValueVector[0] == "od_trips_modes")
        {
          boost::split(odTripsModesVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string odTripsMode : odTripsModesVector)
          {
            odTripsModes.push_back(odTripsMode);
          }
          break;
        }

        // alternatives:
        else if (parameterWithValueVector[0] == "alternatives" || parameterWithValueVector[0] == "alt")
        {
          if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1") { alternatives = true; }
          break;
        }
         else if (parameterWithValueVector[0] == "max_alternatives" || parameterWithValueVector[0] == "max_alt")
        {
          maxAlternatives = std::stoi(parameterWithValueVector[1]);
          break;
        }
        else if (parameterWithValueVector[0] == "alternatives_max_added_travel_time_minutes" || parameterWithValueVector[0] == "alt_max_added_travel_time")
        {
          alternativesMaxAddedTravelTimeSeconds = std::stoi(parameterWithValueVector[1]) * 60;
          break;
        }
        else if (parameterWithValueVector[0] == "alternatives_max_added_travel_time_seconds" || parameterWithValueVector[0] == "alt_max_added_travel_time_seconds")
        {
          alternativesMaxAddedTravelTimeSeconds = std::stoi(parameterWithValueVector[1]);
          break;
        }
        else if (parameterWithValueVector[0] == "alternatives_max_travel_time_ratio" || parameterWithValueVector[0] == "alt_max_ratio")
        {
          alternativesMaxTravelTimeRatio = std::stof(parameterWithValueVector[1]);
          break;
        }
        else if (parameterWithValueVector[0] == "alternatives_min_max_travel_time_minutes" || parameterWithValueVector[0] == "alt_min_max_travel_time")
        {
          minAlternativeMaxTravelTimeSeconds = std::stoi(parameterWithValueVector[1]) * 60;
          break;
        }
        else if (parameterWithValueVector[0] == "alternatives_min_max_travel_time_seconds" || parameterWithValueVector[0] == "alt_min_max_travel_time_seconds")
        {
          minAlternativeMaxTravelTimeSeconds = std::stoi(parameterWithValueVector[1]);
          break;
        }
        
        // other:
        else if (parameterWithValueVector[0] == "transfer_penalty" || parameterWithValueVector[0] == "transfer_penalty_minutes")
        {
          transferPenaltySeconds = std::stoi(parameterWithValueVector[1]) * 60;
          break;
        }
        else if (parameterWithValueVector[0] == "walking_speed_factor" || parameterWithValueVector[0] == "walk_factor") // > 1.0 means faster walking
        {
          walkingSpeedFactor = std::stof(parameterWithValueVector[1]);
          break;
        }
        else if (parameterWithValueVector[0] == "transfer_only_at_same_station"
                 || parameterWithValueVector[0] == "transfer_only_at_station"
                )
        {
          if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1") { transferOnlyAtSameStation = true; }
        }
        else if (parameterWithValueVector[0] == "detailed"
                 || parameterWithValueVector[0] == "detailed_results"
                 || parameterWithValueVector[0] == "detailed_result"
                )
        {
          if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1") { detailedResults = true; }
        }
        else if (parameterWithValueVector[0] == "transfer_between_same_line"
                 || parameterWithValueVector[0] == "allow_same_line_transfer"
                 || parameterWithValueVector[0] == "transfers_between_same_line"
                 || parameterWithValueVector[0] == "allow_same_line_transfers"
                )
        {
          if (parameterWithValueVector[1] == "false" || parameterWithValueVector[1] == "0") { transferBetweenSameLine = false; }
        }

        // not yet implemented
        else if (parameterWithValueVector[0] == "max_number_of_transfers" || parameterWithValueVector[0] == "max_transfers")
        {
          maxNumberOfTransfers = std::stoi(parameterWithValueVector[1]);
        }
        // not yet implemented
        else if (parameterWithValueVector[0] == "calculate_by_number_of_transfers" || parameterWithValueVector[0] == "by_num_transfers")
        {
          if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1")
          { 
            calculateByNumberOfTransfers = true;
          }
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
          break;
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
          break;
        }
        else if (parameterWithValueVector[0] == "access_node_travel_times_seconds" || parameterWithValueVector[0] == "access_node_travel_times")
        {
          boost::split(accessNodeTravelTimesSecondsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string accessNodeTravelTimeSeconds : accessNodeTravelTimesSecondsVector)
          {
            accessNodeTravelTimesSeconds.push_back(std::stoi(accessNodeTravelTimeSeconds));
          }
          calculator.params.accessNodeTravelTimesSeconds = accessNodeTravelTimesSeconds;
          break;
        }
        else if (parameterWithValueVector[0] == "egress_node_travel_times_seconds" || parameterWithValueVector[0] == "egress_node_travel_times")
        {
          boost::split(egressNodeTravelTimesSecondsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string egressNodeTravelTimeSeconds : egressNodeTravelTimesSecondsVector)
          {
            egressNodeTravelTimesSeconds.push_back(std::stoi(egressNodeTravelTimeSeconds));
          }
          calculator.params.egressNodeTravelTimesSeconds = egressNodeTravelTimesSeconds;
          break;
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
          break;
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
          break;
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
          break;
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
          break;
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
          break;
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
          break;
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
          break;
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
          break;
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
          break;
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
          break;
        }
        */


        else if (parameterWithValueVector[0] == "date")
        {
          boost::split(dateVector, parameterWithValueVector[1], boost::is_any_of("/"));
          routingDateYear  = std::stoi(dateVector[0]);
          routingDateMonth = std::stoi(dateVector[1]);
          routingDateDay   = std::stoi(dateVector[2]);
          break;
        }

        else if (parameterWithValueVector[0] == "calculation_name" || parameterWithValueVector[0] == "name")
        {
          calculationName = parameterWithValueVector[1];
          break;
        }
        else if (parameterWithValueVector[0] == "file_format" || parameterWithValueVector[0] == "format" || parameterWithValueVector[0] == "response_format" || parameterWithValueVector[0] == "response")
        {
          responseFormat = parameterWithValueVector[1];
          break;
        }
        else if (parameterWithValueVector[0] == "batch")
        {
          batchNumber = std::stoi(parameterWithValueVector[1]);
          break;
        }
        else if (parameterWithValueVector[0] == "num_batches")
        {
          batchesCount = std::stoi(parameterWithValueVector[1]);
          break;
        }
        else if (parameterWithValueVector[0] == "save_to_file" || parameterWithValueVector[0] == "save_file")
        {
          if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1") { saveResultToFile = true; }
          break;
        }
        else if (parameterWithValueVector[0] == "debug")
        {
          if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1") { debugDisplay = true; }
          break;
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
    
  };
  
}


#endif // TR_PARAMETERS
