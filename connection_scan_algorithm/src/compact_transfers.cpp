#include <numeric>

#include "compact_transfers.hpp"
#include "node.hpp"

namespace TrRouting {

  CompactTransfers::CompactTransfers(const std::map<boost::uuids::uuid, Node> &nodes) {
    // Count the transfers of each node, then accumulate the counts into offsets
    offsets.assign(Node::getMaxUid() + 2, 0);
    for (const auto &[uuid, node] : nodes) {
      offsets[node.uid + 1] = node.transferableNodes.size();
    }
    std::partial_sum(offsets.begin(), offsets.end(), offsets.begin());

    nodeUid.resize(offsets.back());
    time.resize(offsets.back());
    distance.resize(offsets.back());
    for (const auto &[uuid, node] : nodes) {
      int32_t index = offsets[node.uid];
      for (const NodeTimeDistance &transferableNode : node.transferableNodes) {
        nodeUid[index] = transferableNode.node.uid;
        time[index] = transferableNode.time;
        distance[index] = transferableNode.distance;
        index++;
      }
    }
  }

}
