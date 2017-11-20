#ifndef TR_ROUTING_RESULT
#define TR_ROUTING_RESULT

namespace TrRouting
{
  
  
  struct RoutingResult {
    
    int travelTimeSeconds;
    int arrivalTimeSeconds;
    int departureTimeSeconds;
    int numberOfTransfers;
    int calculationTimeMilliseconds;
    std::string status;
    std::string json;
    std::vector<std::tuple<unsigned long long, unsigned long long, unsigned long long, int, int>> legs; // tuple: tripId, routeId, routePathId, boarding sequence, unboarding sequence
    
  };
  
}

#endif // TR_ROUTING_RESULT
