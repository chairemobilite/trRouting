#include "geofilter.hpp"
#include "point.hpp"
#include "node.hpp"

namespace TrRouting
{
  std::tuple<float, float> GeoFilter::calculateLengthOfOneDegree(const Point &point)
  {
    // Taylor series approximation of the longitude and latitude length functions (For more info: https://gis.stackexchange.com/questions/75528/understanding-terms-in-length-of-degree-formula)
    float lengthOfOneDegreeOfLongitude = 111412.84 * cos(point.latitude * M_PI / 180) - 93.5 * cos(3 * point.latitude * M_PI / 180);
    float lengthOfOneDegreeOflatitude = 111132.92 - 559.82 * cos(2 * point.latitude * M_PI / 180) + 1.175 * cos(4 * point.latitude * M_PI / 180);

    return std::make_tuple(lengthOfOneDegreeOfLongitude, lengthOfOneDegreeOflatitude);
  }

  float GeoFilter::calculateMaxDistanceSquared(int maxWalkingTravelTime, float walkingSpeedMetersPerSecond)
  {
    return (maxWalkingTravelTime * walkingSpeedMetersPerSecond) * (maxWalkingTravelTime * walkingSpeedMetersPerSecond);
  }

  float GeoFilter::calculateNodeDistanceSquared(const Point *node, const Point &point, const std::tuple<float, float> &lengthOfOneDegree)
  {
    float distanceXMeters = (node->longitude - point.longitude) * std::get<0>(lengthOfOneDegree);
    float distanceYMeters = (node->latitude - point.latitude) * std::get<1>(lengthOfOneDegree);
    float distanceMetersSquared = distanceXMeters * distanceXMeters + distanceYMeters * distanceYMeters;

    return distanceMetersSquared;
  }

}
