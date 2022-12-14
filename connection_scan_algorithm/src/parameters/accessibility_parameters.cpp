#include <boost/uuid/string_generator.hpp>
#include <boost/algorithm/string.hpp>

#include "parameters.hpp"
#include "scenario.hpp"
#include "point.hpp"

namespace TrRouting
{

  AccessibilityParameters::AccessibilityParameters(std::unique_ptr<Point> place_,
    const Scenario& scenario_,
    int _timeOfTrip,
    int minWaitingTime,
    int maxTotalTime,
    int maxAccessTime,
    int maxEgressTime,
    int maxTransferTime,
    int maxFirstWaitingTime,
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
        place(std::move(place_))
  {
  }

  AccessibilityParameters::AccessibilityParameters(std::unique_ptr<Point> place_,
    const CommonParameters &common_) : 
        CommonParameters(common_),
        place(std::move(place_))
  {
  }

  AccessibilityParameters AccessibilityParameters::createAccessibilityParameter(std::vector<std::pair<std::string, std::string>> &parameters, const std::map<boost::uuids::uuid, Scenario> &scenarios)
  {
    std::optional<Point> place;

    std::vector<std::string> latitudeLongitudeVector;

    // TODO Replace manually parsing parameters by a library that does this
    for (auto & parameterWithValue : parameters)
    {
      // place coordinates:
      if (parameterWithValue.first == "place")
      {
        try
        {
          boost::split(latitudeLongitudeVector, parameterWithValue.second, boost::is_any_of(","));
          if (latitudeLongitudeVector.size() != 2)
          {
            throw ParameterException(ParameterException::Type::INVALID_PLACE);
          }
          place = Point(std::stod(latitudeLongitudeVector[1]), std::stod(latitudeLongitudeVector[0]));
        }
        catch (...)
        {
          throw ParameterException(ParameterException::Type::INVALID_PLACE);
        }
      }

    }

    if (!place.has_value())
    {
      throw ParameterException(ParameterException::Type::MISSING_PLACE);
    }
    
    CommonParameters common = CommonParameters::createCommonParameter(parameters, scenarios);

    return AccessibilityParameters(std::make_unique<TrRouting::Point>(place->latitude, place->longitude),
      common
    );
  }

}
