#include "trip_based_algorithm.hpp"

using json = nlohmann::json;

namespace TrRouting
{
  
  TripBasedAlgorithm::TripBasedAlgorithm(ServerParameters& theParams) : params(theParams)
  {
    algorithmCalculationTime = CalculationTime();
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
  
  // clean memory and destroy objects:
  void TripBasedAlgorithm::destroy()
  {
    std::vector<std::vector<int>>().swap(footpathsIndex);
    std::vector<Footpath>().swap(footpathsBySource);
    std::vector<Footpath>().swap(footpathsByTarget);
    std::vector<RoutePath>().swap(routePaths);
    std::vector<int>().swap(routePathsIndexById);
    std::vector<std::vector<std::vector<int>>>().swap(routePathsIndexByStop);
    std::vector<Stop>().swap(stops);
    std::vector<int>().swap(stopsIndexById);
    std::vector<std::vector<int>>().swap(stopsIndexByRoutePath);

    // by weekday:
    std::vector<std::vector<int>>().swap(arrivalTimes);
    std::vector<std::vector<int>>().swap(departureTimes);
    std::vector<std::vector<std::vector<int>>>().swap(arrivalTimesIndex);
    std::vector<std::vector<std::vector<int>>>().swap(departureTimesIndex);
    std::vector<std::vector<Transfer>>().swap(transfers);
    std::vector<std::vector<std::vector<int>>>().swap(transfersIndex);
    std::vector<std::vector<Trip>>().swap(trips);
    std::vector<std::vector<std::vector<int>>>().swap(tripsIndex);
    std::vector<std::vector<int>>().swap(tripsIndexById);

  }
  
  // call setup only once when starting the calculator. Use updateParams before each calculation.
  void TripBasedAlgorithm::setup()
  {

    algorithmCalculationTime.startStep();

    params.setDefaultValues();
    //setParamsFromYaml("trRoutingTripBasedConfig.yml"); // disable yml config for now
    
    std::string weekdayName  {"monday"};
    params.weekdayIndex = 1; // 0 = sunday, 6 = saturday

    std::string          dataName;
    std::ifstream        stream;
    std::vector<uint8_t> contents;
    json                 jsonContent;
    json::basic_json     jsonData;
    Footpath*            footpath;
    RoutePath*           routePath;
    Trip*                trip;
    Transfer*            transfer;
    Stop*                stop;
    int                  i;


    
    // fetch footpaths_by_source:
    dataName = "footpaths_by_source";
    stream   = std::ifstream("cache/" + params.projectShortname + "__trip_based_routing__" + dataName + ".msgpack", std::ios::in | std::ios::binary);
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
    //std::cerr << footpathsBySource[10].tt << std::endl;
    
    
    
    // fetch footpaths_by_target:
    dataName = "footpaths_by_target";
    stream   = std::ifstream("cache/" + params.projectShortname + "__trip_based_routing__" + dataName + ".msgpack", std::ios::in | std::ios::binary);
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
    //std::cerr << footpathsByTarget[10].tt << std::endl;
    
    
    
    // fetch footpaths_index_by_source: Exact same copy as footpaths_index_by_target, so we don't need both!
    dataName = "footpaths_index_by_source";
    stream   = std::ifstream("cache/" + params.projectShortname + "__trip_based_routing__" + dataName + ".msgpack", std::ios::in | std::ios::binary);
    contents = std::vector<uint8_t>((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    jsonContent = json::from_msgpack(contents);
    footpathsIndex = jsonContent.get<std::vector<std::vector<int>> >();
    //std::cerr << footpathsIndex[10][0] << std::endl;
    
    
    
    // fetch route_paths:
    dataName = "route_paths";
    stream   = std::ifstream("cache/" + params.projectShortname + "__trip_based_routing__" + dataName + ".msgpack", std::ios::in | std::ios::binary);
    contents = std::vector<uint8_t>((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    jsonContent = json::from_msgpack(contents);
    routePaths = std::vector<RoutePath>();
    routePaths.reserve(jsonContent.size());
    routePath = new RoutePath();
    for (json::iterator it = jsonContent.begin(); it != jsonContent.end(); ++it) {
      jsonData = *it;
      routePath->i     = jsonData["i"].get<int>();
      routePath->id    = jsonData["id"].get<long long>();
      routePath->rId   = jsonData["route_id"].get<long long>();
      routePath->rSn   = jsonData["route_shortname"].get<std::string>();
      routePath->rLn   = jsonData["route_longname"].get<std::string>();
      routePath->aId   = jsonData["agency_id"].get<long long>();
      routePath->aName = jsonData["agency_name"].get<std::string>();
      routePaths.push_back(*routePath);
    }
    //std::cerr << routePaths[0].aName << std::endl;
    
    

    // fetch route_paths_index_by_id
    dataName = "route_paths_index_by_id";
    stream   = std::ifstream("cache/" + params.projectShortname + "__trip_based_routing__" + dataName + ".msgpack", std::ios::in | std::ios::binary);
    contents = std::vector<uint8_t>((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    jsonContent = json::from_msgpack(contents);
    routePathsIndexById = jsonContent.get<std::vector<int>>();
    //std::cerr << routePathsIndexById[3] << std::endl;
    
    

    // fetch route_paths_index_by_stop:
    dataName = "route_paths_index_by_stop";
    stream   = std::ifstream("cache/" + params.projectShortname + "__trip_based_routing__" + dataName + ".msgpack", std::ios::in | std::ios::binary);
    contents = std::vector<uint8_t>((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    jsonContent = json::from_msgpack(contents);
    routePathsIndexByStop = jsonContent.get<std::vector<std::vector<std::vector<int>>>>();
    //json dumpJson(routePathsIndexByStop);
    //std::cerr << jsonContent.dump() << std::endl;



    // fetch stops:
    dataName = "stops";
    stream   = std::ifstream("cache/" + params.projectShortname + "__trip_based_routing__" + dataName + ".msgpack", std::ios::in | std::ios::binary);
    contents = std::vector<uint8_t>((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    jsonContent = json::from_msgpack(contents);
    stops = std::vector<Stop>();
    stops.reserve(jsonContent.size());
    stop = new Stop();
    for (json::iterator it = jsonContent.begin(); it != jsonContent.end(); ++it) {
      jsonData = *it;
      stop->i     = jsonData["i"].get<int>();
      //stop->at    = 99999;
      //stop->tt    = -1;
      stop->id    = jsonData["id"].get<long long>();
      stop->code  = jsonData["code"].get<std::string>();
      stop->name  = jsonData["name"].get<std::string>();
      stop->stId  = jsonData["station_id"].get<long long>();
      stop->point = *(new Point(jsonData["latitude"].get<float>(), jsonData["longitude"].get<float>()));
      stops.push_back(*stop);
    }
    //std::cerr << stops[0].name << std::endl;

    

    // fetch stops_index_by_id
    dataName = "stops_index_by_id";
    stream   = std::ifstream("cache/" + params.projectShortname + "__trip_based_routing__" + dataName + ".msgpack", std::ios::in | std::ios::binary);
    contents = std::vector<uint8_t>((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    jsonContent = json::from_msgpack(contents);
    stopsIndexById = jsonContent.get<std::vector<int>>();
    //std::cerr << stopsIndexById[3] << std::endl;
    
    

    // fetch stops_index_by_route_path:
    dataName = "stops_index_by_route_path";
    stream   = std::ifstream("cache/" + params.projectShortname + "__trip_based_routing__" + dataName + ".msgpack", std::ios::in | std::ios::binary);
    contents = std::vector<uint8_t>((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    jsonContent = json::from_msgpack(contents);
    stopsIndexByRoutePath = jsonContent.get<std::vector<std::vector<int>> >();
    //json dumpJson(stopsIndexByRoutePath);
    //std::cerr << jsonContent.dump() << std::endl;



    // fetch arrival_times
    dataName = "arrival_times";
    stream   = std::ifstream("cache/" + params.projectShortname + "__trip_based_routing__" + weekdayName + "__" + dataName + ".msgpack", std::ios::in | std::ios::binary);
    contents = std::vector<uint8_t>((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    jsonContent = json::from_msgpack(contents);
    //std::cerr << jsonContent.dump() << std::endl;
    arrivalTimes[params.weekdayIndex] = jsonContent.get<std::vector<int>>();
    //std::cerr << arrivalTimes[params.weekdayIndex][3] << std::endl;



    // fetch departure_times
    dataName = "departure_times";
    stream   = std::ifstream("cache/" + params.projectShortname + "__trip_based_routing__" + weekdayName + "__" + dataName + ".msgpack", std::ios::in | std::ios::binary);
    contents = std::vector<uint8_t>((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    jsonContent = json::from_msgpack(contents);
    departureTimes[params.weekdayIndex] = jsonContent.get<std::vector<int>>();
    //std::cerr << departureTimes[params.weekdayIndex][3] << std::endl;



    // fetch arrival_times_index
    dataName = "arrival_times_first_last_index_by_trip";
    stream   = std::ifstream("cache/" + params.projectShortname + "__trip_based_routing__" + weekdayName + "__" + dataName + ".msgpack", std::ios::in | std::ios::binary);
    contents = std::vector<uint8_t>((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    jsonContent = json::from_msgpack(contents);
    arrivalTimesIndex[params.weekdayIndex] = jsonContent.get<std::vector<std::vector<int>> >();
    //std::cerr << arrivalTimesIndex[params.weekdayIndex][3][0] << std::endl;



    // fetch departure_times_index
    dataName = "departure_times_first_last_index_by_trip";
    stream   = std::ifstream("cache/" + params.projectShortname + "__trip_based_routing__" + weekdayName + "__" + dataName + ".msgpack", std::ios::in | std::ios::binary);
    contents = std::vector<uint8_t>((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    jsonContent = json::from_msgpack(contents);
    departureTimesIndex[params.weekdayIndex] = jsonContent.get<std::vector<std::vector<int>> >();
    //std::cerr << departureTimesIndex[params.weekdayIndex][3][0] << std::endl;



    // fetch transfers:
    dataName = "transfers";
    stream   = std::ifstream("cache/" + params.projectShortname + "__trip_based_routing__" + weekdayName + "__" + dataName + ".msgpack", std::ios::in | std::ios::binary);
    contents = std::vector<uint8_t>((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    jsonContent = json::from_msgpack(contents);
    transfers[params.weekdayIndex] = std::vector<Transfer>();
    transfers[params.weekdayIndex].reserve(jsonContent.size());
    transfer = new Transfer();
    for (json::iterator it = jsonContent.begin(); it != jsonContent.end(); ++it) {
      jsonData = *it;
      transfer->srcStopSeq = jsonData["source_stop_seq"].get<int>();
      transfer->srcTripI   = jsonData["source_trip_i"].get<int>();
      transfer->tgtStopSeq = jsonData["target_stop_seq"].get<int>();
      transfer->tgtTripI   = jsonData["target_trip_i"].get<int>();
      transfers[params.weekdayIndex].push_back(*transfer);
    }
    //std::cerr << transfers[params.weekdayIndex][3].tgtTripI << std::endl;



    // fetch transfers_index
    dataName = "transfers_first_last_by_arrival_time_index";
    stream   = std::ifstream("cache/" + params.projectShortname + "__trip_based_routing__" + weekdayName + "__" + dataName + ".msgpack", std::ios::in | std::ios::binary);
    contents = std::vector<uint8_t>((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    jsonContent = json::from_msgpack(contents);
    transfersIndex[params.weekdayIndex] = jsonContent.get<std::vector<std::vector<int>> >();
    //std::cerr << transfersIndex[params.weekdayIndex][3][1] << std::endl;



    // fetch trips:
    dataName = "trips";
    stream   = std::ifstream("cache/" + params.projectShortname + "__trip_based_routing__" + weekdayName + "__" + dataName + ".msgpack", std::ios::in | std::ios::binary);
    contents = std::vector<uint8_t>((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    jsonContent = json::from_msgpack(contents);
    trips[params.weekdayIndex] = std::vector<Trip>();
    trips[params.weekdayIndex].reserve(jsonContent.size());
    trip = new Trip();
    for (json::iterator it = jsonContent.begin(); it != jsonContent.end(); ++it) {
      jsonData = *it;
      trip->i    = jsonData["i"].get<int>();
      trip->seq  = jsonData["trip_seq"].get<int>();
      trip->rpI  = jsonData["route_path_i"].get<int>();
      trip->id   = jsonData["id"].get<long long>();
      trips[params.weekdayIndex].push_back(*trip);
    }
    //std::cerr << trips[params.weekdayIndex][3].rpI << std::endl;



    // fetch trips_index
    dataName = "trips_first_last_index_by_route_path";
    stream   = std::ifstream("cache/" + params.projectShortname + "__trip_based_routing__" + weekdayName + "__" + dataName + ".msgpack", std::ios::in | std::ios::binary);
    contents = std::vector<uint8_t>((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    jsonContent = json::from_msgpack(contents);
    tripsIndex[params.weekdayIndex] = jsonContent.get<std::vector<std::vector<int>> >();
    //std::cerr << tripsIndex[params.weekdayIndex][3][0] << std::endl;



    algorithmCalculationTime.stopStep();
    std::cerr << "-- Fetching data from cache -- " << algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
    


    // ------ start testing performance ------- 
    /*
    


    algorithmCalculationTime.startStep();
    std::vector<std::vector<int>> containerVectors(100000, std::vector<int>(3));
    for (int i = 0; i < 100000; i++)
    {
      containerVectors[i] = *(new std::vector<int>{234,453,12});
    }
    algorithmCalculationTime.stopStep();
    std::cerr << "-- testing allocating vectors or int -- " << algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
    algorithmCalculationTime.startStep();
    // this is faster!
    std::vector<ReachableRoutePath> containerStructs;
    for (int i = 0; i < 100000; i++)
    {
      containerStructs.push_back(*(new ReachableRoutePath(234,453,12)));
    }
    algorithmCalculationTime.stopStep();
    std::cerr << "-- testing allocating structs -- " << algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
    




    int a,b,c;
    algorithmCalculationTime.startStep();
    for (int i = 0; i < 1000; i++)
    {
      for(int j = 0; j < containerVectors.size(); j++)
      {
        a = containerVectors[j][0];
        b = containerVectors[j][1];
        c = containerVectors[j][2];
      }
    }
    algorithmCalculationTime.stopStep();
    std::cerr << "-- testing fetching vectors elements -- " << algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
    algorithmCalculationTime.startStep();
    // this is faster!
    for (int i = 0; i < 1000; i++)
    {
      for(int j = 0; j < containerStructs.size(); j++)
      {
        a = containerStructs[j].rpI;
        b = containerStructs[j].stopSeq;
        c = containerStructs[j].tt;
      }
    }
    algorithmCalculationTime.stopStep();
    std::cerr << "-- testing fetching structs elements -- " << algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";




    */
    // ------ end testing performance ------- 

  }    
  
  void TripBasedAlgorithm::refresh()
  {

    //algorithmCalculationTime.startStep();

    //for(auto & stop : stops)
    //{
    //  stop.at = 99999;
    //  stop.tt = -1;
    //}

    //algorithmCalculationTime.stopStep();
    //std::cerr << "-- Refresh -- " << algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";


  }
  
  // Call before each calculation
  void TripBasedAlgorithm::updateParams(ServerParameters& theParams)
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
  
}


