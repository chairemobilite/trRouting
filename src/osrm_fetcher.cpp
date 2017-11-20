#include "osrm_fetcher.hpp"

namespace TrRouting
{
  
  std::vector<std::pair<int,int>> OsrmFetcher::getAccessibleStopsFootpathsFromPoint(const Point point, const std::vector<Stop> stops, std::string mode, int maxTravelTimeSeconds, float defaultSpeedMetersPerSecond, std::string osrmHost, std::string osrmPort)
  {
    
    std::vector<int>                birdDistanceAccessibleStopIndexes;
    std::vector<std::pair<int,int>> accessibleStopsFootpaths;
    
    //std::cout << "mode = " << mode << " speed = " << defaultSpeedMetersPerSecond << " maxTravelTime = " << maxTravelTimeSeconds << " port = " << osrmPort << " host = " << osrmHost << " " << std::endl;
    
    std::cout << std::fixed;
    std::cout << std::setprecision(6);
    
    std::string queryString = "GET /table/v1/" + mode + "/" + std::to_string(point.longitude) +  "," + std::to_string(point.latitude);
    
    float lengthOfOneDegreeOfLongitude = 111412.84 * cos(point.latitude * M_PI / 180) -93.5 * cos (3 * point.latitude * M_PI / 180);
    float lengthOfOneDegreeOflatitude  = 111132.92 - 559.82 * cos(2 * point.latitude * M_PI / 180) + 1.175 * cos(4 * point.latitude * M_PI / 180);
    float maxDistanceMetersSquared     = (maxTravelTimeSeconds * defaultSpeedMetersPerSecond) * (maxTravelTimeSeconds * defaultSpeedMetersPerSecond);
    float distanceMetersSquared;
    float distanceXMeters;
    float distanceYMeters;
    Point stopPoint;
    
    int i {0};
    for (auto & stop : stops)
    {
      stopPoint             = stop.point;
      distanceXMeters       = (stopPoint.longitude - point.longitude) * lengthOfOneDegreeOfLongitude;
      distanceYMeters       = (stopPoint.latitude  - point.latitude)  * lengthOfOneDegreeOflatitude ;
      distanceMetersSquared = distanceXMeters * distanceXMeters + distanceYMeters * distanceYMeters;
      //std::cerr << distanceMeters;
      if (distanceMetersSquared <= maxDistanceMetersSquared)
      {
        birdDistanceAccessibleStopIndexes.push_back(i);
        queryString += ";" + std::to_string(stopPoint.longitude) +  "," + std::to_string(stopPoint.latitude);
      }
      i++;
    }
    
    
    // call osrm on bird distance accessible stops for further filtering by network travel time:
    boost::asio::ip::tcp::iostream s;
    s.connect(osrmHost, osrmPort);
    queryString += "?sources=0";
    queryString += " HTTP/1.1\r\n\r\n";
        
    s << queryString;
    
    std::string header;
    while (std::getline(s, header) && header != "\r"){} // ignore first line
    
    std::stringstream responseJsonSs;
    responseJsonSs << s.rdbuf();
    
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(responseJsonSs, pt);
    
    using boost::property_tree::ptree;
        
    //std::cout << "duration count = " << pt.count("durations") << std::endl;
    
    i = -1;
    
    if (pt.count("durations") == 1)
    {
      ptree durations = pt.get_child("durations");
    
      int travelTimeSeconds;
      
      for (const auto& v : durations) {
        for (const auto& v2 : v.second) {
          if(i >= 0) // first value is duration with starting stop, we must ignore it (i was initialized with -1)
          {
            travelTimeSeconds = (int)ceil(std::stod(v2.second.data()));
            if (travelTimeSeconds <= maxTravelTimeSeconds)
            {
              accessibleStopsFootpaths.push_back(std::make_pair(birdDistanceAccessibleStopIndexes[i], travelTimeSeconds));
            }
          }
          i++;
        }
      }
    }
    
    return accessibleStopsFootpaths;
  }
   
}
