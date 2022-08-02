#ifndef TR_PATH
#define TR_PATH

#include <vector>
#include <string>
#include <boost/uuid/uuid.hpp>

namespace TrRouting
{
  class Line;
  
  class Path {
  
  public:
    Path(const boost::uuids::uuid &auuid,
         const Line &aline,
         const std::string &adirection,
         const std::string &ainternalId,
         const std::vector<int> &anodesIdx,
         const std::vector<int> &atripsIdx,
         const std::vector<int> &asegmentsTravelTimeSeconds,
         const std::vector<int> &asegmentsDistanceMeters):
      uuid(auuid),
      line(aline),
      direction(adirection),
      internalId(ainternalId),
      nodesIdx(anodesIdx),
      tripsIdx(atripsIdx),
      segmentsTravelTimeSeconds(asegmentsTravelTimeSeconds),
      segmentsDistanceMeters(asegmentsDistanceMeters) {}
   
    boost::uuids::uuid uuid;
    const Line &line;
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
