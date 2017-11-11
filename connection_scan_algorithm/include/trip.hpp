#ifndef TR_TRIP
#define TR_TRIP

#include <vector>
#include <boost/serialization/access.hpp>

namespace TrRouting
{
  
  struct Trip {
  
  public:
   
    long long id;
    long long routeId;
    long long routeTypeId;
    long long agencyId;
  
  private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive&ar, const unsigned int version)
    {
        ar & id;
        ar & routeId;
        ar & routeTypeId;
        ar & agencyId;
    }
  };

}

#endif // TR_TRIP
