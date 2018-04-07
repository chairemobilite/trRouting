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
    
    std::string applicationShortname;
    std::string dataFetcherShortname; // csv, database, cache, gtfs
    DatabaseFetcher* databaseFetcher;
    CacheFetcher*    cacheFetcher;
    GtfsFetcher*     gtfsFetcher;
    CsvFetcher*      csvFetcher;
    
    int routingDateYear;   // not implemented, use onlyServiceIds or exceptServiceIds for now
    int routingDateMonth;  // not implemented, use onlyServiceIds or exceptServiceIds for now
    int routingDateDay;    // not implemented, use onlyServiceIds or exceptServiceIds for now
    std::vector<int> onlyServiceIds;
    std::vector<unsigned long long> exceptServiceIds;
    std::vector<unsigned long long> onlyRouteIds;
    std::vector<unsigned long long> exceptRouteIds;
    std::vector<unsigned long long> onlyRouteTypeIds;
    std::vector<unsigned long long> exceptRouteTypeIds;
    std::vector<unsigned long long> onlyAgencyIds;
    std::vector<unsigned long long> exceptAgencyIds;
    std::vector<unsigned long long> accessStopIds;
    std::vector<int>                accessStopTravelTimesSeconds;
    std::vector<unsigned long long> egressStopIds;
    std::vector<int>                egressStopTravelTimesSeconds;
    
    std::vector<std::pair<int,int>> odTripsPeriods; // pair: start_at_seconds, end_at_seconds
    std::vector<std::string>        odTripsGenders;
    std::vector<std::string>        odTripsAgeGroups;
    std::vector<std::string>        odTripsOccupations;
    std::vector<std::string>        odTripsActivities;
    std::vector<std::string>        odTripsModes;

    int departureTimeHour;
    int departureTimeMinutes;
    int arrivalTimeHour;
    int arrivalTimeMinutes;
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
    float walkingSpeedMetersPerSecond;
    float drivingSpeedMetersPerSecond;
    float cyclingSpeedMetersPerSecond;
    
    Point origin;
    Point destination;
    long long originStopId;
    long long destinationStopId;
    OdTrip* odTrip;
    
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
    bool debugDisplay; // display performance and debug info when set to true
    bool tryNextModeIfRoutingFails;
    std::string noResultSecondMode;
    int noResultNextAccessTimeSecondsIncrement;
    int maxNoResultNextAccessTimeSeconds;
    int maxAlternatives; // number of alternatives to calculate before returning results (when alternatives parameter is set to true)
    float alternativesMaxTravelTimeRatio; // travel time of fastest route is multiplied by this ratio to find plausible alternative with a max travel time.
    float minAlternativeMaxTravelTimeSeconds; // if multiplying max travel time ratio with max travel time is too small, keep max travel time to this minimum.
    int   alternativesMaxAddedTravelTimeSeconds; // how many seconds to add to fastest travel time to limit alternatives travel time.
    
    bool returnAllStopsResult;         // keep results for all stops (used in creating accessibility map)
    bool forwardCalculation;           // forward calculation: default. if false: reverse calculation, will ride connections backward (useful when setting the arrival time)
    bool detailedResults;              // return detailed results when using results for all stops
    bool transferOnlyAtSameStation;    // will transfer only between stops having the same station_id (better performance, but make sure your stations are well designed and specified)
    bool transferBetweenSameRoute;     // allow transfers between the same route_id
    bool calculateByNumberOfTransfers; // calculate first the fastest route, then calculate with decreasing number of transfers until no route is found, return results for each number of transfers.
    bool alternatives;                 // calculate alternatives or not
    
    void setDefaultValues()
    {
      odTrip                                 = NULL;
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
      databaseName                           = "tr_all_dev";
      databasePort                           = "5432";
      databaseHost                           = "127.0.0.1";
      databaseUser                           = "postgres";
      databasePassword                       = "";
      osrmRoutingWalkingHost                 = "localhost";
      osrmRoutingWalkingPort                 = "5000";
      osrmRoutingDrivingHost                 = "localhost";
      osrmRoutingDrivingPort                 = "7000";
      osrmRoutingCyclingHost                 = "localhost";
      osrmRoutingCyclingPort                 = "8000";
      accessMode                             = "walking";
      egressMode                             = "walking";
      noResultSecondMode                     = "driving";
      tryNextModeIfRoutingFails              = false;
      noResultNextAccessTimeSecondsIncrement = 5*60;
      maxNoResultNextAccessTimeSeconds       = 40*60;
      returnAllStopsResult                   = false;
      forwardCalculation                     = true;
      detailedResults                        = false;
      transferOnlyAtSameStation              = false;
      transferBetweenSameRoute               = true;
      calculateByNumberOfTransfers           = false;
      alternatives                           = false;
      maxAlternatives                        = 100;
      debugDisplay                           = false;
      alternativesMaxTravelTimeRatio         = 1.5;
      minAlternativeMaxTravelTimeSeconds     = 30*60;
      alternativesMaxAddedTravelTimeSeconds  = 30*60;
    }
    
  };
  
}


#endif // TR_PARAMETERS
