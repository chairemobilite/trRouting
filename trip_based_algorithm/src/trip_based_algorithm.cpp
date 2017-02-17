#include "trip_based_algorithm.hpp"

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
    
    std::ifstream ifs("tr_stsh_2016_03_test__trip_based_routing__route_paths_index_by_id.msgpack", std::ifstream::in);
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    msgpack::unpacked upd;
    msgpack::unpack(upd, buffer.str().data(), buffer.str().size());
    std::cout << upd.get() << std::endl;
    
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


