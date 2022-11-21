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
    reverseJourneysSteps.clear();

    tripsEnabled.clear();
    tripsQueryOverlay.clear();

    spdlog::info("{} connections", transitData.getForwardConnections().size());;

    //int benchmarkingStart = algorithmCalculationTime.getEpoch();

  }

}
