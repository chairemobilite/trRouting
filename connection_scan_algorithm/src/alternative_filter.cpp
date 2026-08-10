#include "alternative_filter.hpp"

#include <algorithm>

#include "line.hpp"
#include "connection_set.hpp"
#include "trip.hpp"

namespace TrRouting {

  AlternativeLineFilter::AlternativeLineFilter(const std::vector<std::reference_wrapper<const Line>> &_excludedLines)
    : AlternativeFilter() {

    // Small optimisation, just saving the line UID and later iterating over that instead of going over the full objects
    excludedLinesUids.reserve(_excludedLines.size());
    for (const Line & line : _excludedLines) {
      excludedLinesUids.push_back(line.uid);
    }
    
  }
  
  void AlternativeLineFilter::runFilter(std::vector<TripQueryData> &tripsQueryOverlay, const ConnectionSet &connectionSet) {

    if (excludedLinesUids.size() > 0) {
      for (auto & tripIte : connectionSet.getTrips())
      {
        const Trip & trip = tripIte.get();

        // If the Trip match any lines in the filter, disable it
        if (std::find(excludedLinesUids.begin(), excludedLinesUids.end(), trip.line.uid) != excludedLinesUids.end())
        {
          tripsQueryOverlay.at(trip.uid).disabled = true;
        }
      }
    }
  }
}
