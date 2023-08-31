#include "osrmgeofilter.hpp"
#include "json.hpp"
#include "point.hpp"
#include "node.hpp"
#include "client_http.hpp"
#include "spdlog/spdlog.h"

namespace TrRouting {

  OsrmGeoFilter::OsrmGeoFilter(const std::string &amode, const std::string &ahost, const std::string &aport) :
    mode(amode),
    host(ahost),
    port(aport)
  {
  }

  std::vector<NodeTimeDistance> OsrmGeoFilter::getAccessibleNodesFootpathsFromPoint(const Point &point,
                                                                                    const std::map<boost::uuids::uuid, Node> &nodes,
                                                                                    int maxWalkingTravelTime,
                                                                                    float walkingSpeedMetersPerSecond,
                                                                                    bool reversed)
  {
    std::vector<std::reference_wrapper<const Node>> birdDistanceAccessibleNodeIndexes;
    std::vector<NodeTimeDistance> accessibleNodesFootpaths;

    auto lengthOfOneDegree = calculateLengthOfOneDegree(point);
    float maxDistanceMetersSquared = calculateMaxDistanceSquared(maxWalkingTravelTime, walkingSpeedMetersPerSecond);
    float distanceMetersSquared;

    spdlog::debug("osrm with host {} and port {}", host, port);

    std::string queryString = "/table/v1/" + mode + "/" + std::to_string(point.longitude) + "," + std::to_string(point.latitude);

    for (auto &&[uuid,node] : nodes)
    {
      distanceMetersSquared = calculateNodeDistanceSquared(node.point.get(), point, lengthOfOneDegree);

      if (distanceMetersSquared <= maxDistanceMetersSquared)
      {
        birdDistanceAccessibleNodeIndexes.push_back(node);
        queryString += ";" + std::to_string(node.point.get()->longitude) + "," + std::to_string(node.point.get()->latitude);
      }
    }

    queryString += "?annotations=duration,distance";

    if (reversed)
    {
      queryString += "&destinations=0";
    }
    else
    {
      queryString += "&sources=0";
    }

    using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;
    HttpClient client(host + ":" + port);
    auto s = client.request("GET", queryString);

    if (s->status_code != "200 OK") {
      spdlog::error("Error fetching OSRM data ({})", s->status_code);
      //TODO We should throw an exception somehow here to invalidate the current calculation
      // and returne an informative error code to the user
      return accessibleNodesFootpaths;
    }
    
    std::stringstream responseJsonSs;
    responseJsonSs << s->content.rdbuf();
    nlohmann::json responseJson = nlohmann::json::parse(responseJsonSs.str());

    if (responseJson["durations"] != nullptr && responseJson["distances"] != nullptr && responseJson["durations"][0] != nullptr && responseJson["distances"][0] != nullptr)
    {
      int numberOfDurations = responseJson["durations"][0].size();
      int numberOfDistances = responseJson["distances"][0].size();

      if (numberOfDurations > 0 && numberOfDistances > 0)
      {
        int travelTimeSeconds;
        int distanceMeters;
        for (int i = 1; i < numberOfDurations; i++) // ignore first (duration with itself)
        {
          travelTimeSeconds = (int)ceil((float)responseJson["durations"][0][i]);
          if (travelTimeSeconds <= maxWalkingTravelTime)
          {
            distanceMeters = (int)ceil((float)responseJson["distances"][0][i]);
            accessibleNodesFootpaths.push_back(NodeTimeDistance(birdDistanceAccessibleNodeIndexes[i - 1],
                                                                travelTimeSeconds,
                                                                distanceMeters));
          }
        }
      }
    }

    spdlog::debug("fetched osrm footpaths ({} footpaths found)",  accessibleNodesFootpaths.size());

    return accessibleNodesFootpaths;
  }
  
}
