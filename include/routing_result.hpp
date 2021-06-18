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
    int initialDepartureTimeSeconds;
    int initialLostTimeAtDepartureSeconds;
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
    nlohmann::json json;
    std::vector<boost::uuids::uuid> lineUuids;
    std::vector<boost::uuids::uuid> tripUuids;
    std::vector<boost::uuids::uuid> boardingNodeUuids;
    std::vector<boost::uuids::uuid> unboardingNodeUuids;
    std::vector<boost::uuids::uuid> agencyUuids;
    std::vector<std::string>        modeShortnames;
    std::vector<int>                tripsIdx;
    std::vector<int>                linesIdx;
    std::vector<int>                inVehicleTravelTimesSeconds;
    std::vector<std::tuple<int, int, int, int, int>> legs; // tuple: tripIdx, lineIdx, pathIdx, start connection index, end connection index
    
  };
  
}

#endif // TR_ROUTING_RESULT
