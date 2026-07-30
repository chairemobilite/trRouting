#include <string>
#include <unordered_map>
#include <vector>

#include "osrmgeofilter.hpp"
#include <nlohmann/json.hpp>
#include "point.hpp"
#include "node.hpp"
#include "client_http.hpp"
#include "spdlog/spdlog.h"

namespace TrRouting {

  namespace {
    /* A node that passed the bird distance filter, paired with the column it
       maps to in the OSRM table response. Nodes sharing coordinates share a
       column, so this is not simply the node's rank in the request. */
    struct PotentialAccessibleNode {
      std::reference_wrapper<const Node> node;
      std::size_t osrmPosition;
    };
    
    // By default, to_string convert a double to a string with 6 decimal digits
    // TODO Add a unit test to ensure the formatting always have the right precisions (default might change in future standards)
    std::string formatOsrmCoordinates(const Point &point) {
      return std::to_string(point.longitude) + "," + std::to_string(point.latitude);
    }
  }

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
    // All the nodes within bird distance,
    // each carrying the OSRM response column it should be read from.
    std::vector<PotentialAccessibleNode> potentialAccessibleNodes;
    // Coordinate string (exactly as sent to OSRM) -> column in the response.
    // Keying on the formatted string rather than the doubles means two nodes
    // whose coordinates differ below the 6 decimals we transmit still collapse
    // to one destination, which is what OSRM would have received anyway.
    std::unordered_map<std::string, std::size_t> osrmPositionByCoordinates;
    // Column 0 of the response is the origin point, so node columns start at 1
    std::size_t nextOsrmPosition = 1;

    std::vector<NodeTimeDistance> accessibleNodesFootpaths;

    auto lengthOfOneDegree = calculateLengthOfOneDegree(point);
    float maxDistanceMetersSquared = calculateMaxDistanceSquared(maxWalkingTravelTime, walkingSpeedMetersPerSecond);
    float distanceMetersSquared;

    spdlog::debug("osrm with host {} and port {}", host, port);

    std::string queryString = "/table/v1/" + mode + "/" + formatOsrmCoordinates(point);

    // We first filter the nodes with euclidean distance using the common distance calculation
    // to only send a subset of nodes to OSRM. We do not reuse the EuclideanGeoFilter directly, since
    // we process the data differently here. (We directly compute the OSRM query.)
    for (auto &&[uuid,node] : nodes)
    {
      distanceMetersSquared = calculateNodeDistanceSquared(node.point.get(), point, lengthOfOneDegree);

      if (distanceMetersSquared <= maxDistanceMetersSquared)
      {
        std::string coordinates = formatOsrmCoordinates(*node.point.get());

        auto inserted = osrmPositionByCoordinates.emplace(coordinates, nextOsrmPosition);
        // The second element of the returned pair of emplace indicate if a new element needed to be inserted,
        // so a true flag means it was a new element.
        if (inserted.second)
        {
          // First node seen at these coordinates: it gets its own destination
          queryString += ";" + coordinates;
          nextOsrmPosition++;
        }

        potentialAccessibleNodes.push_back({std::cref(node), inserted.first->second});
      }
    }

    // If we don't have any node accessible with the euclidean distance, don't bother calculating
    // the exact distance with OSRM
    if (potentialAccessibleNodes.size() == 0) {
      // Return the empty vector
      spdlog::debug("There was no node potentially accessible, we did not ask OSRM");
      return accessibleNodesFootpaths;
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

    nlohmann::json responseJson;
    try {
      using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;
      HttpClient client(host + ":" + port);
      auto s = client.request("GET", queryString);

      if (s->status_code != "200 OK") {
        spdlog::error("Error fetching OSRM data ({})", s->status_code);
        //TODO We should throw an exception somehow here to invalidate the current calculation
        // and returne an informative error code to the user
        return accessibleNodesFootpaths;
      }

      responseJson = nlohmann::json::parse(s->content);
    } catch (const std::exception& e){
      spdlog::error("exception during OSRM request or response parsing: : {}", e.what());
      //TODO See above TODO about handling the errors
      return accessibleNodesFootpaths;
    }

    if (responseJson["durations"] != nullptr && responseJson["distances"] != nullptr && responseJson["durations"][0] != nullptr && responseJson["distances"][0] != nullptr)
    {
      std::size_t numberOfDurations = responseJson["durations"][0].size();
      std::size_t numberOfDistances = responseJson["distances"][0].size();

      // We asked for the origin plus one destination per distinct coordinate.
      // Anything else means we cannot trust the column mapping, so bail out
      // rather than read the wrong walking times.
      if (numberOfDurations != nextOsrmPosition || numberOfDistances != nextOsrmPosition)
      {
        spdlog::error("OSRM returned {} durations and {} distances, expected {}; ignoring the response",
                      numberOfDurations, numberOfDistances, nextOsrmPosition);
        return accessibleNodesFootpaths;
      }

      int travelTimeSeconds;
      int distanceMeters;
      // Iterate over the nodes rather than over the response columns, since
      // co-located nodes share a column
      for (const auto & potentialNode : potentialAccessibleNodes)
      {
        std::size_t position = potentialNode.osrmPosition;

        // Check if the duration and distance values are null before attempting to convert them
        if (!responseJson["durations"][0][position].is_null() && !responseJson["distances"][0][position].is_null())
        {
          travelTimeSeconds = (int)ceil((float)responseJson["durations"][0][position]);
          if (travelTimeSeconds <= maxWalkingTravelTime)
          {
            distanceMeters = (int)ceil((float)responseJson["distances"][0][position]);
            accessibleNodesFootpaths.push_back(NodeTimeDistance(potentialNode.node,
                                                              travelTimeSeconds,
                                                              distanceMeters));
          }
        }
        else
        {
          spdlog::debug("skipping node at index {} due to null duration or distance from OSRM", position);
        }
      }
    }

    spdlog::debug("fetched osrm footpaths ({} footpaths found, {} nodes sent as {} distinct osrm destinations)",
                  accessibleNodesFootpaths.size(), potentialAccessibleNodes.size(), nextOsrmPosition - 1);

    return accessibleNodesFootpaths;
  }
  
}
