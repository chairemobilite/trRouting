#ifndef TR_PARAMETERS
#define TR_PARAMETERS

#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <chrono>
#include <vector>
#include <map>
#include <math.h>
#include <boost/optional.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/algorithm/string.hpp>

#include "point.hpp"
#include "toolbox.hpp"
#include "od_trip.hpp"
#include "scenario.hpp"

namespace TrRouting
{
  
  class DatabaseFetcher;
  class CacheFetcher;
  class GtfsFetcher;
  class CsvFetcher;

  class Parameters {

    public:

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
      unsigned int seed;

      int routingDateYear;   // not implemented, use onlyServicesIdx or exceptServicesIdx for now
      int routingDateMonth;  // not implemented, use onlyServicesIdx or exceptServicesIdx for now
      int routingDateDay;    // not implemented, use onlyServicesIdx or exceptServicesIdx for now
      int onlyDataSourceIdx;
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
      std::vector<int> accessNodeDistancesMeters;
      std::vector<int> egressNodesIdx;
      std::vector<int> egressNodeTravelTimesSeconds;
      std::vector<int> egressNodeDistancesMeters;
      
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
      int maxFirstWaitingTimeSeconds;
      float odTripsSampleRatio;
      float maxOnlyWalkingAccessTravelTimeRatio;
      float walkingSpeedFactor;
      float walkingSpeedMetersPerSecond;
      float drivingSpeedMetersPerSecond;
      float cyclingSpeedMetersPerSecond;

      Point origin;
      Point destination;
      bool hasOrigin;
      bool hasDestination;
      int originNodeIdx;
      int destinationNodeIdx;
      bool calculateAllOdTrips;
      bool saveResultToFile;
      boost::optional<boost::uuids::uuid> scenarioUuid;
      boost::optional<boost::uuids::uuid> dataSourceUuid;
      boost::optional<boost::uuids::uuid> odTripUuid;
      boost::optional<boost::uuids::uuid> startingNodeUuid;
      boost::optional<boost::uuids::uuid> endingNodeUuid;
  
      std::string osrmWalkingPort;
      std::string osrmCyclingPort;
      std::string osrmDrivingPort;
      std::string osrmWalkingHost;
      std::string osrmCyclingHost;
      std::string osrmDrivingHost;
      
      std::string accessMode;
      std::string egressMode;
      bool debugDisplay; // display performance and debug info when set to true
      bool serverDebugDisplay; // same as debugDisplay, but set as override when starting the server
      bool tryNextModeIfRoutingFails;
      std::string noResultSecondMode;
      int noResultNextAccessTimeSecondsIncrement;
      int maxNoResultNextAccessTimeSeconds;
      int maxAlternatives; // number of alternatives to calculate before returning results (when alternatives parameter is set to true)
      float alternativesMaxTravelTimeRatio; // travel time of fastest route is multiplied by this ratio to find plausible alternative with a max travel time.
      float minAlternativeMaxTravelTimeSeconds; // if multiplying max travel time ratio with max travel time is too small, keep max travel time to this minimum.
      int   alternativesMaxAddedTravelTimeSeconds; // how many seconds to add to fastest travel time to limit alternatives travel time.
      int   maxValidAlternatives; // max number of valid alternatives to return

      bool returnAllNodesResult;         // keep results for all nodes (used in creating accessibility map)
      bool forwardCalculation;           // forward calculation: default. if false: reverse calculation, will ride connections backward (useful when setting the arrival time)
      bool detailedResults;              // return detailed results when using results for all nodes
      bool transferOnlyAtSameStation;    // will transfer only between nodes/stops having the same station_id (better performance, but make sure your stations are well designed and specified)
      bool transferBetweenSameLine;      // allow transfers between the same line
      bool calculateByNumberOfTransfers; // calculate first the fastest route, then calculate with decreasing number of transfers until no route is found, return results for each number of transfers.
      bool alternatives;                 // calculate alternatives or not
      bool calculateProfiles;            // calculate profiles for lines, paths and trips (od trips only)
      
      void setDefaultValues();
      bool isCompleteForCalculation();
      void update(std::vector<std::string> &parameters, std::map<boost::uuids::uuid, int> &scenarioIndexesByUuid, std::vector<std::unique_ptr<Scenario>> &scenarios, std::map<boost::uuids::uuid, int> &nodeIndexesByUuid, std::map<boost::uuids::uuid, int> &agencyIndexesByUuid, std::map<boost::uuids::uuid, int> &lineIndexesByUuid, std::map<boost::uuids::uuid, int> &serviceIndexesByUuid, std::map<std::string, int> &modeIndexesByShortname, std::map<boost::uuids::uuid, int> &dataSourceIndexesByUuid);
    
  };
  
}


#endif // TR_PARAMETERS
