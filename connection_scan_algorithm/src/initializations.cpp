#include <boost/uuid/uuid.hpp>
#include "spdlog/spdlog.h"

#include "calculator.hpp"
#include "trip.hpp"
#include "mode.hpp"
#include "agency.hpp"
#include "data_source.hpp"
#include "node.hpp"
#include "line.hpp"
#include "path.hpp"
#include "scenario.hpp"
#include "service.hpp"
#include "od_trip.hpp"
#include "place.hpp"
#include "household.hpp"
#include "person.hpp"
#include "transit_data.hpp"

namespace TrRouting
{

  Calculator::Calculator(const TransitData &_transitData) :
    algorithmCalculationTime(CalculationTime()),
    odTripGlob(std::nullopt),
    transitData(_transitData),
    departureTimeSeconds(0),
    initialDepartureTimeSeconds(0),
    arrivalTimeSeconds(-1),
    maxTimeValue(0),
    minAccessTravelTime(0),
    maxEgressTravelTime(0),
    maxAccessTravelTime(0),
    minEgressTravelTime(0),
    maxAccessWalkingTravelTimeFromOriginToFirstNodeSeconds(0),
    maxAccessWalkingTravelTimeFromLastNodeToDestinationSeconds(0),
    calculationTime(0),
    accessMode("walking"),
    egressMode("walking")
  {

  }

  void Calculator::initializeCalculationData() {
    spdlog::info("preparing nodes tentative times, trips enter connections and journeys...");

      
    nodesTentativeTime.clear();
    nodesReverseTentativeTime.clear();
    nodesAccess.clear();
    nodesEgress.clear();
    forwardJourneysSteps.clear();
    forwardEgressJourneysSteps.clear();
    reverseJourneysSteps.clear();
    reverseAccessJourneysSteps.clear();

    tripsEnabled.clear();
    tripsQueryOverlay.clear();

    spdlog::info("{} connections", transitData.getForwardConnections().size());;

    //int benchmarkingStart = algorithmCalculationTime.getEpoch();

    int lastConnectionIndex = transitData.getForwardConnections().size() - 1;

    forwardConnectionsIndexPerDepartureTimeHour = std::vector<int>(32, -1);
    reverseConnectionsIndexPerArrivalTimeHour   = std::vector<int>(32, lastConnectionIndex);

    int hour {0};
    int i = 0;
    for (auto & connection : transitData.getForwardConnections())
    {
      while (connection->getDepartureTime() >= hour * 3600 && forwardConnectionsIndexPerDepartureTimeHour[hour] == -1 && hour < 32)
      {
        forwardConnectionsIndexPerDepartureTimeHour[hour] = i;
        hour++;
      }
      i++;
    }

    hour = 31;
    i = 0;
    for (auto & connection : transitData.getReverseConnections())
    {
      while (connection->getArrivalTime() <= hour * 3600 && reverseConnectionsIndexPerArrivalTimeHour[hour] == lastConnectionIndex && hour >= 0)
      {
        reverseConnectionsIndexPerArrivalTimeHour[hour] = i;
        hour--;
      }
      i++;
    }

    for (int h = 0; h < 32; h++)
    {
      if (forwardConnectionsIndexPerDepartureTimeHour[h] == -1)
      {
        forwardConnectionsIndexPerDepartureTimeHour[h] = lastConnectionIndex;
      }
    }

  }

}
