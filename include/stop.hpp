#ifndef TR_STOP
#define TR_STOP

#include <boost/serialization/access.hpp>
#include "point.hpp"

namespace TrRouting
{
  
  struct Stop {
  
  public:
   
    unsigned long long id;
    std::string code;
    std::string name;
    long long stationId;
    Point point;
  
  };

}

#endif // TR_STOP
