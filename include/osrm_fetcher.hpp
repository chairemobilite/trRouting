#ifndef TR_OSRM_FETCHER
#define TR_OSRM_FETCHER


#include <string>
#include <vector>
#include <memory>
#include <tuple>

#include <map> //For node map types, TODO have a typedef somewhere for the nodes array
#include <boost/uuid/uuid.hpp>


namespace TrRouting
{
  class Point;
  class Node;
  class NodeTimeDistance;

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

    static std::vector<NodeTimeDistance> getAccessibleNodesFootpathsFromPoint(const Point &point, const std::map<boost::uuids::uuid, Node> &nodes, std::string mode, int maxWalkingTravelTime, float walkingSpeedMetersPerSecond, bool reversed = false);

  protected:
    static std::vector<NodeTimeDistance> getNodesFromBirdDistance(const Point &point, const std::map<boost::uuids::uuid, Node> &nodes, int maxWalkingTravelTime, float walkingSpeedMetersPerSecond);
    static std::vector<NodeTimeDistance> getNodesFromOsrm(const Point &point, const std::map<boost::uuids::uuid, Node> &nodes, std::string mode, int maxWalkingTravelTime, float walkingSpeedMetersPerSecond, bool reversed);

    static std::tuple<float, float> calculateLengthOfOneDegree(const Point &point);
    static float calculateMaxDistanceSquared(int maxWalkingTravelTime, float walkingSpeed);
    static float calculateNodeDistanceSquared(const Point *node, const Point &point, const std::tuple<float, float> &lengthOfOneDegree);
  };
}

#endif // TR_OSRM_FETCHER
