#include <chrono>
#include <boost/uuid/string_generator.hpp>
#include <boost/algorithm/string.hpp>
#include "spdlog/spdlog.h"

#include "parameters.hpp"
#include "node.hpp"
#include "od_trip.hpp"

namespace TrRouting
{

  OdTripLegacyParameters::OdTripLegacyParameters()
  {
    odTripsPeriods.clear();
    odTripsGenders.clear();
    odTripsAgeGroups.clear();
    odTripsOccupations.clear();
    odTripsActivities.clear();
    odTripsModes.clear();

    onlyDataSource.reset();

    batchNumber                            = 1;
    batchesCount                           = 1;
    odTripsSampleRatio                     = 1.0;
    odTripUuid.reset();
    odTripsSampleSize                      = -1;
    calculateProfiles                      = true;
    seed                                   = std::chrono::system_clock::now().time_since_epoch().count();

  }
}
