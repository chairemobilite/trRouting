#ifndef TR_ROUTING_RESULT
#define TR_ROUTING_RESULT

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
    std::vector<unsigned long long> routeIds;
    std::vector<unsigned long long> routeTypeIds;
    std::vector<unsigned long long> agencyIds;
    std::vector<std::tuple<unsigned long long, unsigned long long, unsigned long long, int, int>> legs; // tuple: tripId, routeId, routePathId, boarding sequence, unboarding sequence
    
  };
  
}

#endif // TR_ROUTING_RESULT
