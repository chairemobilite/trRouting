#include "trip_based_algorithm.hpp"

using json = nlohmann::json;

namespace TrRouting
{
  
  TripBasedAlgorithm::TripBasedAlgorithm(Parameters& theParams) : params(theParams)
  {
    setup();
  }
  
  TripBasedAlgorithm::TripBasedAlgorithm()
  {
    
  }
  
  void TripBasedAlgorithm::resetAccessEgressModes()
  {
    accessMode = params.accessMode;
    egressMode = params.egressMode;
    maxAccessWalkingTravelTimeFromOriginToFirstStopMinutes     = params.maxAccessWalkingTravelTimeFromOriginToFirstStopMinutes;
    maxAccessWalkingTravelTimeFromLastStopToDestinationMinutes = params.maxAccessWalkingTravelTimeFromLastStopToDestinationMinutes;
  }
  
  // call setup only once when starting the calculator. Use updateParams before each calculation.
  void TripBasedAlgorithm::setup()
  {
    params.setDefaultValues();
    setParamsFromYaml("trRoutingTripBasedConfig.yml");
    
    
    std::string weekdayName {"sunday"};
    std::string dataName;
    std::ifstream stream;
    std::vector<uint8_t> contents;
    json jsonContent;
    json::basic_json jsonData;
    Footpath* footpath;
    RoutePath* routePath;
    Trip* trip;
    int i;
    
    // fetch footpaths_by_source:
    dataName = "footpaths_by_source";
    stream   = std::ifstream("cache/" + params.applicationShortname + "__trip_based_routing__" + dataName + ".msgpack", std::ios::in | std::ios::binary);
    contents = std::vector<uint8_t>((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    jsonContent = json::from_msgpack(contents);
    footpathsBySource = std::vector<Footpath>();
    footpathsBySource.reserve(jsonContent.size());
    footpath = new Footpath();
    for (json::iterator it = jsonContent.begin(); it != jsonContent.end(); ++it) {
      jsonData = *it;
      footpath->i    = jsonData["i"].get<int>();
      footpath->srcI = jsonData["source_stop_i"].get<int>();
      footpath->tgtI = jsonData["target_stop_i"].get<int>();
      footpath->tt   = jsonData["tt"].get<int>();
      footpathsBySource.push_back(*footpath);
    }
    //std::cout << footpathsBySource[345].tt << std::endl;
    
    
    
    // fetch footpaths_by_target:
    dataName = "footpaths_by_target";
    stream   = std::ifstream("cache/" + params.applicationShortname + "__trip_based_routing__" + dataName + ".msgpack", std::ios::in | std::ios::binary);
    contents = std::vector<uint8_t>((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    jsonContent = json::from_msgpack(contents);
    footpathsByTarget = std::vector<Footpath>();
    footpathsByTarget.reserve(jsonContent.size());
    footpath = new Footpath();
    for (json::iterator it = jsonContent.begin(); it != jsonContent.end(); ++it) {
      jsonData = *it;
      footpath->i    = jsonData["i"].get<int>();
      footpath->srcI = jsonData["source_stop_i"].get<int>();
      footpath->tgtI = jsonData["target_stop_i"].get<int>();
      footpath->tt   = jsonData["tt"].get<int>();
      footpathsByTarget.push_back(*footpath);
    }
    //std::cout << footpathsByTarget[345].tt << std::endl;
    
    
    
    // fetch footpaths_index_by_source: Exact same copy as footpaths_index_by_target, so we don't need both!
    dataName = "footpaths_index_by_source";
    stream   = std::ifstream("cache/" + params.applicationShortname + "__trip_based_routing__" + dataName + ".msgpack", std::ios::in | std::ios::binary);
    contents = std::vector<uint8_t>((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    jsonContent = json::from_msgpack(contents);
    footpathsIndex = std::vector<std::vector<int> >(jsonContent.size(), std::vector<int>(2,-1)); // initialize all to [-1, -1]
    i = 0;
    for (json::iterator it = jsonContent.begin(); it != jsonContent.end(); ++it) {
      jsonData = *it;
      footpathsIndex[i][0] = jsonData[0].get<int>();
      footpathsIndex[i][1] = jsonData[1].get<int>();
      i++;
    }
    //std::cout << footpathsIndexBySource[345][0] << std::endl;
    
    
    
    // fetch route_paths:
    dataName = "route_paths";
    stream   = std::ifstream("cache/" + params.applicationShortname + "__trip_based_routing__" + dataName + ".msgpack", std::ios::in | std::ios::binary);
    contents = std::vector<uint8_t>((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    jsonContent = json::from_msgpack(contents);
    routePaths = std::vector<RoutePath>();
    routePaths.reserve(jsonContent.size());
    routePath = new RoutePath();
    for (json::iterator it = jsonContent.begin(); it != jsonContent.end(); ++it) {
      jsonData = *it;
      routePath->i   = jsonData["i"].get<int>();
      routePath->id  = jsonData["id"].get<long long>();
      routePath->rId = jsonData["route_id"].get<long long>();
      routePaths.push_back(*routePath);
    }
    //std::cout << routePaths[21].routeId << std::endl;
    
    
    
    // fetch route_paths_index_by_id
    dataName = "route_paths_index_by_id";
    stream   = std::ifstream("cache/" + params.applicationShortname + "__trip_based_routing__" + dataName + ".msgpack", std::ios::in | std::ios::binary);
    contents = std::vector<uint8_t>((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    jsonContent = json::from_msgpack(contents);
    routePathsIndexById = std::vector<int>(jsonContent.size(), -1); // initialize all to -1
    i = 0;
    for (json::iterator it = jsonContent.begin(); it != jsonContent.end(); ++it) {
      jsonData = *it;
      routePathsIndexById[i] = jsonData[0].get<int>();
      i++;
    }
    std::cout << jsonContent.dump() << std::endl;
    
    
    
    // fetch trips:
    dataName = "trips";
    stream   = std::ifstream("cache/" + params.applicationShortname + "__trip_based_routing__" + weekdayName + "__" + dataName + ".msgpack", std::ios::in | std::ios::binary);
    contents = std::vector<uint8_t>((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    jsonContent = json::from_msgpack(contents);
    trips = std::vector<Trip>();
    trips.reserve(jsonContent.size());
    trip = new Trip();
    for (json::iterator it = jsonContent.begin(); it != jsonContent.end(); ++it) {
      jsonData = *it;
      trip->i    = jsonData["i"].get<int>();
      trip->id   = jsonData["id"].get<long long>();
      trip->rpI  = jsonData["route_path_i"].get<int>();
      trip->seq  = jsonData["trip_seq"].get<int>();
      trips.push_back(*trip);
    }
    
  
    
    
  }    
  
  void TripBasedAlgorithm::refresh()
  {
    
  }
  
  // Call before each calculation
  void TripBasedAlgorithm::updateParams(Parameters& theParams)
  {
    params = theParams;
  }
  
  void TripBasedAlgorithm::setParamsFromYaml(std::string yamlFilePath)
  {
    
    if(yamlFilePath == "")
    {
      yamlFilePath = "trRoutingTripBasedConfig.yml";
    }
    
    // Override params using yaml config file:
    YAML::Node config = YAML::LoadFile(yamlFilePath);
    
    if (config["databaseName"])
    {
      params.databaseName = config["databaseName"].as<std::string>();
    }
    if (config["databaseHost"])
    {
      params.databaseHost = config["databaseHost"].as<std::string>();
    }
    if (config["databaseUser"])
    {
      params.databaseUser = config["databaseUser"].as<std::string>();
    }
    if (config["databasePort"])
    {
      params.databasePort = config["databasePort"].as<std::string>();
    }
    if (config["osrmRoutingWalkingPort"])
    {
      params.osrmRoutingWalkingPort = config["osrmRoutingWalkingPort"].as<std::string>();
    }
    if (config["osrmRoutingWalkingHost"])
    {
      params.osrmRoutingWalkingHost = config["osrmRoutingWalkingHost"].as<std::string>();
    }
    if (config["osrmRoutingDrivingPort"])
    {
      params.osrmRoutingDrivingPort = config["osrmRoutingDrivingPort"].as<std::string>();
    }
    if (config["osrmRoutingDrivingHost"])
    {
      params.osrmRoutingDrivingHost = config["osrmRoutingDrivingHost"].as<std::string>();
    }
    if (config["osrmRoutingCyclingPort"])
    {
      params.osrmRoutingCyclingPort = config["osrmRoutingCyclingPort"].as<std::string>();
    }
    if (config["osrmRoutingCyclingHost"])
    {
      params.osrmRoutingCyclingHost = config["osrmRoutingCyclingHost"].as<std::string>();
    }
    if (config["maxAccessWalkingTravelTimeFromOriginToFirstStopMinutes"])
    {
      params.maxAccessWalkingTravelTimeFromOriginToFirstStopMinutes = config["maxAccessWalkingTravelTimeFromOriginToFirstStopMinutes"].as<int>();
    }
    if (config["maxAccessWalkingTravelTimeFromLastStopToDestinationMinutes"])
    {
      params.maxAccessWalkingTravelTimeFromLastStopToDestinationMinutes = config["maxAccessWalkingTravelTimeFromLastStopToDestinationMinutes"].as<int>();
    }
    if (config["maxTransferWalkingTravelTimeMinutes"])
    {
      params.maxTransferWalkingTravelTimeMinutes = config["maxTransferWalkingTravelTimeMinutes"].as<int>();
    }
    if (config["minWaitingTimeMinutes"])
    {
      params.minWaitingTimeMinutes = config["minWaitingTimeMinutes"].as<int>();
    }
    if (config["walkingSpeedMetersPerSecond"])
    {
      params.walkingSpeedMetersPerSecond = config["walkingSpeedMetersPerSecond"].as<float>();
    }
    if (config["drivingSpeedMetersPerSecond"])
    {
      params.drivingSpeedMetersPerSecond = config["drivingSpeedMetersPerSecond"].as<float>();
    }
    if (config["cyclingSpeedMetersPerSecond"])
    {
      params.cyclingSpeedMetersPerSecond = config["cyclingSpeedMetersPerSecond"].as<float>();
    }
    if (config["accessMode"])
    {
      params.accessMode = config["accessMode"].as<std::string>();
    }
    if (config["egressMode"])
    {
      params.egressMode = config["egressMode"].as<std::string>();
    }
    if (config["noResultSecondMode"])
    {
      params.noResultSecondMode = config["noResultSecondMode"].as<std::string>();
    }
    if (config["noResultNextAccessTimeMinutesIncrement"])
    {
      params.noResultNextAccessTimeMinutesIncrement = config["noResultNextAccessTimeMinutesIncrement"].as<int>();
    }
    if (config["tryNextModeIfRoutingFails"])
    {
      params.tryNextModeIfRoutingFails = config["tryNextModeIfRoutingFails"].as<bool>();
    }
    if (config["maxNoResultNextAccessTimeMinutes"])
    {
      params.maxNoResultNextAccessTimeMinutes = config["maxNoResultNextAccessTimeMinutes"].as<int>();
    }
    
  }
  
  std::string TripBasedAlgorithm::calculate()
  {
    return "";
  }
  
}


