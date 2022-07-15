
#ifndef TR_MODES_INITIALIZATION
#define TR_MODES_INITIALIZATION

#include <string>
#include <vector>
#include "spdlog/spdlog.h"

#include "cache_fetcher.hpp"
#include "mode.hpp"

namespace TrRouting
{

  const std::map<std::string, Mode> CacheFetcher::getModes()
  {

    std::map<std::string, Mode> modes;

    spdlog::info("Initializing modes...");

    modes.emplace("bus", Mode("bus", "Bus", 3, 700));
    modes.emplace("rail", Mode("rail", "Rail", 2, 100));
    modes.emplace("highSpeedRail", Mode("highSpeedRail", "High speed rail", 2, 101));
    modes.emplace("metro", Mode("metro", "Subway/Metro", 1, 400));
    modes.emplace("monorail", Mode("monorail", "Monorail", 1, 405));
    modes.emplace("tram", Mode("tram", "Tram/LRT", 0, 900));
    modes.emplace("tramTrain", Mode("tramTrain", "Tram Train", 0, 900));
    modes.emplace("water", Mode("water", "Ferry/Boat", 4, 1000));
    modes.emplace("gondola", Mode("gondola", "Gondola/Aerial tram", 6, 1300));
    modes.emplace("funicular", Mode("funicular", "Funicular", 7, 1400));
    modes.emplace("taxi", Mode("taxi", "Taxi/Cab/Minivan", 3, 1500));
    modes.emplace("cableCar", Mode("cableCar", "Cable car", 5, 1701));
    modes.emplace("horse", Mode("horse", "Horse carriage", 3, 1702));
    modes.emplace("other", Mode("other", "Other", 3, 1700));
    // TODO investigate if "transferable" should really be a mode or a flag of another object
    modes.emplace(Mode::TRANSFERABLE, Mode(Mode::TRANSFERABLE, "Transferable", -1, -1));

    return modes;
  }

}

#endif // TR_MODES_INITIALIZATION
