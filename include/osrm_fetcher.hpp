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
  class Parameters;

  class OsrmFetcher
  {

  public:
    OsrmFetcher() {}
    OsrmFetcher(std::string projectShortname)
    {
    }

    static std::vector<std::tuple<int, int, int>> getAccessibleNodesFootpathsFromPoint(const Point &point, const std::vector<std::unique_ptr<Node>> &nodes, std::string mode, Parameters &params, int maxWalkingTravelTime, float walkingSpeedMetersPerSecond, bool reversed = false);

  protected:
    static std::vector<std::tuple<int, int, int>> getNodesFromBirdDistance(const Point &point, const std::vector<std::unique_ptr<Node>> &nodes, Parameters &params, int maxWalkingTravelTime, float walkingSpeedMetersPerSecond);
    static std::vector<std::tuple<int, int, int>> getNodesFromOsrm(const Point &point, const std::vector<std::unique_ptr<Node>> &nodes, std::string mode, Parameters &params, int maxWalkingTravelTime, float walkingSpeedMetersPerSecond, bool reversed);

    static std::tuple<float, float> calculateLengthOfOneDegree(const Point &point);
    static float calculateMaxDistanceSquared(int maxWalkingTravelTime, float walkingSpeed);
    static float calculateNodeDistanceSquared(const Point *node, const Point &point, const std::tuple<float, float> &lengthOfOneDegree);
  };

}

#endif // TR_OSRM_FETCHER
