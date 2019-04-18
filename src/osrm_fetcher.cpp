#include "osrm_fetcher.hpp"

namespace TrRouting
{
  
  std::vector<std::pair<int,int>> OsrmFetcher::getAccessibleNodesFootpathsFromPoint(const Point point, const std::vector<Node> nodes, std::string mode, Parameters& params, bool reversed)
  {

    std::vector<int>                birdDistanceAccessibleNodeIndexes;
    std::vector<std::pair<int,int>> accessibleNodesFootpaths;

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
        distanceXMeters       = (node.point.longitude - point.longitude) * lengthOfOneDegreeOfLongitude;
        distanceYMeters       = (node.point.latitude  - point.latitude)  * lengthOfOneDegreeOflatitude ;
        distanceMetersSquared = distanceXMeters * distanceXMeters + distanceYMeters * distanceYMeters;
        if (distanceMetersSquared <= maxDistanceMetersSquared)
        {
          birdDistanceAccessibleNodeIndexes.push_back(i);
          osrmParams.coordinates.push_back({osrm::util::FloatLongitude{node.point.longitude}, osrm::util::FloatLatitude{node.point.latitude}});
        }
        i++;
      }
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
          //auto &distances = result.values["distances"].get<osrm::json::Array>().values.at(0).get<osrm::json::Array>().values;
          for (int i = 0; i < birdDistanceAccessibleNodeIndexes.size(); i++)
          {
            const int duration = durations.at(i+1).get<osrm::json::Array>().values.at(0).get<osrm::json::Number>().value; // ignore i = 0 (source with itself)
            //const int distance = distances.at(i+1).get<osrm::json::Number>().value;
            if (duration <= params.maxAccessWalkingTravelTimeSeconds)
            {
              accessibleNodesFootpaths.push_back(std::make_pair(birdDistanceAccessibleNodeIndexes[i], duration));
            }
          }
        }
        else
        {
          auto &durations = result.values["durations"].get<osrm::json::Array>().values.at(0).get<osrm::json::Array>().values;
          //auto &distances = result.values["distances"].get<osrm::json::Array>().values.at(0).get<osrm::json::Array>().values;
          for (int i = 0; i < birdDistanceAccessibleNodeIndexes.size(); i++)
          {
            const int duration = durations.at(i+1).get<osrm::json::Number>().value; // ignore i = 0 (source with itself)
            //const int distance = distances.at(i+1).get<osrm::json::Number>().value;
            if (duration <= params.maxAccessWalkingTravelTimeSeconds)
            {
              accessibleNodesFootpaths.push_back(std::make_pair(birdDistanceAccessibleNodeIndexes[i], duration));
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
        distanceXMeters       = (node.point.longitude - point.longitude) * lengthOfOneDegreeOfLongitude;
        distanceYMeters       = (node.point.latitude  - point.latitude)  * lengthOfOneDegreeOflatitude ;
        distanceMetersSquared = distanceXMeters * distanceXMeters + distanceYMeters * distanceYMeters;
        if (distanceMetersSquared <= maxDistanceMetersSquared)
        {
          birdDistanceAccessibleNodeIndexes.push_back(i);
          queryString += ";" + std::to_string(node.point.longitude) +  "," + std::to_string(node.point.latitude);
        }
        i++;
      }

      // call osrm on bird distance accessible nodes for further filtering by network travel time:
      boost::asio::ip::tcp::iostream s;

      s.connect(params.osrmWalkingHost, params.osrmWalkingPort);
      
      if (reversed)
      {
        queryString += "?destinations=0";
      }
      else
      {
        queryString += "?sources=0";
      }
      queryString += " HTTP/1.1\r\n\r\n";

      // std::cerr << queryString << std::endl;

      s << queryString;

      std::string header;
      while (std::getline(s, header) && header != "\r"){} // ignore first line

      std::stringstream responseJsonSs;
      responseJsonSs << s.rdbuf();

      boost::property_tree::ptree pt;
      boost::property_tree::read_json(responseJsonSs, pt);

      //std::cerr << responseJsonSs.str() << std::endl;

      using boost::property_tree::ptree;

      //std::cout << "duration count = " << pt.count("durations") << std::endl;

      i = -1;

      if (pt.count("durations") == 1)
      {
        ptree durations = pt.get_child("durations");

        int travelTimeSeconds;

        for (const auto& v : durations) {
          for (const auto& v2 : v.second) {
            if(i >= 0) // first value is duration with self, we must ignore it (i was initialized with -1)
            {
              travelTimeSeconds = (int)ceil(std::stod(v2.second.data()));
              if (travelTimeSeconds <= params.maxAccessWalkingTravelTimeSeconds)
              {
                accessibleNodesFootpaths.push_back(std::make_pair(birdDistanceAccessibleNodeIndexes[i], travelTimeSeconds));
              }
            }
            i++;
          }
        }
      }
    }
    
    return accessibleNodesFootpaths;
  }
   
}
