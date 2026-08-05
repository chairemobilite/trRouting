#pragma once

#include <unordered_map>
#include <vector>

#include "line.hpp"
#include "connection_set.hpp"
#include "trip.hpp"

namespace TrRouting
{

  /**
     Generic interface to handle configuring and executing a filter to disable some trips to generate alternatives
     route results
   */  
  class AlternativeFilter {
  public:
    AlternativeFilter() {}
    virtual ~AlternativeFilter() = default;
    /**
       Apply a filter to a connectionSet and set flags in the trips disabled overlay. The clearing
       of the trips overlay should be done outside of the filter, so we can apply multiple filter to the same set

       @param tripsDisabled Container to be filled with flags of which trips are no longer available
       @param connectionSet Current list of trips matching the query scenarios

       @return void (Could be changed to return a bool to represent if something was filtered or not)
     */
    virtual void runFilter(std::unordered_map<Trip::uid_t, bool> &tripsDisabled, const ConnectionSet &connectionSet) = 0;
  };

  /** Alternative Line filter
      Take a list of line and will disable all trips using that line.
   */
  class AlternativeLineFilter : public AlternativeFilter {
  public:
    AlternativeLineFilter(const std::vector<std::reference_wrapper<const Line>> &_excludedLines);
    virtual void runFilter(std::unordered_map<Trip::uid_t, bool> &tripsDisabled, const ConnectionSet &connectionSet) override;
  protected:
    /** Simplified copy of the line vector (with only the UIDs) */
    std::vector<Line::uid_t> excludedLinesUids;
  };
    
}


