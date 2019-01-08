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
    
    CacheFetcher*    cacheFetcher;
    GtfsFetcher*     gtfsFetcher;
    CsvFetcher*      csvFetcher;
    
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

    int departureTimeHour;
    int departureTimeMinutes;
    int departureTimeSeconds;
    int arrivalTimeHour;
    int arrivalTimeMinutes;
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
    float walkingSpeedMetersPerSecond;
    float drivingSpeedMetersPerSecond;
    float cyclingSpeedMetersPerSecond;
    
    Point origin;
    Point destination;
    int originNodeIdx;
    int destinationNodeIdx;
    OdTrip* odTrip;
    
    std::string osrmRoutingWalkingPort;
    std::string osrmRoutingWalkingHost;
    std::string osrmRoutingDrivingPort;
    std::string osrmRoutingDrivingHost;
    std::string osrmRoutingCyclingPort;
    std::string osrmRoutingCyclingHost;
    std::string osrmFilePath; // path to .osrm file
    bool osrmUseLib;
    boost::optional<osrm::OSRM> osrmRouter;
    int updateOdTrips; // if 1: update od trips access and egress nodes from database. Set to 1 only if nodes and/or od trips were modified.
    
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
      cacheDirectoryPath                     = "cache/transit/";
      odTrip                                 = NULL;
      walkingSpeedMetersPerSecond            = 5/3.6; // 5 km/h
      drivingSpeedMetersPerSecond            = 90/3.6; // 90 km/h
      cyclingSpeedMetersPerSecond            = 25/3.6; // 25 km/h
      maxTotalTravelTimeSeconds              = MAX_INT;
      maxNumberOfTransfers                   = -1; // -1 means no limit
      departureTimeSeconds                   = -1;
      arrivalTimeSeconds                     = -1;
      minWaitingTimeSeconds                  = 5*60;
      maxAccessWalkingTravelTimeSeconds      = 20*60;
      maxEgressWalkingTravelTimeSeconds      = 20*60;
      maxTransferWalkingTravelTimeSeconds    = 20*60; // depends of transfer data provided
      maxTotalWalkingTravelTimeSeconds       = 60*60; // not used right now
      maxOnlyWalkingAccessTravelTimeRatio    = 1.5; // prefer walking only if it is faster than transit and total only walking travel time <= maxAccessWalkingTravelTimeSeconds * this ratio
      transferPenaltySeconds                 = 0; // not used right now
      updateOdTrips                          = 0;
      osrmRoutingWalkingHost                 = "localhost";
      osrmRoutingWalkingPort                 = "5000";
      osrmRoutingDrivingHost                 = "localhost";
      osrmRoutingDrivingPort                 = "7000";
      osrmRoutingCyclingHost                 = "localhost";
      osrmRoutingCyclingPort                 = "8000";
      osrmUseLib                             = false;
      osrmFilePath                           = "osrm/walk/walk.osrm";
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
