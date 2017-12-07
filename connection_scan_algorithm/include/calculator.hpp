#ifndef TR_CALCULATOR
#define TR_CALCULATOR


#include <pqxx/pqxx> 
#include <string>
#include <ctime>
#include <utility>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <iterator>
#include <vector>
#include <math.h>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
//#include <boost/compute.hpp>
#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <limits>
#include <stdlib.h>

#include "json.hpp"
#include "toolbox.hpp"
#include "stop.hpp"
#include "route.hpp"
#include "trip.hpp"
#include "point.hpp"
#include "parameters.hpp"
#include "routing_result.hpp"
#include "osrm_fetcher.hpp"
#include "calculation_time.hpp"
#include "database_fetcher.hpp"
#include "cache_fetcher.hpp"
#include "gtfs_fetcher.hpp"
#include "csv_fetcher.hpp"

extern int stepCount;

extern std::string consoleRed;
extern std::string consoleGreen;
extern std::string consoleYellow;
extern std::string consoleCyan;
extern std::string consoleMagenta;
extern std::string consoleResetColor;

namespace TrRouting
{
  
  class Calculator {
  
  public:
    
    std::string applicationShortname;
    Calculator();
    Calculator(Parameters& theParams);
    void                    prepare();
    void                    reset();
    RoutingResult           calculate();
    std::tuple<int,int,int> forwardCalculation(); // best arrival time,   best egress stop index, best egress travel time: MAX_INT,-1,-1 if non routable, too long or all stops result
    std::tuple<int,int,int> reverseCalculation(); // best departure time, best access stop index, best access travel time: -1,-1,-1 if non routable, too long or all stops result
    RoutingResult           forwardJourney(int bestArrivalTime, int bestEgressStopIndex, int bestEgressTravelTime);
    RoutingResult           reverseJourney(int bestDepartureTime, int bestAccessStopIndex, int bestAccessTravelTime);

    Parameters params;
    CalculationTime algorithmCalculationTime;
    std::vector<OdTrip>                  odTrips;
    std::map<unsigned long long, int>    odTripIndexesById;

  private:
    
    enum connectionIndexes : short { STOP_DEP = 0, STOP_ARR = 1, TIME_DEP = 2, TIME_ARR = 3, TRIP = 4, CAN_BOARD = 5, CAN_UNBOARD = 6, SEQUENCE = 7 };
    std::map<std::string,int> pickUpTypes = {
      {"regular", 0},
      {"no_pickup", 1},
      {"must_phone", 2},
      {"must_coordinate_with_driver", 3}
    };
    std::map<std::string,int> dropOffTypes = {
      {"regular", 0},
      {"no_drop_off", 1},
      {"must_phone", 2},
      {"must_coordinate_with_driver", 3}
    };
    
    int                                  departureTimeSeconds;
    int                                  initialDepartureTimeSeconds;
    int                                  arrivalTimeSeconds;
    std::vector<Stop>                    stops;
    std::map<unsigned long long, int>    stopIndexesById;
    std::vector<Route>                   routes;
    std::map<unsigned long long, int>    routeIndexesById;
    std::vector<Trip>                    trips;
    std::map<unsigned long long, int>    tripIndexesById;
    std::vector<std::tuple<int,int,int>> footpaths; // tuple: departingStopIndex, arrivalStopIndex, walkingTravelTimeSeconds
    std::vector<std::pair<int,int>>      footpathsRanges; // index: stopIndex, pair: index of first footpath, index of last footpath
    std::vector<int>                     stopsTentativeTime; // arrival time at stop (MAX_INT if not yet reached or unreachable)
    std::vector<int>                     stopsReverseTentativeTime; // departure time at stop (MAX_INT if not yet reached or unreachable)
    //std::vector<int>                     stopsD; // for reverse calculation with best departure time
    ///std::vector<st::deque<std::pair<int,int>>> stopsReverseTentativeTime;
    std::vector<int>                     stopsAccessTravelTime; // travel time from origin to accessible stops (-1 if unreachable by access mode)
    std::vector<int>                     stopsEgressTravelTime; // travel time to reach destination (-1 if unreachable by egress mode)
    //std::vector<int>                     stopsEgressFootpathTravelTimesSeconds; // not sure we need this...
    std::vector<int>                     tripsEnterConnection; // index of the entering connection for each trip index 
    std::vector<int>                     tripsEnterConnectionTransferTravelTime; // index of the entering connection for each trip index 
    std::vector<int>                     tripsExitConnection; // index of the exiting connection for each trip index 
    std::vector<int>                     tripsExitConnectionTransferTravelTime; // index of the exiting connection for each trip index 
    //std::vector<int>                     tripsReverseTime;
    std::vector<int>                     tripsEnabled; // allow/disallow use of this trip during calculation
    std::vector<int>                     tripsUsable; // after forwarrd calculation, keep a list of usable trips in time range for reverse calculation
    std::vector<std::tuple<int,int,int,int,int,short,short,int>> forwardConnections; // tuple: initialDepartureTimeSecondsStopIndex, arrivalStopIndex, departureTimeSeconds, arrivalTimeSeconds, tripIndex, canBoard, canUnboard, sequence in trip
    std::vector<std::tuple<int,int,int,int,int,short,short,int>> reverseConnections; // tuple: departureStopIndex, arrivalStopIndex, departureTimeSeconds, arrivalTimeSeconds, tripIndex, canBoard, canUnboard, sequence in trip
    std::vector<std::pair<int,int>>      accessFootpaths; // pair: accessStopIndex, walkingTravelTimeSeconds
    std::vector<std::pair<int,int>>      egressFootpaths; // pair: egressStopIndex, walkingTravelTimeSeconds
    std::vector<std::tuple<int,int,int,int,int,short>> forwardJourneys; // index = stop index, tuple: final enter connection, final exit connection, final footpath, final exit trip index, transfer travel time, is same stop transfer (first, second, third and fourth values = -1 for access and egress journeys)
    std::vector<std::tuple<int,int,int,int,int,short>> forwardEgressJourneys; // index = stop index, tuple: final enter connection, final exit connection, final footpath, final exit trip index, transfer travel time, is same stop transfer (first, second, third and fourth values = -1 for access and egress journeys)
    std::vector<std::tuple<int,int,int,int,int,short>> reverseJourneys; // index = stop index, tuple: final enter connection, final exit connection, final footpath, final exit trip index, transfer travel time, is same stop transfer (first, second, third and fourth values = -1 for access and egress journeys)
    std::vector<std::tuple<int,int,int,int,int,short>> reverseAccessJourneys; // index = stop index, tuple: final enter connection, final exit connection, final footpath, final exit trip index, transfer travel time, is same stop transfer (first, second, third and fourth values = -1 for access and egress journeys)
    int                                  maxTimeValue;
    int                                  minAccessTravelTime;
    int                                  maxEgressTravelTime;
    int                                  maxAccessTravelTime;
    int                                  minEgressTravelTime;
    std::string                          accessMode;
    std::string                          egressMode;
    int                                  maxAccessWalkingTravelTimeFromOriginToFirstStopSeconds;
    int                                  maxAccessWalkingTravelTimeFromLastStopToDestinationSeconds;
    long long                            calculationTime;
  };
  
}

#endif // TR_CALCULATOR
