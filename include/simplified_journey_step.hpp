#ifndef TR_SIMPLIFIED_JOURNEY_STEP
#define TR_SIMPLIFIED_JOURNEY_STEP

#include <vector>
#include "point.hpp"

namespace TrRouting
{
  
  enum JourneyStepAction { WALK, CYCLE, DRIVE, RIDE, BOARD, UNBOARD }; // CYCLE, DRIVE are not yet implemented
  
  struct SimplifiedJourneyStep {
    
    JourneyStepAction action;
    unsigned long long id;
    long long          connectionId;       // board, ride, unboard
    int                accessTimeMinutes;  // walk, cycle, drive
    long long          accessFromStopId;   // walk, cycle, drive
    long long          accessFromTripId;   // walk, cycle, drive
    int                readyToBoardMinuteOfDay;  // walk, cycle, drive
    //std::string        mode; // walk, cycle, drive
    //long long arrivingStopId;     // walk, cycle, drive, unboard
    
    //long long arrivingAgencyId; // walk, cycle, drive, unboard
    //long long departingAgencyId; // walk, cycle, drive, board
    //long long arrivingStopId; // walk, cycle, drive, unboard
    //long long departingStopId; // walk, cycle, drive, board
    //long long arrivingPathStopSequenceId; // walk, cycle, drive, unboard
    //long long departingPathStopSequenceId; // walk, cycle, drive, board
    //long long arrivingRouteId; // walk, cycle, drive, unboard
    //long long departingRouteId; // walk, cycle, drive, board
    //long long arrivingTripId; // walk, cycle, drive, unboard
    //long long departingTripId; // walk, cycle, drive, board
    //long long arrivingConnectionId; // walk, cycle, drive, unboard
    //long long departingConnectionId; // walk, cycle, drive, board
    //long long arrivingRouteTypeId; // walk, cycle, drive, unboard
    //long long departingRouteTypeId; // walk, cycle, drive, board
    //int waitingTimeMinutes; // board
    //int walkingTimeMinutes; // walk
    //int cyclingTimeMinutes; // cycle
    //int drivingTimeMinutes; // drive
    //int ridingTimeMinutes;  // ride
    //int arrivingMinuteOfDay; // walk, cycle, drive, unboard
    //int departingMinuteOfDay; // walk, cycle, drive, board
    //int readyToBoardMinuteOfDay; // walk, cycle, drive
    
  };
  

}

#endif // TR_SIMPLIFIED_JOURNEY_STEP
