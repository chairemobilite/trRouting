#include <boost/uuid/string_generator.hpp>
#include <boost/algorithm/string.hpp>

#include "spdlog/spdlog.h"
#include "parameters.hpp"
#include "scenario.hpp"
#include "point.hpp"

namespace TrRouting
{

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

  RouteParameters RouteParameters::createRouteODParameter(std::vector<std::pair<std::string, std::string>> &parameters, const std::map<boost::uuids::uuid, Scenario> &scenarios)
  {

    std::optional<Point> origin;
    std::optional<Point> destination;
    bool alternatives = false;

    std::vector<std::string> latitudeLongitudeVector;

    // TODO Replace manually parsing parameters by a library that does this
    for (auto & parameterWithValue : parameters)
    {
      spdlog::debug(" received parameter {} with value {}", parameterWithValue.first, parameterWithValue.second);

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
