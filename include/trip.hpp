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
    unsigned long long routePathId;
    unsigned long long routeTypeId;
    unsigned long long agencyId;
    unsigned long long serviceId;
  
  };

}

#endif // TR_TRIP
