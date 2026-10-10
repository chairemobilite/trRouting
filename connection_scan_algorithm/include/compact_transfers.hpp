#ifndef TR_COMPACT_TRANSFERS
#define TR_COMPACT_TRANSFERS

#include <cstdint>
#include <map>
#include <vector>
#include <boost/uuid/uuid.hpp>

namespace TrRouting {

  class Node;

  /**
   * @brief Node::transferableNodes of all nodes as flat arrays of 32-bit
   * integers, in compressed sparse row (CSR) form indexed by node uid: the
   * transfers from the node with uid n are at indexes [offsets[n], offsets[n + 1])
   * of nodeUid, time and distance, in the order of Node::transferableNodes.
   *
   * Contiguous arrays let the batched CSA relax the transfers for many
   * queries at once, and can be copied as is to a GPU.
   */
  class CompactTransfers {
  public:
    /**
     * @param nodes The nodes of the transit data, with their transferable nodes
     */
    explicit CompactTransfers(const std::map<boost::uuids::uuid, Node> &nodes);

    std::vector<int32_t> offsets; // Node::getMaxUid() + 2 entries
    std::vector<int32_t> nodeUid; // Transferable node
    std::vector<int32_t> time; // Walking time in seconds
    std::vector<int32_t> distance; // Walking distance in meters
  };

}

#endif // TR_COMPACT_TRANSFERS
