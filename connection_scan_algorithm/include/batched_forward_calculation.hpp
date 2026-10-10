#ifndef TR_BATCHED_FORWARD_CALCULATION
#define TR_BATCHED_FORWARD_CALCULATION

#include <optional>
#include <vector>

#include "node.hpp"
#include "line.hpp"
#include "routing_result.hpp"

namespace TrRouting {

  class CommonParameters;
  class CompactConnectionSet;
  class CompactTransfers;

  /**
   * @brief One query of a batched forward calculation
   */
  struct BatchedForwardQuery {
    int departureTimeSeconds;
    // As returned by GeoFilter::getAccessibleNodesFootpathsFromPoint, before the walking speed factor
    std::vector<NodeTimeDistance> accessFootpaths;
    std::vector<NodeTimeDistance> egressFootpaths;
    // The trips of these lines are not used, like AlternativeLineFilter
    std::vector<Line::uid_t> excludedLineUids;
  };

  /**
   * @brief Result of one query of a batched forward calculation
   */
  struct BatchedForwardResult {
    // Set when there is no arrival, with the reason Calculator::calculateSingle would throw
    std::optional<NoRoutingReason> noRoutingReason;
    int arrivalTimeSeconds = -1;
    Node::uid_t egressNodeUid = -1;
  };

  /**
   * Forward CSA of many queries at once, returning for each query the same
   * best arrival time and egress node as Calculator::reset followed by
   * Calculator::forwardCalculation, without the journey steps.
   *
   * The connections are scanned once for the whole batch by the batched_csa
   * Rust crate (batched_csa/src/forward.rs). Memory use is proportional to
   * (nodes + trips) * queries.
   *
   * @param connections The connections of the scenario
   * @param transfers The transferable nodes of all nodes
   * @param parameters The parameters shared by all queries: min waiting time,
   * max total travel time, max transfer walking time, max inner time of trip
   * buffer and walking speed factor. The time of trip, origin and destination
   * are ignored.
   * @param queries The queries
   * @return One result per query, in the same order
   */
  std::vector<BatchedForwardResult> batchedForwardCalculation(
    const CompactConnectionSet &connections,
    const CompactTransfers &transfers,
    const CommonParameters &parameters,
    const std::vector<BatchedForwardQuery> &queries
  );

}

#endif // TR_BATCHED_FORWARD_CALCULATION
