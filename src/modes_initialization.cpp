
#ifndef TR_MODES_INITIALIZATION
#define TR_MODES_INITIALIZATION

#include <string>
#include <vector>

#include "cache_fetcher.hpp"
#include "mode.hpp"

namespace TrRouting
{

  const std::pair<std::vector<Mode>, std::map<std::string, int>> CacheFetcher::getModes()
  {

    using T = Mode;

    std::vector<T> ts;
    std::map<std::string, int> tIndexesByShortname;

    std::cout << "Initializing modes..." << std::endl;

    T t1 = {"bus", "Bus", 3, 700};
    ts.push_back(t1);
    tIndexesByShortname[t1.shortname] = ts.size() - 1;

    T t2 = {"rail", "Rail", 2, 100};
    ts.push_back(t2);
    tIndexesByShortname[t2.shortname] = ts.size() - 1;

    T t3 = {"highSpeedRail", "High speed rail", 2, 101};
    ts.push_back(t3);
    tIndexesByShortname[t3.shortname] = ts.size() - 1;

    T t4 = {"metro", "Subway/Metro", 1, 400};
    ts.push_back(t4);
    tIndexesByShortname[t4.shortname] = ts.size() - 1;

    T t5 = {"monorail", "Monorail", 1, 405};
    ts.push_back(t5);
    tIndexesByShortname[t5.shortname] = ts.size() - 1;

    T t6 = {"tram", "Tram/LRT", 0, 900};
    ts.push_back(t6);
    tIndexesByShortname[t6.shortname] = ts.size() - 1;

    T t7 = {"tramTrain", "Tram Train", 0, 900};
    ts.push_back(t7);
    tIndexesByShortname[t7.shortname] = ts.size() - 1;

    T t8 = {"water", "Ferry/Boat", 4, 1000};
    ts.push_back(t8);
    tIndexesByShortname[t8.shortname] = ts.size() - 1;

    T t9 = {"gondola", "Gondola/Aerial tram", 6, 1300};
    ts.push_back(t9);
    tIndexesByShortname[t9.shortname] = ts.size() - 1;

    T t10 = {"funicular", "Funicular", 7, 1400};
    ts.push_back(t10);
    tIndexesByShortname[t10.shortname] = ts.size() - 1;

    T t11 = {"taxi", "Taxi/Cab/Minivan", 3, 1500};
    ts.push_back(t11);
    tIndexesByShortname[t11.shortname] = ts.size() - 1;

    T t12 = {"cableCar", "Cable car", 5, 1701};
    ts.push_back(t12);
    tIndexesByShortname[t12.shortname] = ts.size() - 1;
    
    T t13 = {"horse", "Horse carriage", 3, 1702};
    ts.push_back(t13);
    tIndexesByShortname[t13.shortname] = ts.size() - 1;

    T t14 = {"other", "Other", 3, 1700};
    ts.push_back(t14);
    tIndexesByShortname[t14.shortname] = ts.size() - 1;
    
    //std::cout << "Modes: \n" << Toolbox::prettyPrintStructVector(ts) << std::endl;

    return std::make_pair(ts, tIndexesByShortname);
  }

}

#endif // TR_MODES_INITIALIZATION