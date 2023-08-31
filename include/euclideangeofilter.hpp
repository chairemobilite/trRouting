#ifndef TR_EUCLIDEAN_GEO_FILTER
#define TR_EUCLIDEAN_GEO_FILTER

#include "geofilter.hpp"
#include <string>

namespace TrRouting
{
  /* Filter nodes using Euclidean distance */
  class EuclideanGeoFilter : public GeoFilter
  {
  public:
    EuclideanGeoFilter();
    
    virtual std::vector<NodeTimeDistance> getAccessibleNodesFootpathsFromPoint(const Point &point,
                                                                       const std::map<boost::uuids::uuid, Node> &nodes,
                                                                       int maxWalkingTravelTime,
                                                                       float walkingSpeedMetersPerSecond,
                                                                       bool reversed = false);    
  };
}

#endif // TR_EUCLIDEAN_GEO_FILTER
