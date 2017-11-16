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
#include <boost/compute.hpp>
#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <limits>
#include <stdlib.h>

#include "toolbox.hpp"
#include "stop.hpp"
#include "route.hpp"
#include "trip.hpp"
#include "point.hpp"
#include "parameters.hpp"
#include "routing_result.hpp"
#include "osrm_fetcher.hpp"
#include "calculation_time.hpp"

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
    int calculationId; // used to set reachable connections without having to reset a bool every time
    Calculator();
    Calculator(Parameters& theParams);
    void prepare();
    RoutingResult calculate();
    void reset();
    void updateParams(Parameters& theParams);
    Parameters params;
    void resetAccessEgressModes();
    CalculationTime algorithmCalculationTime;
    
  private:
    
    enum connectionIndexes : short { STOP_DEP = 0, STOP_ARR = 1, TIME_DEP = 2, TIME_ARR = 3, TRIP = 4, CAN_BOARD = 5, CAN_UNBOARD = 6 };
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
    int                                  maxUnboardingTimeSeconds; // the maximum unboarding time possible, according to parameters
    std::vector<Stop>                    stops;
    std::map<unsigned long long, int>    stopIndexesById;
    std::vector<Route>                   routes;
    std::map<unsigned long long, int>    routeIndexesById;
    std::vector<Trip>                    trips;
    std::map<unsigned long long, int>    tripIndexesById;
    std::vector<std::tuple<int,int,int>> footpaths; // tuple: departingStopIndex, arrivalStopIndex, walkingTravelTimeSeconds
    std::vector<std::pair<int,int>>      footpathsRanges; // index: stopIndex, pair: index of first footpath, index of last footpath
    std::vector<int>                     stopsTentativeTime; // arrival time at stop (MAX_INT if not yet reached or unreachable)
    std::vector<int>                     stopsAccessTravelTime; // travel time from origin to accessible stops (-1 if unreachable by access mode)
    std::vector<int>                     stopsEgressTravelTime; // travel time to reach destination (-1 if unreachable by egress mode)
    //std::vector<int>                     stopsEgressFootpathTravelTimesSeconds; // not sure we need this...
    std::vector<int>                     tripsEnterConnection; // index of the entering connection for each trip index 
    std::vector<int>                     tripsEnterConnectionTransferTravelTime; // index of the entering connection for each trip index 
    std::vector<int>                     tripsEnabled; // allow/disallow use of this trip during calculation
    std::vector<std::tuple<int,int,int,int,int,short,short>> forwardConnections; // tuple: departureStopIndex, arrivalStopIndex, departureTimeSeconds, arrivalTimeSeconds, tripIndex, canBoard, canUnboard
    std::vector<std::tuple<int,int,int,int,int,short,short>> reverseConnections; // tuple: departureStopIndex, arrivalStopIndex, departureTimeSeconds, arrivalTimeSeconds, tripIndex, canBoard, canUnboard
    std::vector<std::pair<int,int>>      accessFootpaths; // tuple: accessStopIndex, walkingTravelTimeSeconds
    std::vector<std::pair<int,int>>      egressFootpaths; // tuple: egressStopIndex, walkingTravelTimeSeconds
    std::vector<std::tuple<int,int,int,int,int>> journeys; // index = stop index, tuple: final enter connection, final exit connection, final footpath, final exit trip index, transfer travel time (first, second, third and fourth values = -1 for access and egress journeys)
    int                                  maxTimeValue;
    std::string                          accessMode;
    std::string                          egressMode;
    int                                  maxAccessWalkingTravelTimeFromOriginToFirstStopSeconds;
    int                                  maxAccessWalkingTravelTimeFromLastStopToDestinationSeconds;
  };
  
}

#endif // TR_CALCULATOR
