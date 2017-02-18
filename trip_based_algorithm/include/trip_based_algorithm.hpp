#ifndef TR_TRIP_BASED_ALGORITHM
#define TR_TRIP_BASED_ALGORITHM

#include <string>
#include <ctime>
#include <utility>
#include <iostream>
#include <sstream>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <msgpack.hpp>
#include <json.hpp>
#include <yaml-cpp/yaml.h>
#include <limits>

#include "parameters.hpp"
#include "footpath.hpp"
#include "route_path.hpp"
#include "trip.hpp"
#include "stop.hpp"
#include "calculation_time.hpp"


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
    
    std::string applicationShortname;
    TripBasedAlgorithm();
    TripBasedAlgorithm(Parameters& theParams);
    void setup();
    void setParamsFromYaml(std::string yamlFilePath = "");
    std::string calculate();
    void refresh();
    void updateParams(Parameters& theParams);
    Parameters params;
    void resetAccessEgressModes();
    
  private:
    
    std::vector<std::vector<int> > footpathsIndex;
    std::vector<Footpath> footpathsBySource;
    std::vector<Footpath> footpathsByTarget;
    std::vector<Trip> trips;
    std::vector<RoutePath> routePaths;
    std::vector<int> routePathsIndexById;
    std::vector<std::vector<std::vector<int> > > routePathsIndexByStop;
    std::vector<Stop> stops;
    
    std::map<std::string,int> pickUpTypes;     // not yet implemented
    std::map<std::string,int> dropOffTypes;    // not yet implemented
    std::string  accessMode;
    std::string  egressMode;
    int  maxAccessWalkingTravelTimeFromOriginToFirstStopMinutes;
    int  maxAccessWalkingTravelTimeFromLastStopToDestinationMinutes;

  };
  
}

#endif
