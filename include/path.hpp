#ifndef TR_PATH
#define TR_PATH

#include <vector>
#include <string>
#include <boost/uuid/uuid.hpp>
#include "node.hpp"

namespace TrRouting
{
  class Line;
  class Trip;
  
  class Path {
  
  public:
    Path(const boost::uuids::uuid &auuid,
         const Line &aline,
         const std::string &adirection,
         const std::string &ainternalId,
         const std::vector<std::reference_wrapper<const Node>> &anodesRef,
         const std::vector<std::reference_wrapper<const Trip>> &atripsRef,
         const std::vector<int> &asegmentsTravelTimeSeconds,
         const std::vector<int> &asegmentsDistanceMeters):
      uuid(auuid),
      line(aline),
      direction(adirection),
      internalId(ainternalId),
      nodesRef(anodesRef),
      tripsRef(atripsRef),
      segmentsTravelTimeSeconds(asegmentsTravelTimeSeconds),
      segmentsDistanceMeters(asegmentsDistanceMeters) {}

    /* Alternative constructor where we pass a NodeTimeDistance vector instead of
     separate vectors for nodes, segments time and distance. Easier to handle in some cases */
    Path(const boost::uuids::uuid &auuid,
         const Line &aline,
         const std::string &adirection,
         const std::string &ainternalId,
         const std::vector<NodeTimeDistance> &anodesTimeDistance,
         const std::vector<std::reference_wrapper<const Trip>> &atripsRef):
      uuid(auuid),
      line(aline),
      direction(adirection),
      internalId(ainternalId),
      tripsRef(atripsRef)
      {
        for (const NodeTimeDistance & ntd: anodesTimeDistance) {
          nodesRef.push_back(ntd.node);
          segmentsTravelTimeSeconds.push_back(ntd.time);
          segmentsDistanceMeters.push_back(ntd.distance);
        }
      }

    boost::uuids::uuid uuid;
    const Line &line;
    std::string direction;
    std::string internalId;
    std::vector<std::reference_wrapper<const Node>> nodesRef;
    std::vector<std::reference_wrapper<const Trip>> tripsRef;
    //TODO Should probably be integrated with nodes as a NodeTimeDistance object. Need
    // to validate their usage
    std::vector<int> segmentsTravelTimeSeconds;
    std::vector<int> segmentsDistanceMeters;

    const std::string toString() {
      return "Path " + boost::uuids::to_string(uuid) + "\n  direction " + direction;
    }

  };

}

#endif // TR_PATH
