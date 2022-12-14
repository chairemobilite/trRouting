#include <boost/uuid/string_generator.hpp>
#include <boost/algorithm/string.hpp>

#include "parameters.hpp"
#include "toolbox.hpp" //MAX_INT
#include "scenario.hpp"
#include "point.hpp"

namespace TrRouting
{
  static int DEFAULT_MIN_WAITING_TIME = 3 * 60;
  static int DEFAULT_MAX_TOTAL_TIME = MAX_INT;
  static int DEFAULT_MAX_ACCESS_TRAVEL_TIME = 20 * 60;
  static int DEFAULT_MAX_EGRESS_TRAVEL_TIME = 20 * 60;
  static int DEFAULT_MAX_TRANSFER_TRAVEL_TIME = 20 * 60;
  static int DEFAULT_FIRST_WAITING_TIME = 30 * 60;

  CommonParameters::CommonParameters(const Scenario& scenario_,
    int _timeOfTrip,
    int minWaitingTime,
    int maxTotalTime,
    int maxAccessTime,
    int maxEgressTime,
    int maxTransferTime,
    int maxFirstWaitingTime,
    bool forward) :
        scenario(scenario_),
        timeOfTrip(_timeOfTrip),
        minWaitingTimeSeconds(minWaitingTime),
        maxTotalTravelTimeSeconds(maxTotalTime),
        maxAccessWalkingTravelTimeSeconds(maxAccessTime),
        maxEgressWalkingTravelTimeSeconds(maxEgressTime),
        maxTransferWalkingTravelTimeSeconds(maxTransferTime),
        maxFirstWaitingTimeSeconds(maxFirstWaitingTime),
        forwardCalculation(forward)
  {
    scenarioUuid = scenario.uuid;
    onlyServices = scenario.servicesList;
    onlyLines = scenario.onlyLines;
    onlyAgencies = scenario.onlyAgencies;
    onlyNodes = scenario.onlyNodes;
    onlyModes = scenario.onlyModes;
    exceptLines = scenario.exceptLines;
    exceptAgencies = scenario.exceptAgencies;
    exceptNodes = scenario.exceptNodes;
    exceptModes = scenario.exceptModes;
  }

  CommonParameters::CommonParameters(const CommonParameters& baseParams):
    scenario(baseParams.scenario),
    timeOfTrip(baseParams.timeOfTrip),
    minWaitingTimeSeconds(baseParams.minWaitingTimeSeconds),
    maxTotalTravelTimeSeconds(baseParams.maxTotalTravelTimeSeconds),
    maxAccessWalkingTravelTimeSeconds(baseParams.maxAccessWalkingTravelTimeSeconds),
    maxEgressWalkingTravelTimeSeconds(baseParams.maxEgressWalkingTravelTimeSeconds),
    maxTransferWalkingTravelTimeSeconds(baseParams.maxTransferWalkingTravelTimeSeconds),
    maxFirstWaitingTimeSeconds(baseParams.maxFirstWaitingTimeSeconds),
    scenarioUuid(baseParams.scenarioUuid),
    onlyServices(baseParams.onlyServices),
    onlyLines(baseParams.onlyLines),
    onlyAgencies(baseParams.onlyAgencies),
    onlyModes(baseParams.onlyModes),
    onlyNodes(baseParams.onlyNodes),
    exceptLines(baseParams.exceptLines),
    exceptAgencies(baseParams.exceptAgencies),
    exceptModes(baseParams.exceptModes),
    exceptNodes(baseParams.exceptNodes),
    forwardCalculation(baseParams.forwardCalculation)
  {
  }

  RouteParameters::RouteParameters(std::unique_ptr<Point> orig_,
    std::unique_ptr<Point> dest_,
    const Scenario& scenario_,
    int _timeOfTrip,
    int minWaitingTime,
    int maxTotalTime,
    int maxAccessTime,
    int maxEgressTime,
    int maxTransferTime,
    int maxFirstWaitingTime,
    bool alt,
    bool forward) : 
        CommonParameters(scenario_,
                      _timeOfTrip,
                      minWaitingTime,
                      maxTotalTime,
                      maxAccessTime,
                      maxEgressTime,
                      maxTransferTime,
                      maxFirstWaitingTime,
                      forward),
        origin(std::move(orig_)),
        destination(std::move(dest_)),
        withAlternatives(alt)
  {
  }

