#ifndef TR_NODE
#define TR_NODE

#include <vector>
#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "point.hpp"

namespace TrRouting
{
  
  struct Node {
  
  public:
   
    boost::uuids::uuid uuid;
    unsigned long long id;
    std::string code;
    std::string name;
    std::string internalId;
    int stationIdx;
    std::unique_ptr<Point> point;
    std::vector<int> transferableNodesIdx;
    std::vector<int> transferableTravelTimesSeconds;
    std::vector<int> transferableDistancesMeters;
    std::vector<int> reverseTransferableNodesIdx;
    std::vector<int> reverseTransferableTravelTimesSeconds;
    std::vector<int> reverseTransferableDistancesMeters;

    const std::string toString() {
      return "Node " + boost::uuids::to_string(uuid) + " (id " + std::to_string(id) + ")\n  code " + code + "\n  name " + name + "\n  latitude " + std::to_string(point.get()->latitude)  + "\n  longitude " + std::to_string(point.get()->longitude);
    }

  };

}

#endif // TR_NODE
