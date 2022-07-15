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

  RouteParameters::RouteParameters(std::unique_ptr<Point> orig_,
    std::unique_ptr<Point> dest_,
    Scenario& scenario_,
    int _timeOfTrip,
    int minWaitingTime,
    int maxTotalTime,
    int maxAccessTime,
    int maxEgressTime,
    int maxTransferTime,
    int maxFirstWaitingTime,
    bool alt,
    bool forward) :
        origin(std::move(orig_)),
        destination(std::move(dest_)),
        scenario(scenario_),
        timeOfTrip(_timeOfTrip),
        minWaitingTimeSeconds(minWaitingTime),
        maxTotalTravelTimeSeconds(maxTotalTime),
        maxAccessWalkingTravelTimeSeconds(maxAccessTime),
        maxEgressWalkingTravelTimeSeconds(maxEgressTime),
        maxTransferWalkingTravelTimeSeconds(maxTransferTime),
        maxFirstWaitingTimeSeconds(maxFirstWaitingTime),
        withAlternatives(alt),
        forwardCalculation(forward)
  {
    scenarioUuid = scenario.uuid;
    onlyServicesIdx = scenario.servicesIdx;
    onlyLinesIdx = scenario.onlyLinesIdx;
    onlyAgenciesIdx = scenario.onlyAgenciesIdx;
    onlyNodesIdx = scenario.onlyNodesIdx;
    onlyModes = scenario.onlyModes;
    exceptLinesIdx = scenario.exceptLinesIdx;
    exceptAgenciesIdx = scenario.exceptAgenciesIdx;
    exceptNodesIdx = scenario.exceptNodesIdx;
    exceptModes = scenario.exceptModes;
  }

  RouteParameters::RouteParameters(const RouteParameters& routeParams):
    origin(std::make_unique<Point>(routeParams.origin.get()->latitude, routeParams.origin.get()->longitude)),
    destination(std::make_unique<Point>(routeParams.destination.get()->latitude, routeParams.destination.get()->longitude)),
    scenario(routeParams.scenario),
    timeOfTrip(routeParams.timeOfTrip),
    minWaitingTimeSeconds(routeParams.minWaitingTimeSeconds),
    maxTotalTravelTimeSeconds(routeParams.maxTotalTravelTimeSeconds),
    maxAccessWalkingTravelTimeSeconds(routeParams.maxAccessWalkingTravelTimeSeconds),
    maxEgressWalkingTravelTimeSeconds(routeParams.maxEgressWalkingTravelTimeSeconds),
    maxTransferWalkingTravelTimeSeconds(routeParams.maxTransferWalkingTravelTimeSeconds),
    maxFirstWaitingTimeSeconds(routeParams.maxFirstWaitingTimeSeconds),
    withAlternatives(routeParams.withAlternatives),
    forwardCalculation(routeParams.forwardCalculation),
    scenarioUuid(routeParams.scenarioUuid),
    onlyServicesIdx(routeParams.onlyServicesIdx),
    onlyLinesIdx(routeParams.onlyLinesIdx),
    onlyAgenciesIdx(routeParams.onlyAgenciesIdx),
    onlyNodesIdx(routeParams.onlyNodesIdx),
    onlyModes(routeParams.onlyModes),
    exceptLinesIdx(routeParams.exceptLinesIdx),
    exceptAgenciesIdx(routeParams.exceptAgenciesIdx),
    exceptNodesIdx(routeParams.exceptNodesIdx),
    exceptModes(routeParams.exceptModes)
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

  RouteParameters RouteParameters::createRouteODParameter(std::vector<std::pair<std::string, std::string>> &parameters, std::map<boost::uuids::uuid, int> &scenarioIndexesByUuid, std::vector<std::unique_ptr<Scenario>> &scenarios)
  {

    boost::uuids::string_generator uuidGenerator;

    Scenario * scenario = nullptr;
    std::optional<boost::uuids::uuid> scenarioUuid;
    boost::uuids::uuid originNodeUuid;
    boost::uuids::uuid destinationNodeUuid;

    // Initialize default values
    int timeOfTrip = -1;
    int minWaitingTimeSeconds = DEFAULT_MIN_WAITING_TIME;
    int maxTotalTravelTimeSeconds = DEFAULT_MAX_TOTAL_TIME;
    int maxAccessWalkingTravelTimeSeconds = DEFAULT_MAX_ACCESS_TRAVEL_TIME;
    int maxEgressWalkingTravelTimeSeconds = DEFAULT_MAX_EGRESS_TRAVEL_TIME;
    int maxTransferWalkingTravelTimeSeconds = DEFAULT_MAX_TRANSFER_TRAVEL_TIME;
    int maxFirstWaitingTimeSeconds = DEFAULT_FIRST_WAITING_TIME; // Ignore all connections at access nodes if waiting time would be more than this value.
    std::optional<Point> origin;
    std::optional<Point> destination;
    bool alternatives = false;
    bool forwardCalculation = true;

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

      // times and date:
      else if (parameterWithValue.first == "time_of_trip")
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
        scenarioUuid = uuidGenerator(parameterWithValue.second);
        if (scenarioIndexesByUuid.count(*scenarioUuid) == 1)
        {
          scenario = scenarios[scenarioIndexesByUuid[*scenarioUuid]].get();
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

      else if (parameterWithValue.first == "alternatives")
      {
        if (parameterWithValue.second == "true" || parameterWithValue.second == "1")
        {
          alternatives = true;
        }
        continue;
      }
    }

    // Validate scenario parameters
    if (scenario == nullptr)
    {
      throw ParameterException(ParameterException::Type::MISSING_SCENARIO);
    }
    else if (scenario->servicesIdx.size() <= 0)
    {
      throw ParameterException(ParameterException::Type::EMPTY_SCENARIO);
    }
    else if (!origin.has_value())
    {
      throw ParameterException(ParameterException::Type::MISSING_ORIGIN);
    }
    else if (!destination.has_value())
    {
      throw ParameterException(ParameterException::Type::MISSING_DESTINATION);
    }
    else if (timeOfTrip < 0)
    {
      throw ParameterException(ParameterException::Type::MISSING_TIME_OF_TRIP);
    }

    return RouteParameters(std::make_unique<TrRouting::Point>(origin->latitude, origin->longitude),
      std::make_unique<TrRouting::Point>(destination->latitude, destination->longitude),
      *scenario,
      timeOfTrip,
      minWaitingTimeSeconds,
      maxTotalTravelTimeSeconds,
      maxAccessWalkingTravelTimeSeconds,
      maxEgressWalkingTravelTimeSeconds,
      maxTransferWalkingTravelTimeSeconds,
      maxFirstWaitingTimeSeconds,
      alternatives,
      forwardCalculation);
  }

}