  RouteParameters::RouteParameters(std::unique_ptr<Point> orig_,
    std::unique_ptr<Point> dest_,
    bool alt,
    const CommonParameters &common_) : 
        CommonParameters(common_),
        origin(std::move(orig_)),
        destination(std::move(dest_)),
        withAlternatives(alt)
  {
  }

  RouteParameters::RouteParameters(const RouteParameters& routeParams):
    CommonParameters(routeParams),
    origin(std::make_unique<Point>(routeParams.origin.get()->latitude, routeParams.origin.get()->longitude)),
    destination(std::make_unique<Point>(routeParams.destination.get()->latitude, routeParams.destination.get()->longitude)),
    withAlternatives(routeParams.withAlternatives)
  {
  }

  static int getIntegerValue(std::string strValue) {
    try
    {
      return std::stoi(strValue);
    }
    catch (...)
    {
      // Throwing an error, but do we want to fallback to default and just ignore this one?
      throw ParameterException(ParameterException::Type::INVALID_NUMERICAL_DATA);
    }
  }

  CommonParameters CommonParameters::createCommonParameter(std::vector<std::pair<std::string, std::string>> &parameters, const std::map<boost::uuids::uuid, Scenario> &scenarios)
  {
    boost::uuids::string_generator uuidGenerator;

    std::optional<std::reference_wrapper<const Scenario>> scenario;

    // Initialize default values
    int timeOfTrip = -1;
    int minWaitingTimeSeconds = DEFAULT_MIN_WAITING_TIME;
    int maxTotalTravelTimeSeconds = DEFAULT_MAX_TOTAL_TIME;
    int maxAccessWalkingTravelTimeSeconds = DEFAULT_MAX_ACCESS_TRAVEL_TIME;
    int maxEgressWalkingTravelTimeSeconds = DEFAULT_MAX_EGRESS_TRAVEL_TIME;
    int maxTransferWalkingTravelTimeSeconds = DEFAULT_MAX_TRANSFER_TRAVEL_TIME;
    int maxFirstWaitingTimeSeconds = DEFAULT_FIRST_WAITING_TIME; // Ignore all connections at access nodes if waiting time would be more than this value.
    bool forwardCalculation = true;

    // TODO Replace manually parsing parameters by a library that does this
    for (auto & parameterWithValue : parameters)
    {
      // times and date:
      if (parameterWithValue.first == "time_of_trip")
      {
        timeOfTrip = getIntegerValue(parameterWithValue.second);
        if (timeOfTrip < 0)
        {
          timeOfTrip = -1;
        }
      }
      else if (parameterWithValue.first == "time_type")
      {
        if (parameterWithValue.second == "1")
        {
          forwardCalculation = false;
        }
        continue;
      }

      // scenario:
      else if (parameterWithValue.first == "scenario_id")
      {
        boost::uuids::uuid scenarioUuid  = uuidGenerator(parameterWithValue.second);

        auto scenarioIte = scenarios.find(scenarioUuid);
        if (scenarioIte != scenarios.end())
        {
          scenario = scenarioIte->second;
        }
        continue;
      }

      // min and max parameters:
      else if (parameterWithValue.first == "min_waiting_time")
      {
        minWaitingTimeSeconds = getIntegerValue(parameterWithValue.second);
        if (minWaitingTimeSeconds < 0)
        {
          minWaitingTimeSeconds = 0;
        }
        continue;
      }
      else if (parameterWithValue.first == "max_travel_time")
      {
        maxTotalTravelTimeSeconds = getIntegerValue(parameterWithValue.second);
        if (maxTotalTravelTimeSeconds <= 0)
        {
          maxTotalTravelTimeSeconds = MAX_INT;
        }
        continue;
      }
      else if (parameterWithValue.first == "max_access_travel_time")
      {
        maxAccessWalkingTravelTimeSeconds = getIntegerValue(parameterWithValue.second);
        if (maxAccessWalkingTravelTimeSeconds <= 0)
        {
          maxAccessWalkingTravelTimeSeconds = MAX_INT;
        }
        continue;
      }
      else if (parameterWithValue.first == "max_egress_travel_time")
      {
        maxEgressWalkingTravelTimeSeconds = getIntegerValue(parameterWithValue.second);
        if (maxEgressWalkingTravelTimeSeconds <= 0)
        {
          maxEgressWalkingTravelTimeSeconds = MAX_INT;
        }
        continue;
      }
      else if (parameterWithValue.first == "max_transfer_travel_time")
      {
        maxTransferWalkingTravelTimeSeconds = getIntegerValue(parameterWithValue.second);
        if (maxTransferWalkingTravelTimeSeconds <= 0)
        {
          maxTransferWalkingTravelTimeSeconds = MAX_INT;
        }
        continue;
      }
      else if (parameterWithValue.first == "max_first_waiting_time")
      {
        maxFirstWaitingTimeSeconds = getIntegerValue(parameterWithValue.second);
        if (maxFirstWaitingTimeSeconds <= 0)
        {
          maxFirstWaitingTimeSeconds = -1;
        }
        continue;
      }
    }

    // Validate scenario parameters
    if (!scenario.has_value())
    {
      throw ParameterException(ParameterException::Type::MISSING_SCENARIO);
    }
    else if (scenario.value().get().servicesList.size() <= 0)
    {
      throw ParameterException(ParameterException::Type::EMPTY_SCENARIO);
    }
    else if (timeOfTrip < 0)
    {
      throw ParameterException(ParameterException::Type::MISSING_TIME_OF_TRIP);
    }

    return CommonParameters(scenario.value().get(),
      timeOfTrip,
      minWaitingTimeSeconds,
      maxTotalTravelTimeSeconds,
      maxAccessWalkingTravelTimeSeconds,
      maxEgressWalkingTravelTimeSeconds,
      maxTransferWalkingTravelTimeSeconds,
      maxFirstWaitingTimeSeconds,
      forwardCalculation);
  }

