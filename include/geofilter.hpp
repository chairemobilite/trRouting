#ifndef TR_GEO_FILTER
#define TR_GEO_FILTER

#include <vector>
#include <map> //For node map types, TODO have a typedef somewhere for the nodes array
#include <boost/uuid/uuid.hpp>
#include <tuple>

namespace TrRouting
{
  class Point;
  class Node;
  class NodeTimeDistance;

  /* Base class to implement filter based in geography */
  class GeoFilter
  {
  public:
    virtual std::vector<NodeTimeDistance> getAccessibleNodesFootpathsFromPoint(const Point &point,
                                                                       const std::map<boost::uuids::uuid, Node> &nodes,
                                                                       int maxWalkingTravelTime,
                                                                       float walkingSpeedMetersPerSecond,
                                                                       bool reversed = false) = 0;
  protected:
    // Common utility functions
    static std::tuple<float, float> calculateLengthOfOneDegree(const Point &point);
    static float calculateMaxDistanceSquared(int maxWalkingTravelTime, float walkingSpeed);
    //TODO WHy one point * and other &
    static float calculateNodeDistanceSquared(const Point *node, const Point &point, const std::tuple<float, float> &lengthOfOneDegree);

  };
}

#endif // TR_GEO_FILTER
