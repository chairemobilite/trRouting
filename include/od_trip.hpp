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
    boost::uuids::uuid personUuid;
    boost::uuids::uuid householdUuid;
    int age; // -1: unknown or nil
    int departureTimeSeconds;
    int arrivalTimeSeconds;
    int walkingTravelTimeSeconds;
    int cyclingTravelTimeSeconds;
    int drivingTravelTimeSeconds;
    float expansionFactor;
    std::string ageGroup;
    std::string gender;
    std::string mode;
    std::string occupation;
    std::string originActivity;
    std::string destinationActivity;
    long long accessFootpathsStartIndex;
    long long accessFootpathsEndIndex;
    long long egressFootpathsStartIndex;
    long long egressFootpathsEndIndex;
    Point origin;
    Point destination;
    Point homeLocation;

  };

}

#endif // TR_OD_TRIP
