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
#include "connection_cache.hpp"
#include "transit_data.hpp"
#include "geofilter.hpp"

namespace TrRouting
{

  Calculator::Calculator(const TransitData &_transitData, GeoFilter &_geoFilter) :
    algorithmCalculationTime(CalculationTime()),
    odTripGlob(std::nullopt),
    transitData(_transitData),
    geoFilter(_geoFilter),
    departureTimeSeconds(0),
    arrivalTimeSeconds(-1),
    minAccessTravelTime(0),
    maxEgressTravelTime(0),
    maxAccessTravelTime(0),
    minEgressTravelTime(0),
    calculationTime(0)
  {

  }

  void Calculator::initializeCalculationData() {
    spdlog::info("preparing nodes tentative times, trips enter connections and journeys...");

      
    nodesReverseTentativeTime.clear();
    nodesAccess.clear();
    nodesEgress.clear();
    forwardJourneysSteps.clear();
    reverseJourneysSteps.clear();

    tripsDisabled.clear();
    tripsQueryOverlay.clear();

    spdlog::info("{} connections", transitData.getConnectionCount());;

    //int benchmarkingStart = algorithmCalculationTime.getEpoch();

  }

}
