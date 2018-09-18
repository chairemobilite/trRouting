#include "osrm_fetcher.hpp"

namespace TrRouting
{
  
  std::vector<std::pair<int,int>> OsrmFetcher::getAccessibleStopsFootpathsFromPoint(const Point point, const std::vector<Stop> stops, std::string mode, Parameters& params)
  {

    std::vector<int>                birdDistanceAccessibleStopIndexes;
    std::vector<std::pair<int,int>> accessibleStopsFootpaths;

    float lengthOfOneDegreeOfLongitude = 111412.84 * cos(point.latitude * M_PI / 180) -93.5 * cos (3 * point.latitude * M_PI / 180);
    float lengthOfOneDegreeOflatitude  = 111132.92 - 559.82 * cos(2 * point.latitude * M_PI / 180) + 1.175 * cos(4 * point.latitude * M_PI / 180);
    float maxDistanceMetersSquared     = (params.maxAccessWalkingTravelTimeSeconds * params.walkingSpeedMetersPerSecond) * (params.maxAccessWalkingTravelTimeSeconds * params.walkingSpeedMetersPerSecond);
    float distanceMetersSquared;
    float distanceXMeters;
    float distanceYMeters;

    if (params.osrmUseLib)
    {
      //osrm::EngineConfig osrmConfig;
      //osrmConfig.storage_config    = {params.osrmFilePath};
      //osrmConfig.use_shared_memory = false;
      //osrmConfig.algorithm         = osrm::EngineConfig::Algorithm::CH;
      //osrm::OSRM osrmRouter2{osrmConfig};
      osrm::TableParameters osrmParams{};

      osrmParams.coordinates.push_back({osrm::util::FloatLongitude{point.longitude}, osrm::util::FloatLatitude{point.latitude}});
      int i {0};
      for (auto & stop : stops)
      {
        distanceXMeters       = (stop.point.longitude - point.longitude) * lengthOfOneDegreeOfLongitude;
        distanceYMeters       = (stop.point.latitude  - point.latitude)  * lengthOfOneDegreeOflatitude ;
        distanceMetersSquared = distanceXMeters * distanceXMeters + distanceYMeters * distanceYMeters;
        if (distanceMetersSquared <= maxDistanceMetersSquared)
        {
          birdDistanceAccessibleStopIndexes.push_back(i);
          osrmParams.coordinates.push_back({osrm::util::FloatLongitude{stop.point.longitude}, osrm::util::FloatLatitude{stop.point.latitude}});
        }
        i++;
      }
      osrmParams.sources.push_back(0);
      osrm::json::Object result;
      
      const auto status = params.osrmRouter.get().Table(osrmParams, result);
      //std::cerr << "numberOfStops: " << osrmParams.coordinates.size() << std::endl;
      if (status == osrm::Status::Ok)
      {
        auto &durations = result.values["durations"].get<osrm::json::Array>().values.at(0).get<osrm::json::Array>().values;
        //auto &distances = result.values["distances"].get<osrm::json::Array>().values.at(0).get<osrm::json::Array>().values;
        for (int i = 0; i < birdDistanceAccessibleStopIndexes.size(); i++)
        {
          const int duration = durations.at(i+1).get<osrm::json::Number>().value; // ignore i = 0 (source with itself)
          //const int distance = distances.at(i+1).get<osrm::json::Number>().value;
          if (duration <= params.maxAccessWalkingTravelTimeSeconds)
          {
            accessibleStopsFootpaths.push_back(std::make_pair(birdDistanceAccessibleStopIndexes[i], duration));
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
   
      //std::cout << "mode = " << mode << " speed = " << defaultSpeedMetersPerSecond << " maxTravelTime = " << maxTravelTimeSeconds << " port = " << osrmPort << " host = " << osrmHost << " " << std::endl;
      std::string queryString = "GET /table/v1/" + mode + "/" + std::to_string(point.longitude) +  "," + std::to_string(point.latitude);

      int i {0};
      for (auto & stop : stops)
      {
        //std::cerr << stop.point.latitude << std::endl;
        distanceXMeters       = (stop.point.longitude - point.longitude) * lengthOfOneDegreeOfLongitude;
        distanceYMeters       = (stop.point.latitude  - point.latitude)  * lengthOfOneDegreeOflatitude ;
        distanceMetersSquared = distanceXMeters * distanceXMeters + distanceYMeters * distanceYMeters;
        if (distanceMetersSquared <= maxDistanceMetersSquared)
        {
          birdDistanceAccessibleStopIndexes.push_back(i);
          queryString += ";" + std::to_string(stop.point.longitude) +  "," + std::to_string(stop.point.latitude);
        }
        i++;
      }

      // call osrm on bird distance accessible stops for further filtering by network travel time:
      boost::asio::ip::tcp::iostream s;
      s.connect(params.osrmRoutingWalkingHost, params.osrmRoutingWalkingPort);
      queryString += "?sources=0";
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
                accessibleStopsFootpaths.push_back(std::make_pair(birdDistanceAccessibleStopIndexes[i], travelTimeSeconds));
              }
            }
            i++;
          }
        }
      }
    }
    
    return accessibleStopsFootpaths;
  }
   
}
