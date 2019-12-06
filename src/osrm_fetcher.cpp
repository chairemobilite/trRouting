#include "osrm_fetcher.hpp"
#include "json.hpp"

namespace TrRouting
{
  
  std::vector<std::tuple<int,int,int>> OsrmFetcher::getAccessibleNodesFootpathsFromPoint(const Point point, const std::vector<std::unique_ptr<Node>> &nodes, std::string mode, Parameters& params, bool reversed)
  {

    std::vector<int>                     birdDistanceAccessibleNodeIndexes;
    std::vector<std::tuple<int,int,int>> accessibleNodesFootpaths;

    float lengthOfOneDegreeOfLongitude = 111412.84 * cos(point.latitude * M_PI / 180) -93.5 * cos (3 * point.latitude * M_PI / 180);
    float lengthOfOneDegreeOflatitude  = 111132.92 - 559.82 * cos(2 * point.latitude * M_PI / 180) + 1.175 * cos(4 * point.latitude * M_PI / 180);
    float maxDistanceMetersSquared     = (params.maxAccessWalkingTravelTimeSeconds * params.walkingSpeedMetersPerSecond) * (params.maxAccessWalkingTravelTimeSeconds * params.walkingSpeedMetersPerSecond);
    float distanceMetersSquared;
    float distanceXMeters;
    float distanceYMeters;

    if (params.osrmWalkingUseLib)
    {
      osrm::TableParameters osrmParams{};

      osrmParams.coordinates.push_back({osrm::util::FloatLongitude{point.longitude}, osrm::util::FloatLatitude{point.latitude}});
      int i {0};
      for (auto & node : nodes)
      {
        distanceXMeters       = (node->point.get()->longitude - point.longitude) * lengthOfOneDegreeOfLongitude;
        distanceYMeters       = (node->point.get()->latitude  - point.latitude)  * lengthOfOneDegreeOflatitude ;
        distanceMetersSquared = distanceXMeters * distanceXMeters + distanceYMeters * distanceYMeters;
        if (distanceMetersSquared <= maxDistanceMetersSquared)
        {
          birdDistanceAccessibleNodeIndexes.push_back(i);
          osrmParams.coordinates.push_back({osrm::util::FloatLongitude{node->point.get()->longitude}, osrm::util::FloatLatitude{node->point.get()->latitude}});
        }
        i++;
      }
      //osrmParams.annotations.push_back("durations"); // not sure it works like this, needs more testing...
      //osrmParams.annotations.push_back("distances"); // not sure it works like this, needs more testing...
      if (reversed)
      {
        osrmParams.destinations.push_back(0);
      }
      else
      {
        osrmParams.sources.push_back(0);
      }

      osrm::json::Object result;
      
      const auto status = params.osrmWalkingRouter.get().Table(osrmParams, result);
      //std::cerr << "numberOfNodes: " << osrmParams.coordinates.size() << std::endl;
      if (status == osrm::Status::Ok)
      {
        if (reversed)
        {
          auto &durations = result.values["durations"].get<osrm::json::Array>().values;
          auto &distances = result.values["distances"].get<osrm::json::Array>().values.at(0).get<osrm::json::Array>().values;
          for (int i = 0; i < birdDistanceAccessibleNodeIndexes.size(); i++)
          {
            const int duration = durations.at(i+1).get<osrm::json::Array>().values.at(0).get<osrm::json::Number>().value; // ignore i = 0 (source with itself)
            const int distance = distances.at(i+1).get<osrm::json::Number>().value;
            if (duration <= params.maxAccessWalkingTravelTimeSeconds)
            {
              accessibleNodesFootpaths.push_back(std::make_tuple(birdDistanceAccessibleNodeIndexes[i], duration, distance));
            }
          }
        }
        else
        {
          auto &durations = result.values["durations"].get<osrm::json::Array>().values.at(0).get<osrm::json::Array>().values;
          auto &distances = result.values["distances"].get<osrm::json::Array>().values.at(0).get<osrm::json::Array>().values;
          for (int i = 0; i < birdDistanceAccessibleNodeIndexes.size(); i++)
          {
            const int duration = durations.at(i+1).get<osrm::json::Number>().value; // ignore i = 0 (source with itself)
            const int distance = distances.at(i+1).get<osrm::json::Number>().value;
            if (duration <= params.maxAccessWalkingTravelTimeSeconds)
            {
              accessibleNodesFootpaths.push_back(std::make_tuple(birdDistanceAccessibleNodeIndexes[i], duration, distance));
            }
          }
        }
      }
      else if (status == osrm::Status::Error)
      {
          const auto code = result.values["code"].get<osrm::json::String>().value;
          const auto message = result.values["message"].get<osrm::json::String>().value;

          std::cerr << "OSRM routing failed with code: " << code << std::endl;
          std::cerr << "Error message: " << code << std::endl;
      }

    }
    else
    {
   
      if (params.debugDisplay)
        std::cout << "osrm with host " << params.osrmWalkingHost << " and port " << params.osrmWalkingPort << std::endl;
      
      //std::cout << "mode = " << mode << " speed = " << defaultSpeedMetersPerSecond << " maxTravelTime = " << maxTravelTimeSeconds << " port = " << osrmPort << " host = " << osrmHost << " " << std::endl;
      std::string queryString = "GET /table/v1/" + mode + "/" + std::to_string(point.longitude) +  "," + std::to_string(point.latitude);

      int i {0};
      for (auto & node : nodes)
      {
        //std::cerr << node.point.latitude << std::endl;
        distanceXMeters       = (node->point.get()->longitude - point.longitude) * lengthOfOneDegreeOfLongitude;
        distanceYMeters       = (node->point.get()->latitude  - point.latitude)  * lengthOfOneDegreeOflatitude ;
        distanceMetersSquared = distanceXMeters * distanceXMeters + distanceYMeters * distanceYMeters;
        if (distanceMetersSquared <= maxDistanceMetersSquared)
        {
          birdDistanceAccessibleNodeIndexes.push_back(i);
          queryString += ";" + std::to_string(node->point.get()->longitude) +  "," + std::to_string(node->point.get()->latitude);
        }
        i++;
      }

      queryString += "?annotations=duration,distance";

      // call osrm on bird distance accessible nodes for further filtering by network travel time:
      boost::asio::ip::tcp::iostream s;

      s.connect(params.osrmWalkingHost, params.osrmWalkingPort);
      
      if (reversed)
      {
        queryString += "&destinations=0";
      }
      else
      {
        queryString += "&sources=0";
      }
      queryString += " HTTP/1.1\r\n\r\n";

      // std::cerr << queryString << std::endl;

      s << queryString;

      std::string header;
      while (std::getline(s, header) && header != "\r"){} // ignore first line

      std::stringstream responseJsonSs;
      responseJsonSs << s.rdbuf();

      nlohmann::json responseJson = nlohmann::json::parse(responseJsonSs.str());
      int numberOfDurations = responseJson["durations"].size();
      //std::cout << "numberOfDurations: " << responseJson["durations"][0].dump(2) << std::endl;
      int numberOfDistances = responseJson["distances"].size();
      //std::cout << "numberOfDistances: " << responseJson["distances"][0].dump(2) << std::endl;
      
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
            if (travelTimeSeconds <= params.maxAccessWalkingTravelTimeSeconds)
            {
              distanceMeters = (int)ceil((float)responseJson["distances"][0][i]);
              accessibleNodesFootpaths.push_back(std::make_tuple(birdDistanceAccessibleNodeIndexes[i-1], travelTimeSeconds, distanceMeters));
            }
          }
        }
      }
    }

    if (params.debugDisplay)
        std::cout << "fetched osrm footpaths (" << accessibleNodesFootpaths.size() << " foopaths found)" << std::endl;
    
    return accessibleNodesFootpaths;
  }
   
}
