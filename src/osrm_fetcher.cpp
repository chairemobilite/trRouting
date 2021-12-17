#include "osrm_fetcher.hpp"
#include "json.hpp"

namespace TrRouting
{
  std::vector<std::tuple<int, int, int>> OsrmFetcher::getAccessibleNodesFootpathsFromPoint(const Point &point, const std::vector<std::unique_ptr<Node>> &nodes, std::string mode, Parameters &params, int maxWalkingTravelTime, float walkingSpeedMetersPerSecond, bool reversed)
  {
    if (params.birdDistanceAccessibilityEnabled)
    {
      return getNodesFromBirdDistance(point, nodes, params, maxWalkingTravelTime, walkingSpeedMetersPerSecond);
    }
    else
    {
      return getNodesFromOsrm(point, nodes, mode, params, maxWalkingTravelTime, walkingSpeedMetersPerSecond, reversed);
    }
  }

  std::vector<std::tuple<int, int, int>> OsrmFetcher::getNodesFromBirdDistance(const Point &point, const std::vector<std::unique_ptr<Node>> &nodes, Parameters &params, int maxWalkingTravelTime, float walkingSpeedMetersPerSecond)
  {
    std::vector<std::tuple<int, int, int>> accessibleNodesFootpaths;

    auto lengthOfOneDegree = calculateLengthOfOneDegree(point);
    float maxDistanceMetersSquared = calculateMaxDistanceSquared(maxWalkingTravelTime, walkingSpeedMetersPerSecond);
    float distanceMetersSquared;

    if (params.debugDisplay)
      std::cout << "use of bird distance " << std::endl;

    int i{0};
    for (auto &node : nodes)
    {
      distanceMetersSquared = calculateNodeDistanceSquared(node->point.get(), point, lengthOfOneDegree);

      if (distanceMetersSquared <= maxDistanceMetersSquared)
      {
        int distanceMeters = sqrt(distanceMetersSquared);
        int travelTimeSeconds = distanceMeters / walkingSpeedMetersPerSecond;
        accessibleNodesFootpaths.push_back(std::make_tuple(i, travelTimeSeconds, distanceMeters));
      }
      i++;
    }

    if (params.debugDisplay)
      std::cout << "fetched footpaths using bird distance (" << accessibleNodesFootpaths.size() << " foopaths found)" << std::endl;

    return accessibleNodesFootpaths;
  }

  std::vector<std::tuple<int, int, int>> OsrmFetcher::getNodesFromOsrm(const Point &point, const std::vector<std::unique_ptr<Node>> &nodes, std::string mode, Parameters &params, int maxWalkingTravelTime, float walkingSpeedMetersPerSecond, bool reversed)
  {
    std::vector<int> birdDistanceAccessibleNodeIndexes;
    std::vector<std::tuple<int, int, int>> accessibleNodesFootpaths;

    auto lengthOfOneDegree = calculateLengthOfOneDegree(point);
    float maxDistanceMetersSquared = calculateMaxDistanceSquared(maxWalkingTravelTime, walkingSpeedMetersPerSecond);
    float distanceMetersSquared;

    if (params.debugDisplay)
      std::cout << "osrm with host " << params.osrmWalkingHost << " and port " << params.osrmWalkingPort << std::endl;

    std::string queryString = "/table/v1/" + mode + "/" + std::to_string(point.longitude) + "," + std::to_string(point.latitude);

    int i{0};
    for (auto &node : nodes)
    {
      distanceMetersSquared = calculateNodeDistanceSquared(node->point.get(), point, lengthOfOneDegree);

      if (distanceMetersSquared <= maxDistanceMetersSquared)
      {
        birdDistanceAccessibleNodeIndexes.push_back(i);
        queryString += ";" + std::to_string(node->point.get()->longitude) + "," + std::to_string(node->point.get()->latitude);
      }
      i++;
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
    HttpClient client(params.osrmWalkingHost + ":" + params.osrmWalkingPort);
    auto s = client.request("GET", queryString);

    std::stringstream responseJsonSs;
    responseJsonSs << s->content.rdbuf();
    nlohmann::json responseJson = nlohmann::json::parse(responseJsonSs.str());

    if (responseJson["durations"] != nullptr && responseJson["distances"] != nullptr && responseJson["durations"][0] != nullptr && responseJson["distances"][0] != nullptr)
    {
      int numberOfDurations = responseJson["durations"][0].size();
      //std::cout << "numberOfDurations: " << responseJson["durations"][0].dump(2) << std::endl;
      int numberOfDistances = responseJson["distances"][0].size();
      //std::cout << "numberOfDistances: " << responseJson["distances"][0].dump(2) << std::endl;
      int j = 0;
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
            accessibleNodesFootpaths.push_back(std::make_tuple(birdDistanceAccessibleNodeIndexes[i - 1], travelTimeSeconds, distanceMeters));
          }
        }
      }
    }

    if (params.debugDisplay)
      std::cout << "fetched osrm footpaths (" << accessibleNodesFootpaths.size() << " foopaths found)" << std::endl;

    return accessibleNodesFootpaths;
  }

  std::tuple<float, float> OsrmFetcher::calculateLengthOfOneDegree(const Point &point)
  {
    // Taylor series approximation of the longitude and latitude length functions (For more info: https://gis.stackexchange.com/questions/75528/understanding-terms-in-length-of-degree-formula)
    float lengthOfOneDegreeOfLongitude = 111412.84 * cos(point.latitude * M_PI / 180) - 93.5 * cos(3 * point.latitude * M_PI / 180);
    float lengthOfOneDegreeOflatitude = 111132.92 - 559.82 * cos(2 * point.latitude * M_PI / 180) + 1.175 * cos(4 * point.latitude * M_PI / 180);

    return std::make_tuple(lengthOfOneDegreeOfLongitude, lengthOfOneDegreeOflatitude);
  }

  float OsrmFetcher::calculateMaxDistanceSquared(int maxWalkingTravelTime, float walkingSpeedMetersPerSecond)
  {
    return (maxWalkingTravelTime * walkingSpeedMetersPerSecond) * (maxWalkingTravelTime * walkingSpeedMetersPerSecond);
  }

  float OsrmFetcher::calculateNodeDistanceSquared(const Point *node, const Point &point, const std::tuple<float, float> &lengthOfOneDegree)
  {
    float distanceXMeters = (node->longitude - point.longitude) * std::get<0>(lengthOfOneDegree);
    float distanceYMeters = (node->latitude - point.latitude) * std::get<1>(lengthOfOneDegree);
    float distanceMetersSquared = distanceXMeters * distanceXMeters + distanceYMeters * distanceYMeters;

    return distanceMetersSquared;
  }

}
