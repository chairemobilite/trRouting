#ifndef TR_OD_TRIP
#define TR_OD_TRIP

#include <vector>
#include <boost/serialization/access.hpp>

#include "point.hpp"

namespace TrRouting
{
  
  struct OdTrip {
  
  public:
   
    unsigned long long id;
    unsigned long long personId;
    unsigned long long householdId;
    int age; // -1: unknown or nil
    int departureTimeSeconds;
    int walkingTravelTimeSeconds;
    int cyclingTravelTimeSeconds;
    int drivingTravelTimeSeconds;
    float expansionFactor;
    std::string ageGroup;
    std::string gender;
    std::string mode;
    std::string occupation;
    std::string activity;
    std::vector<std::pair<int,int>> accessFootpaths;
    std::vector<std::pair<int,int>> egressFootpaths;
    Point origin;
    Point destination;
  
  private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive&ar, const unsigned int version)
    {
      ar & id;
      ar & personId;
      ar & householdId;
      ar & age;
      ar & departureTimeSeconds;
      ar & walkingTravelTimeSeconds;
      ar & cyclingTravelTimeSeconds;
      ar & drivingTravelTimeSeconds;
      ar & ageGroup;
      ar & gender;
      ar & mode;
      ar & occupation;
      ar & activity;
      ar & accessFootpaths;
      ar & egressFootpaths;
      ar & origin;
      ar & destination;
      ar & expansionFactor;
    }
  };

}

#endif // TR_OD_TRIP
