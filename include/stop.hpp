#ifndef TR_STOP
#define TR_STOP

#include <vector>
#include <string>
#include <boost/uuid/uuid.hpp>

#include "point.hpp"

namespace TrRouting
{
  
  struct Stop {
  
  public:
   
    boost::uuids::uuid uuid;
    unsigned long long id;
    std::string code;
    std::string name;
    boost::uuids::uuid nodeUuid;
    std::unique_ptr<Point> point;
    std::vector<int> transferableStopsIdx;
    std::vector<int> transferableTravelTimesSeconds;
    std::vector<int> transferableDistancesMeters;
  
  };

}

#endif // TR_STOP
