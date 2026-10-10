#ifndef TR_COMPACT_CONNECTION_SET
#define TR_COMPACT_CONNECTION_SET

#include <cstdint>
#include <vector>

namespace TrRouting {

  class ConnectionSet;

  /**
   * @brief Forward connections of a ConnectionSet as flat arrays of 32-bit
   * integers (one array per field), in the order of
   * ConnectionSet::getForwardConnections().
   *
   * Contiguous arrays let the batched CSA scan the connections for many
   * queries at once, and can be copied as is to a GPU.
   *
   * Nodes and lines are identified by their uid. Trips are identified by a
   * dense index in ConnectionSet::getTrips(), because Trip uids span all the
   * trips of the transit data, not only the ones of the set.
   */
  class CompactConnectionSet {
  public:
    static constexpr uint32_t CAN_BOARD = 1;
    static constexpr uint32_t CAN_UNBOARD = 2;

    /**
     * @param connectionSet The connections to copy
     */
    explicit CompactConnectionSet(const ConnectionSet &connectionSet);

    // One entry per forward connection
    std::vector<int32_t> departureNodeUid;
    std::vector<int32_t> arrivalNodeUid;
    std::vector<int32_t> departureTime;
    std::vector<int32_t> arrivalTime;
    std::vector<int32_t> tripIndex;
    std::vector<int32_t> minWaitingTime; // -1 to use the default of the query
    std::vector<uint32_t> flags; // Combination of CAN_BOARD and CAN_UNBOARD

    // One entry per trip, indexed by tripIndex
    std::vector<int32_t> tripLineUid;
  };

}

#endif // TR_COMPACT_CONNECTION_SET
