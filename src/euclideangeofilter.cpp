#include "euclideangeofilter.hpp"
#include "point.hpp"
#include "node.hpp"
#include "spdlog/spdlog.h"

namespace TrRouting
{

  EuclideanGeoFilter::EuclideanGeoFilter() {}


  std::vector<NodeTimeDistance> EuclideanGeoFilter::getAccessibleNodesFootpathsFromPoint(const Point &point,
                                                                                         const std::map<boost::uuids::uuid, Node> &nodes,
                                                                                         int maxWalkingTravelTime,
                                                                                         float walkingSpeedMetersPerSecond,
                                                                                         bool /*Unused reversed*/) {
    std::vector<NodeTimeDistance> accessibleNodesFootpaths;

    auto lengthOfOneDegree = calculateLengthOfOneDegree(point);
    float maxDistanceMetersSquared = calculateMaxDistanceSquared(maxWalkingTravelTime, walkingSpeedMetersPerSecond);
    float distanceMetersSquared;

    spdlog::debug("use of bird distance ");

    for (auto &&[uuid,node] : nodes)
    {
      distanceMetersSquared = calculateNodeDistanceSquared(node.point.get(), point, lengthOfOneDegree);

      if (distanceMetersSquared <= maxDistanceMetersSquared)
      {
        int distanceMeters = sqrt(distanceMetersSquared);
        int travelTimeSeconds = distanceMeters / walkingSpeedMetersPerSecond;
        accessibleNodesFootpaths.push_back(NodeTimeDistance(node, travelTimeSeconds, distanceMeters));
      }
    }

    spdlog::debug("fetched footpaths using bird distance ({} footpaths found)", accessibleNodesFootpaths.size());

    return accessibleNodesFootpaths;
  }
}
