#ifndef TR_OSRM_FETCHER
#define TR_OSRM_FETCHER

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <exception>
#include <iostream>
#include <string>
#include <utility>
#include <cstdlib>

#include "point.hpp"
#include "node.hpp"
#include "parameters.hpp"
#include "client_http.hpp"

namespace TrRouting
{
  class OsrmFetcher
  {

  public:
    OsrmFetcher() {}
    OsrmFetcher(std::string projectShortname)
    {
    }

    static std::vector<std::tuple<int, int, int>> getAccessibleNodesFootpathsFromPoint(const Point &point, const std::vector<std::unique_ptr<Node>> &nodes, std::string mode, ServerParameters &params, int maxWalkingTravelTime, float walkingSpeedMetersPerSecond, bool reversed = false);

  protected:
    static std::vector<std::tuple<int, int, int>> getNodesFromBirdDistance(const Point &point, const std::vector<std::unique_ptr<Node>> &nodes, ServerParameters &params, int maxWalkingTravelTime, float walkingSpeedMetersPerSecond);
    static std::vector<std::tuple<int, int, int>> getNodesFromOsrm(const Point &point, const std::vector<std::unique_ptr<Node>> &nodes, std::string mode, ServerParameters &params, int maxWalkingTravelTime, float walkingSpeedMetersPerSecond, bool reversed);

    static std::tuple<float, float> calculateLengthOfOneDegree(const Point &point);
    static float calculateMaxDistanceSquared(int maxWalkingTravelTime, float walkingSpeed);
    static float calculateNodeDistanceSquared(const Point *node, const Point &point, const std::tuple<float, float> &lengthOfOneDegree);
  };

}

#endif // TR_OSRM_FETCHER
