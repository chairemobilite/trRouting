#ifndef TR_NODE
#define TR_NODE

#include <vector>
#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/optional.hpp>

#include "point.hpp"

namespace TrRouting
{
  
  struct Node {
  
  public:
   
    boost::uuids::uuid uuid;
    unsigned long long id;
    std::string code;
    std::string name;
    boost::optional<boost::uuids::uuid> stationUuid;
    Point point;
    std::vector<int> transferableNodesIdx;
    std::vector<int> transferableTravelTimesSeconds;
    std::vector<int> transferableDistancesMeters;
  
  };

}

#endif // TR_NODE
