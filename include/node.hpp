#ifndef TR_NODE
#define TR_NODE

#include <vector>
#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
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
    int stationIdx;
    Point point;
    std::vector<int> transferableNodesIdx;
    std::vector<int> transferableTravelTimesSeconds;
    std::vector<int> transferableDistancesMeters;

    const std::string toString() {
      return "Node " + boost::uuids::to_string(uuid) + " (id " + std::to_string(id) + ")\n  code " + code + "\n  name " + name + "\n  latitude " + std::to_string(point.latitude)  + "\n  longitude " + std::to_string(point.longitude);
    }

  };

}

#endif // TR_NODE
