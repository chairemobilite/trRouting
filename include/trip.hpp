#ifndef TR_TRIP
#define TR_TRIP

#include <vector>
#include <boost/serialization/access.hpp>

namespace TrRouting
{
  
  struct Trip {
  
  public:
   
    unsigned long long id;
    unsigned long long routeId;
    unsigned long long routeTypeId;
    unsigned long long agencyId;
    unsigned long long serviceId;
  
  private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive&ar, const unsigned int version)
    {
        ar & id;
        ar & routeId;
        ar & routeTypeId;
        ar & agencyId;
        ar & serviceId;
    }
  };

}

#endif // TR_TRIP
