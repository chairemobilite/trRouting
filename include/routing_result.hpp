#ifndef TR_ROUTING_RESULT
#define TR_ROUTING_RESULT

#include <vector>
#include <string>

namespace TrRouting
{
  
  struct RoutingResult {
    
    int travelTimeSeconds;
    int arrivalTimeSeconds;
    int departureTimeSeconds;
    int numberOfTransfers;
    int inVehicleTravelTimeSeconds;
    int transferTravelTimeSeconds;
    int waitingTimeSeconds;
    int accessTravelTimeSeconds;
    int egressTravelTimeSeconds;
    int transferWaitingTimeSeconds;
    int firstWaitingTimeSeconds;
    int nonTransitTravelTimeSeconds;
    int calculationTimeMilliseconds;
    std::string status;
    std::string json;
    std::vector<boost::uuids::uuid> lineUuids;
    std::vector<boost::uuids::uuid> tripUuids;
    std::vector<boost::uuids::uuid> boardingNodeUuids;
    std::vector<boost::uuids::uuid> unboardingNodeUuids;
    std::vector<boost::uuids::uuid> agencyUuids;
    std::vector<std::string>        modeShortnames;
    std::vector<int>                tripsIdx;
    std::vector<int>                linesIdx;
    std::vector<int>                inVehicleTravelTimesSeconds;
    std::vector<std::tuple<boost::uuids::uuid, boost::uuids::uuid, boost::uuids::uuid, int, int>> legs; // tuple: tripUuid, lineUuid, pathUuid, boarding sequence, unboarding sequence
    
  };
  
}

#endif // TR_ROUTING_RESULT
