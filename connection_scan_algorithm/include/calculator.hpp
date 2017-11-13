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

#include "stop.hpp"
#include "route.hpp"
#include "trip.hpp"
#include "point.hpp"
#include "parameters.hpp"
#include "routing_result.hpp"
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
    
    void prepareStops();
    void prepareRoutes();
    void prepareTrips();
    void prepareConnections();
    void prepareAccessFoothpaths();
    void prepareEgressFootpaths();
    
    void resetStopsTentativeArrivalTimes();
    void resetStopsEgressFootpathTravelTimesSeconds();
    void resetTripsEnterConnection();
    void resetJourneys();
    void resetVariables();

    std::map<std::string,int>                         pickUpTypes = {
      {"regular", 0},
      {"no_pickup", 1},
      {"must_phone", 2},
      {"must_coordinate_with_driver", 3}
    };
    std::map<std::string,int>                         dropOffTypes = {
      {"regular", 0},
      {"no_drop_off", 1},
      {"must_phone", 2},
      {"must_coordinate_with_driver", 3}
    };
    int                                               maxUnboardingTimeSeconds; // the maximum unboarding time possible, according to parameters
    std::vector<Stop>                                 stops;
    std::map<unsigned long long, int>                 stopIndexesById;
    std::vector<Route>                                routes;
    std::map<unsigned long long, int>                 routeIndexesById;
    std::vector<Trip>                                 trips;
    std::map<unsigned long long, int>                 tripIndexesById;
    std::vector<std::vector<std::tuple<int,int,int>>> footpaths; // tuple: departingStopIndex, arrivalStopIndex, walkingTravelTimeSeconds
    std::vector<int>                                  stopsTentativeArrivalTimesSeconds;
    std::vector<int>                                  stopsEgressFootpathTravelTimesSeconds;
    std::vector<int>                                  tripsEnterConnection; // index of the entering connection for each trip index  
    std::vector<std::tuple<int,int,int,int,int>>      forwardConnections; // tuple: departureStopIndex, arrivalStopIndex, departureTimeSeconds, arrivalTimeSeconds, tripIndex
    std::vector<std::tuple<int,int,int,int,int>>      reverseConnections; // tuple: departureStopIndex, arrivalStopIndex, departureTimeSeconds, arrivalTimeSeconds, tripIndex
    std::vector<std::pair<int,int>>                   accessFootpaths; // tuple: accessStopIndex, walkingTravelTimeSeconds
    std::vector<std::pair<int,int>>                   egressFootpaths; // tuple: egressStopIndex, walkingTravelTimeSeconds
    std::vector<std::tuple<int,int,int>>              journeys; // index = stop index, tuple: final enter connection, final exit connection, final footpath
    int                                               maxTimeValue;
    std::string                                       accessMode;
    std::string                                       egressMode;
    int                                               maxAccessWalkingTravelTimeFromOriginToFirstStopSeconds;
    int                                               maxAccessWalkingTravelTimeFromLastStopToDestinationSeconds;
  };
  
}

#endif // TR_CALCULATOR
