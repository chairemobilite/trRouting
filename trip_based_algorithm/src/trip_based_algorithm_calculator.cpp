#include "trip_based_algorithm.hpp"


namespace TrRouting
{
  
  // (Algorithm 4)
  json TripBasedAlgorithm::calculate()
  {
    // prepare json content and refresh variables for calculation:
    json jsonResult;
    refresh();
    resetAccessEgressModes();
    int nMax     = params.maxNumberOfTransfers;
    int infinite = 99999;

    // prepare target stop indexes to calculate according to returnAllStopsResult param value:
    std::vector<int> tgtStopIs;
    if (params.returnAllStopsResult)
    {
      tgtStopIs = std::vector<int>(stops.size());
      // fill the target stop indexes vector with increasing values from 0 to the number of stop indexes - 1:
      std::iota(std::begin(tgtStopIs), std::end(tgtStopIs), 0);
    }
    else
    {
      tgtStopIs = std::vector<int>(1,stopsIndexById[params.endingStopId]);
    }

    // calculate for each stop index in the target stop indexes vector:
    for(const auto & tgtStopI : tgtStopIs)
    {
      std::vector<int>                tauMin(nMax + 1, infinite); // set arrival time to infinite (line 1: we replace J by a vector tauMin including min arrival time for each number of transfers n)
      std::vector<ReachableRoutePath> reachableRoutePaths; // (line 2)
      std::vector<TripSegment>        tripSegments; // (line 3)
      tripSegments.reserve(nMax + 1);

    }



    return jsonResult;
  }

}