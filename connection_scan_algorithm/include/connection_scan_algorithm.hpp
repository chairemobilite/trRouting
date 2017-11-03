#ifndef TR_CONNECTION_SCAN_ALGORITHM
#define TR_CONNECTION_SCAN_ALGORITHM

#include <string>
#include <ctime>
#include <utility>
#include <iostream>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <yaml-cpp/yaml.h>
#include <limits>

#include "parameters.hpp"
#include "db_fetcher.hpp"
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
  
  class ConnectionScanAlgorithm {
  
  public:
    
    std::string applicationShortname;
    int calculationId; // used to set reachable connections without having to reset a bool every time
    long long journeyStepId; // used as ids for journey steps (will increment)
    std::map<long long, SimplifiedJourneyStep*> journeyStepsById; // map of all journey steps by id
    ConnectionScanAlgorithm();
    ConnectionScanAlgorithm(Parameters& theParams);
    void setup();
    std::string calculate(std::string tripIdentifier, const std::map<unsigned long long, int>& cachedNearestStopsIdsFromStartingPoint = std::map<unsigned long long, int>(), const std::map<unsigned long long, int>& cachedNearestStopsIdsFromEndingPoint = std::map<unsigned long long, int>());
    void refresh();
    std::map<unsigned long long, std::vector<Connection*> > getConnectionsByStartPathStopSequenceId(std::vector<Connection*> theConnectionsByDepartureTime);
    std::map<unsigned long long, std::vector<Connection*> > getConnectionsByEndPathStopSequenceId(std::vector<Connection*> theConnectionsByArrivalTime);
    std::map<unsigned long long, std::vector<unsigned long long> > getPathStopSequencesByStopId(std::map<unsigned long long,PathStopSequence> thePathStopSequencesById);
    void updateParams(Parameters& theParams);
    Parameters params;
    void resetAccessEgressModes();
    CalculationTime algorithmCalculationTime;
    
  private:
    
    int                                                                   maxUnboardingTimeMinutes; // the maximum unboarding time possible, according to parameters
    std::map<std::string,int>                                             pickUpTypes;
    std::map<std::string,int>                                             dropOffTypes;
    std::map<unsigned long long,std::map<unsigned long long, int> >       transferDurationsByStopId;
    std::map<unsigned long long,PathStopSequence>                         pathStopSequencesById;
    std::map<unsigned long long,std::vector<unsigned long long> >         pathStopSequencesByStopId;
    std::map<unsigned long long,Stop>                                     stopsById;
    std::map<unsigned long long,Route>                                    routesById;
    std::map<unsigned long long,Connection>                               forwardConnectionsById;
    std::map<unsigned long long,Connection>                               reverseConnectionsById;
    std::vector<Connection*>                                              connectionsByArrivalTime;
    std::vector<Connection*>                                              connectionsByDepartureTime;
    std::map<unsigned long long,std::vector<Connection*> >                connectionsByStartPathStopSequenceId;
    std::map<unsigned long long,std::vector<Connection*> >                connectionsByEndPathStopSequenceId;
    int                                                                   maxTimeValue;
    std::string                                                           accessMode;
    std::string                                                           egressMode;
    int                                                                   maxAccessWalkingTravelTimeFromOriginToFirstStopMinutes;
    int                                                                   maxAccessWalkingTravelTimeFromLastStopToDestinationMinutes;
  };
  
}

#endif
