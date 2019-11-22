#ifndef TR_PATH
#define TR_PATH

#include <vector>
#include <string>
#include <boost/uuid/uuid.hpp>

namespace TrRouting
{
  
  struct Path {
  
  public:
   
    boost::uuids::uuid uuid;
    int lineIdx;
    std::string direction;
    std::string internalId;
    std::vector<int> nodesIdx;
    std::vector<int> tripsIdx;
    std::vector<int> segmentsTravelTimeSeconds;
    std::vector<int> segmentsDistanceMeters;

    const std::string toString() {
      return "Path " + boost::uuids::to_string(uuid) + "\n  direction " + direction;
    }

  };

}

#endif // TR_PATH
