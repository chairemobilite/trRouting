#ifndef TR_TRANSIT_ROUTING_PARAMETERS
#define TR_TRANSIT_ROUTING_PARAMETERS

#include <vector>
#include <string>
#include "point.hpp"

namespace TrRouting
{
  struct Parameters {
    
    std::string applicationShortname;
    std::string dataFetcher; // csv, database, cache
    
    int routingDateYear;   // not implemented, use onlyServiceIds or exceptServiceIds for now
    int routingDateMonth;  // not implemented, use onlyServiceIds or exceptServiceIds for now
    int routingDateDay;    // not implemented, use onlyServiceIds or exceptServiceIds for now
    std::map<int, bool> onlyServiceIds;
    std::map<int, bool> exceptServiceIds;
    std::map<int, bool> onlyRouteIds;
    std::map<int, bool> exceptRouteIds;
    std::map<int, bool> onlyRouteTypeIds;
    std::map<int, bool> exceptRouteTypeIds;
    std::map<int, bool> onlyAgencyIds;
    std::map<int, bool> exceptAgencyIds;
    
    int departureTimeHour;
    int departureTimeMinutes;
    int arrivalTimeHour;
    int arrivalTimeMinutes;
    
    int maxTotalTravelTimeSeconds;
    int maxNumberOfTransfers;
    int minWaitingTimeSeconds;
    int transferPenaltySeconds;
    int maxAccessWalkingDistanceMeters;
    int maxAccessWalkingTravelTimeFromOriginToFirstStopSeconds;
    int maxAccessWalkingTravelTimeFromLastStopToDestinationSeconds;
    int maxTransferWalkingTravelTimeSeconds;
    int maxTotalWalkingTravelTimeSeconds;
    float maxOnlyWalkingAccessTravelTimeRatio;
    float walkingSpeedMetersPerSecond;
    float drivingSpeedMetersPerSecond;
    float cyclingSpeedMetersPerSecond;
    
    Point origin;
    Point destination;
    
    long long startingStopId;
    long long endingStopId;
    
    std::vector<unsigned long long> serviceIds;
    std::vector<unsigned long long> ignoreRouteIds;
    std::vector<unsigned long long> ignoreRouteTypeIds;
    
    std::string connectionsSqlWhereClause;
    std::string transfersSqlWhereClause;
    std::string databaseName;
    std::string databaseHost;
    std::string databaseUser;
    std::string databasePort;
    std::string databasePassword;
    std::string osrmRoutingWalkingPort;
    std::string osrmRoutingWalkingHost;
    std::string osrmRoutingDrivingPort;
    std::string osrmRoutingDrivingHost;
    std::string osrmRoutingCyclingPort;
    std::string osrmRoutingCyclingHost;
    
    std::string accessMode;
    std::string egressMode;
    bool tryNextModeIfRoutingFails;
    std::string noResultSecondMode;
    int noResultNextAccessTimeSecondsIncrement;
    int maxNoResultNextAccessTimeSeconds;
    
    bool returnAllStopsResult;         // keep results for all stops (used in creating accessibility map)
    bool forwardCalculation;           // forward calculation: default. if false: reverse calculation, will ride connections backward (useful when setting the arrival time)
    bool detailedResults;              // return detailed results when using results for all stops
    bool transferOnlyAtSameStation;    // will transfer only between stops having the same station_id (better performance, but make sure your stations are well designed and specified)
    bool transferBetweenSameRoute;     // allow transfers between the same route_id
    bool calculateByNumberOfTransfers; // calculate first the fastest route, then calculate with decreasing number of transfers until no route is found, return results for each number of transfers.
    
    void setDefaultValues()
    {
      walkingSpeedMetersPerSecond                                = 5/3.6; // 5 km/h
      drivingSpeedMetersPerSecond                                = 90/3.6; // 90 km/h
      cyclingSpeedMetersPerSecond                                = 25/3.6; // 25 km/h
      maxTotalTravelTimeSeconds                                  = -1; // -1 means no limit
      maxNumberOfTransfers                                       = -1; // -1 means no limit
      minWaitingTimeSeconds                                      = 5*60;
      maxAccessWalkingTravelTimeFromOriginToFirstStopSeconds     = 20*60;
      maxAccessWalkingTravelTimeFromLastStopToDestinationSeconds = 20*60;
      maxTransferWalkingTravelTimeSeconds                        = 20*60; // depends of transfer data provided
      maxTotalWalkingTravelTimeSeconds                           = 60*60; // not used right now
      maxOnlyWalkingAccessTravelTimeRatio                        = 1.5; // prefer walking only if it is faster than transit and total only walking travel time <= maxAccessWalkingTravelTimeFromOriginToFirstStopSeconds * this ratio
      transferPenaltySeconds                                     = 0; // not used right now
      connectionsSqlWhereClause                                  = "";
      transfersSqlWhereClause                                    = "true";
      databaseName                                               = "tr_all_dev";
      databasePort                                               = "5432";
      databaseHost                                               = "127.0.0.1";
      databaseUser                                               = "postgres";
      databasePassword                                           = "";
      osrmRoutingWalkingHost                                     = "localhost";
      osrmRoutingWalkingPort                                     = "5000";
      osrmRoutingDrivingHost                                     = "localhost";
      osrmRoutingDrivingPort                                     = "7000";
      osrmRoutingCyclingHost                                     = "localhost";
      osrmRoutingCyclingPort                                     = "8000";
      accessMode                                                 = "walking";
      egressMode                                                 = "walking";
      noResultSecondMode                                         = "driving";
      tryNextModeIfRoutingFails                                  = false;
      noResultNextAccessTimeSecondsIncrement                     = 5*60;
      maxNoResultNextAccessTimeSeconds                           = 40*60;
      returnAllStopsResult                                       = false;
      forwardCalculation                                         = true;
      detailedResults                                            = false;
      transferOnlyAtSameStation                                  = false;
      transferBetweenSameRoute                                   = true;
      calculateByNumberOfTransfers                               = false;
    }
    
  };
  
}


#endif // TR_TRANSIT_ROUTING_PARAMETERS