  RouteParameters RouteParameters::createRouteODParameter(std::vector<std::pair<std::string, std::string>> &parameters, const std::map<boost::uuids::uuid, Scenario> &scenarios)
  {

    std::optional<Point> origin;
    std::optional<Point> destination;
    bool alternatives = false;

    std::vector<std::string> latitudeLongitudeVector;

    // TODO Replace manually parsing parameters by a library that does this
    for (auto & parameterWithValue : parameters)
    {
      // origin and destination:
      if (parameterWithValue.first == "origin")
      {
        try
        {
          boost::split(latitudeLongitudeVector, parameterWithValue.second, boost::is_any_of(","));
          if (latitudeLongitudeVector.size() != 2)
          {
            throw ParameterException(ParameterException::Type::INVALID_ORIGIN);
          }
          origin = Point(std::stod(latitudeLongitudeVector[1]), std::stod(latitudeLongitudeVector[0]));
        }
        catch (...)
        {
          throw ParameterException(ParameterException::Type::INVALID_ORIGIN);
        }
      }
      else if (parameterWithValue.first == "destination")
      {
        try
        {
          boost::split(latitudeLongitudeVector, parameterWithValue.second, boost::is_any_of(","));
          if (latitudeLongitudeVector.size() != 2)
          {
            throw ParameterException(ParameterException::Type::INVALID_DESTINATION);
          }
          destination = Point(std::stod(latitudeLongitudeVector[1]), std::stod(latitudeLongitudeVector[0]));
        }
        catch (...)
        {
          throw ParameterException(ParameterException::Type::INVALID_DESTINATION);
        }
      }
      else if (parameterWithValue.first == "alternatives")
      {
        if (parameterWithValue.second == "true" || parameterWithValue.second == "1")
        {
          alternatives = true;
        }
        continue;
      }
    }

    // Validate parameters
    if (!origin.has_value())
    {
      throw ParameterException(ParameterException::Type::MISSING_ORIGIN);
    }
    else if (!destination.has_value())
    {
      throw ParameterException(ParameterException::Type::MISSING_DESTINATION);
    }

    CommonParameters common = CommonParameters::createCommonParameter(parameters, scenarios);

    return RouteParameters(std::make_unique<TrRouting::Point>(origin->latitude, origin->longitude),
      std::make_unique<TrRouting::Point>(destination->latitude, destination->longitude),
      alternatives,
      common);
  }

}
