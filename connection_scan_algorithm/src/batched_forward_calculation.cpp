#include "batched_forward_calculation.hpp"
#include "batched_csa.h"
#include "compact_connection_set.hpp"
#include "compact_transfers.hpp"
#include "parameters.hpp"

namespace TrRouting {

  static_assert(CompactConnectionSet::CAN_BOARD == BATCHED_CSA_CAN_BOARD && CompactConnectionSet::CAN_UNBOARD == BATCHED_CSA_CAN_UNBOARD);

  namespace {

    // Append the footpaths of a query and their end offset
    void appendFootpaths(std::vector<BatchedCsaFootpath> &footpaths, std::vector<int32_t> &offsets, const std::vector<NodeTimeDistance> &queryFootpaths) {
      for (const NodeTimeDistance &footpath : queryFootpaths) {
        footpaths.push_back({footpath.node.uid, footpath.time});
      }
      offsets.push_back(footpaths.size());
    }

  }

  std::vector<BatchedForwardResult> batchedForwardCalculation(
    const CompactConnectionSet &connections,
    const CompactTransfers &transfers,
    const CommonParameters &parameters,
    const std::vector<BatchedForwardQuery> &queries
  ) {
    const BatchedCsaConnections batchConnections {
      connections.departureNodeUid.data(),
      connections.arrivalNodeUid.data(),
      connections.departureTime.data(),
      connections.arrivalTime.data(),
      connections.tripIndex.data(),
      connections.minWaitingTime.data(),
      connections.flags.data(),
      connections.departureTime.size(),
      connections.tripLineUid.data(),
      connections.tripLineUid.size()
    };
    const BatchedCsaTransfers batchTransfers {
      transfers.offsets.data(),
      transfers.offsets.size() - 1,
      transfers.nodeUid.data(),
      transfers.time.data(),
      transfers.nodeUid.size()
    };
    const BatchedCsaParameters batchParameters {
      parameters.getMinWaitingTimeSeconds(),
      parameters.getMaxTotalTravelTimeSeconds(),
      parameters.getMaxTransferWalkingTravelTimeSeconds(),
      parameters.getMaxInnerTimeOfTripBufferSeconds(),
      parameters.getWalkingSpeedFactor()
    };

    // Concatenate the arrays of the queries, with their offsets
    std::vector<int32_t> departureTimes;
    std::vector<BatchedCsaFootpath> access;
    std::vector<BatchedCsaFootpath> egress;
    std::vector<int32_t> excludedLines;
    std::vector<int32_t> accessOffsets {0};
    std::vector<int32_t> egressOffsets {0};
    std::vector<int32_t> excludedLinesOffsets {0};
    for (const BatchedForwardQuery &query : queries) {
      departureTimes.push_back(query.departureTimeSeconds);
      appendFootpaths(access, accessOffsets, query.accessFootpaths);
      appendFootpaths(egress, egressOffsets, query.egressFootpaths);
      excludedLines.insert(excludedLines.end(), query.excludedLineUids.begin(), query.excludedLineUids.end());
      excludedLinesOffsets.push_back(excludedLines.size());
    }
    const BatchedCsaQueries batchQueries {
      queries.size(),
      departureTimes.data(),
      accessOffsets.data(),
      access.data(),
      egressOffsets.data(),
      egress.data(),
      excludedLinesOffsets.data(),
      excludedLines.data()
    };

    std::vector<BatchedCsaForwardResult> batchResults(queries.size());
    batched_csa_forward_calculate(&batchConnections, &batchTransfers, &batchParameters, &batchQueries, batchResults.data());

    std::vector<BatchedForwardResult> results;
    results.reserve(queries.size());
    for (const BatchedCsaForwardResult &batchResult : batchResults) {
      BatchedForwardResult result;
      if (batchResult.no_routing_reason >= 0) {
        result.noRoutingReason = static_cast<NoRoutingReason>(batchResult.no_routing_reason);
      }
      result.arrivalTimeSeconds = batchResult.arrival_time;
      result.egressNodeUid = batchResult.egress_node;
      results.push_back(result);
    }
    return results;
  }

}
