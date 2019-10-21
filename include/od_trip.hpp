#ifndef TR_OD_TRIP
#define TR_OD_TRIP

#include <vector>
#include <boost/uuid/uuid.hpp>
#include "point.hpp"

namespace TrRouting
{
  
  struct OdTrip {
  
  public:

    boost::uuids::uuid uuid;
    unsigned long long id;
    int dataSourceIdx;
    int householdIdx;
    int personIdx;
    int departureTimeSeconds;
    int arrivalTimeSeconds;
    int walkingTravelTimeSeconds;
    int cyclingTravelTimeSeconds;
    int drivingTravelTimeSeconds;
    float expansionFactor;
    std::string mode;
    std::string originActivity;
    std::string destinationActivity;
    std::string internalId;
    std::vector<int> originNodesIdx;
    std::vector<int> originNodesTravelTimesSeconds;
    //std::vector<int> originNodesDistancesMeters;
    std::vector<int> destinationNodesIdx;
    std::vector<int> destinationNodesTravelTimesSeconds;
    //std::vector<int> destinationNodesDistancesMeters;
    std::unique_ptr<Point> origin;
    std::unique_ptr<Point> destination;

    const std::string toString() {
      return "Od trip " + boost::uuids::to_string(uuid);
    }

  };

  

}

#endif // TR_OD_TRIP
