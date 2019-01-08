#ifndef TR_TRIP_BASED_ALGORITHM
#define TR_TRIP_BASED_ALGORITHM

#include <string>
#include <ctime>
#include <utility>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <msgpack.hpp>
#include <json.hpp>
#include <yaml-cpp/yaml.h>
#include <limits>
#include <thread>

#include "parameters.hpp"
#include "footpath.hpp"
#include "route_path.hpp"
#include "trip.hpp"
#include "transfer.hpp"
#include "stop.hpp"
#include "reachable_route_path.hpp"
#include "trip_segment.hpp"
#include "calculation_time.hpp"

using json = nlohmann::json;

extern std::string consoleRed;
extern std::string consoleGreen;
extern std::string consoleYellow;
extern std::string consoleCyan;
extern std::string consoleMagenta;
extern std::string consoleResetColor;

namespace TrRouting
{
  
  class TripBasedAlgorithm {
  
  public:
    
    std::string projectShortname;
    TripBasedAlgorithm();
    TripBasedAlgorithm(Parameters& theParams);
    void setup();
    void setParamsFromYaml(std::string yamlFilePath = "");
    json calculate();
    void refresh();
    void destroy();
    void updateParams(Parameters& theParams);
    Parameters params;
    void resetAccessEgressModes();
    CalculationTime algorithmCalculationTime;
    
  private:
    
    std::vector<std::vector<int>>              footpathsIndex;
    std::vector<Footpath>                      footpathsBySource;
    std::vector<Footpath>                      footpathsByTarget;
    std::vector<RoutePath>                     routePaths;
    std::vector<int>                           routePathsIndexById;
    std::vector<std::vector<std::vector<int>>> routePathsIndexByStop;
    std::vector<Stop>                          stops;
    std::vector<int>                           stopsIndexById;
    std::vector<std::vector<int>>              stopsIndexByRoutePath;

    // by weekday:
    std::vector<std::vector<int>>              arrivalTimes        = std::vector<std::vector<int>>(7,std::vector<int>());
    std::vector<std::vector<int>>              departureTimes      = std::vector<std::vector<int>>(7,std::vector<int>());
    std::vector<std::vector<std::vector<int>>> arrivalTimesIndex   = std::vector<std::vector<std::vector<int>>>(7,std::vector<std::vector<int>>());
    std::vector<std::vector<std::vector<int>>> departureTimesIndex = std::vector<std::vector<std::vector<int>>>(7,std::vector<std::vector<int>>());
    std::vector<std::vector<Transfer>>         transfers           = std::vector<std::vector<Transfer>>(7,std::vector<Transfer>());
    std::vector<std::vector<std::vector<int>>> transfersIndex      = std::vector<std::vector<std::vector<int>>>(7,std::vector<std::vector<int>>());
    std::vector<std::vector<Trip>>             trips               = std::vector<std::vector<Trip>>(7,std::vector<Trip>());
    std::vector<std::vector<std::vector<int>>> tripsIndex          = std::vector<std::vector<std::vector<int>>>(7,std::vector<std::vector<int>>());
    std::vector<std::vector<int>>              tripsIndexById      = std::vector<std::vector<int>>(7,std::vector<int>());
    
    std::map<std::string,int> pickUpTypes;  // not yet implemented
    std::map<std::string,int> dropOffTypes; // not yet implemented
    std::string               accessMode;
    std::string               egressMode;
    int                       maxAccessWalkingTravelTimeFromOriginToFirstStopMinutes;
    int                       maxAccessWalkingTravelTimeFromLastStopToDestinationMinutes;

  };
  
}

#endif
