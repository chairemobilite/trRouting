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
    std::string gender;
    std::string mode;
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
      ar & gender;
      ar & mode;
      ar & departureTimeSeconds;
      ar & accessFootpaths;
      ar & egressFootpaths;
      ar & origin;
      ar & destination;
    }
  };

}

#endif // TR_OD_TRIP
