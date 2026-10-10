#include "gtest/gtest.h"
#include "connection_set_test.hpp"
#include "compact_transfers.hpp"
#include "node.hpp"

// The CSR range of each node must match its transferable nodes, in the same order
TEST_F(ConnectionSetFixtureTests, TestCompactTransfersMatchTransferableNodes)
{
    TrRouting::CompactTransfers compact(transitData.getNodes());

    ASSERT_EQ((size_t)TrRouting::Node::getMaxUid() + 2, compact.offsets.size());
    size_t transfersCount = 0;
    for (const auto &[uuid, node] : transitData.getNodes()) {
        SCOPED_TRACE("node " + std::to_string(node.uid));
        const int32_t begin = compact.offsets[node.uid];
        ASSERT_EQ(node.transferableNodes.size(), (size_t)(compact.offsets[node.uid + 1] - begin));
        for (size_t i = 0; i < node.transferableNodes.size(); i++) {
            EXPECT_EQ(node.transferableNodes[i].node.uid, compact.nodeUid[begin + i]);
            EXPECT_EQ(node.transferableNodes[i].time, compact.time[begin + i]);
            EXPECT_EQ(node.transferableNodes[i].distance, compact.distance[begin + i]);
        }
        transfersCount += node.transferableNodes.size();
    }
    ASSERT_GT(transfersCount, 0u);
    EXPECT_EQ(transfersCount, compact.nodeUid.size());
}
