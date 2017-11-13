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
    
  };
  
}

#endif // TR_ROUTING_RESULT
