#ifndef TR_STATION
#define TR_STATION

#include <vector>
#include <string>
#include <boost/uuid/uuid.hpp>

#include "point.hpp"

namespace TrRouting
{
  
  struct Station {
  
  public:
   
    boost::uuids::uuid uuid;
    unsigned long long id;
    std::string code;
    std::string name;
    std::string internalId;
    std::unique_ptr<Point> point;
    
    const std::string toString() {
      return "Station " + boost::uuids::to_string(uuid) + " (id " + std::to_string(id) + ")\n  code " + code + "\n  name " + name;
    }

  };

}

#endif // TR_STATION
