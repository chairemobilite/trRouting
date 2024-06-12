#include <boost/uuid/string_generator.hpp>
#include <boost/algorithm/string.hpp>

#include "parameters.hpp"
#include "scenario.hpp"
#include "point.hpp"

namespace TrRouting
{

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
    scenarioUuid = scenario.uuid; //TODO Check if this is used somewhere
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

  int CommonParameters::getIntegerValue(std::string strValue) {
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
        timeOfTrip = CommonParameters::getIntegerValue(parameterWithValue.second);
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
        boost::uuids::uuid scenarioUuid;
        try {
          scenarioUuid  = uuidGenerator(parameterWithValue.second);
        } catch (std::runtime_error const& exc) {
          // If we cannot parse the parameter as a valid uuid, return an INVALID error
          throw ParameterException(ParameterException::Type::INVALID_SCENARIO);
        }
        auto scenarioIte = scenarios.find(scenarioUuid);
        if (scenarioIte != scenarios.end())
        {
          scenario = scenarioIte->second;
        } else {
          // If the scenario UUID is not in our DB return an INVALID error
          throw ParameterException(ParameterException::Type::INVALID_SCENARIO);
        }
        continue;
      }

      // min and max parameters:
      else if (parameterWithValue.first == "min_waiting_time")
      {
        minWaitingTimeSeconds = CommonParameters::getIntegerValue(parameterWithValue.second);
        if (minWaitingTimeSeconds < 0)
        {
          minWaitingTimeSeconds = 0;
        }
        continue;
      }
      else if (parameterWithValue.first == "max_travel_time")
      {
        maxTotalTravelTimeSeconds = CommonParameters::getIntegerValue(parameterWithValue.second);
        if (maxTotalTravelTimeSeconds <= 0)
        {
          maxTotalTravelTimeSeconds = MAX_INT;
        }
        continue;
      }
      else if (parameterWithValue.first == "max_access_travel_time")
      {
        maxAccessWalkingTravelTimeSeconds = CommonParameters::getIntegerValue(parameterWithValue.second);
        if (maxAccessWalkingTravelTimeSeconds <= 0)
        {
          maxAccessWalkingTravelTimeSeconds = MAX_INT;
        }
        continue;
      }
      else if (parameterWithValue.first == "max_egress_travel_time")
      {
        maxEgressWalkingTravelTimeSeconds = CommonParameters::getIntegerValue(parameterWithValue.second);
        if (maxEgressWalkingTravelTimeSeconds <= 0)
        {
          maxEgressWalkingTravelTimeSeconds = MAX_INT;
        }
        continue;
      }
      else if (parameterWithValue.first == "max_transfer_travel_time")
      {
        maxTransferWalkingTravelTimeSeconds = CommonParameters::getIntegerValue(parameterWithValue.second);
        if (maxTransferWalkingTravelTimeSeconds <= 0)
        {
          maxTransferWalkingTravelTimeSeconds = MAX_INT;
        }
        continue;
      }
      else if (parameterWithValue.first == "max_first_waiting_time")
      {
        maxFirstWaitingTimeSeconds = CommonParameters::getIntegerValue(parameterWithValue.second);
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

 
}
