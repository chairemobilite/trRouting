#ifndef TR_OSRM_FETCHER
#define TR_OSRM_FETCHER


#include <string>
#include <vector>
#include <memory>
#include <tuple>

namespace TrRouting
{
  class Point;
  class Node;

  class OsrmFetcher
  {

  public:
    //TODO Eventually, this data should be private to an instance, but for now all use case are static
    static std::string osrmWalkingPort;
    static std::string osrmCyclingPort;
    static std::string osrmDrivingPort;
    static std::string osrmWalkingHost;
    static std::string osrmCyclingHost;
    static std::string osrmDrivingHost;

    //TODO This should be managed outside of the osrm_fetcher, with a dedicated data fetcher
    static bool birdDistanceAccessibilityEnabled; // true if the accessibility information is obtained using bird distances instead of osrm

    static std::vector<std::tuple<int, int, int>> getAccessibleNodesFootpathsFromPoint(const Point &point, const std::vector<std::unique_ptr<Node>> &nodes, std::string mode, int maxWalkingTravelTime, float walkingSpeedMetersPerSecond, bool reversed = false);

  protected:
    static std::vector<std::tuple<int, int, int>> getNodesFromBirdDistance(const Point &point, const std::vector<std::unique_ptr<Node>> &nodes, int maxWalkingTravelTime, float walkingSpeedMetersPerSecond);
    static std::vector<std::tuple<int, int, int>> getNodesFromOsrm(const Point &point, const std::vector<std::unique_ptr<Node>> &nodes, std::string mode, int maxWalkingTravelTime, float walkingSpeedMetersPerSecond, bool reversed);

    static std::tuple<float, float> calculateLengthOfOneDegree(const Point &point);
    static float calculateMaxDistanceSquared(int maxWalkingTravelTime, float walkingSpeed);
    static float calculateNodeDistanceSquared(const Point *node, const Point &point, const std::tuple<float, float> &lengthOfOneDegree);
  };
}

#endif // TR_OSRM_FETCHER
