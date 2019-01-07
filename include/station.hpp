#ifndef TR_STATION
#define TR_STATION

#include <vector>
#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/optional.hpp>

#include "point.hpp"

namespace TrRouting
{
  
  struct Station {
  
  public:
   
    boost::uuids::uuid uuid;
    unsigned long long id;
    std::string code;
    std::string name;
    Point point;
  
  };

}

#endif // TR_STATION
