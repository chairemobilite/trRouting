#include <errno.h>
#include <filesystem>

#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "cache_fetcher.hpp"
#include "cache_fetcher_test.hpp"
#include "node.hpp"
#include "capnp/nodeCollection.capnp.h"
#include "capnp/node.capnp.h"

namespace fs = std::filesystem;

class NodeCacheFetcherFixtureTests : public BaseCacheFetcherFixtureTests
{
protected:
    std::vector<std::unique_ptr<TrRouting::Node>>     nodes;
    std::map<boost::uuids::uuid, int>        nodeIndexesByUuid;
    std::map<boost::uuids::uuid, int>        stationIndexesByUuid;

public:
    void SetUp( ) override
    {
        BaseCacheFetcherFixtureTests::SetUp();
        // Copy the invalid file
        fs::copy_file(BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/genericInvalid.capnpbin", BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/nodes.capnpbin");
        // Create a nodes directory and create invalid files with same names as valid ones
        fs::create_directory(BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/nodes");
    }

    void TearDown( ) override
    {
        BaseCacheFetcherFixtureTests::TearDown();
        // Remove the invalid file and nodes directory
        fs::remove(BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/nodes.capnpbin");
        fs::remove_all(BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/nodes");
    }
};

TEST_F(NodeCacheFetcherFixtureTests, TestGetNodesInvalidNodesFile)
{
    int retVal = cacheFetcher.getNodes(nodes, nodeIndexesByUuid, stationIndexesByUuid, INVALID_CUSTOM_PATH);
    ASSERT_EQ(-EBADMSG, retVal);
    ASSERT_EQ(0, nodes.size());
}

TEST_F(NodeCacheFetcherFixtureTests, TestGetUnexistingSingleNodeFile)
{
    // Copy the file from the valid nodes, but have nodes be unexisting
    fs::copy_file(BASE_CACHE_DIRECTORY_NAME + "/" + VALID_CUSTOM_PATH + "/nodes.capnpbin", BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/nodes.capnpbin", fs::copy_options::overwrite_existing);
    int retVal = cacheFetcher.getNodes(nodes, nodeIndexesByUuid, stationIndexesByUuid, INVALID_CUSTOM_PATH);
    // TODO: Is this right?
    ASSERT_EQ(0, retVal);
    ASSERT_EQ(11, nodes.size());
}

TEST_F(NodeCacheFetcherFixtureTests, TestGetSingleInvalidNodeFile)
{
    // Copy the file from the valid nodes, but have invalid node files
    fs::copy_file(BASE_CACHE_DIRECTORY_NAME + "/" + VALID_CUSTOM_PATH + "/nodes.capnpbin", BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/nodes.capnpbin", fs::copy_options::overwrite_existing);
    for(auto& p: fs::directory_iterator(BASE_CACHE_DIRECTORY_NAME + "/" + VALID_CUSTOM_PATH + "/nodes"))
    {
        // Copy the invalid file for each node name
        fs::copy_file(BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/genericInvalid.capnpbin", BASE_CACHE_DIRECTORY_NAME + "/" + INVALID_CUSTOM_PATH + "/nodes/" + p.path().filename().c_str());
    }
    int retVal = cacheFetcher.getNodes(nodes, nodeIndexesByUuid, stationIndexesByUuid, INVALID_CUSTOM_PATH);
    ASSERT_EQ(-EBADMSG, retVal);
}

TEST_F(NodeCacheFetcherFixtureTests, TestGetNodesValid)
{
    int retVal = cacheFetcher.getNodes(nodes, nodeIndexesByUuid, stationIndexesByUuid, VALID_CUSTOM_PATH);
    ASSERT_EQ(0, retVal);
    ASSERT_EQ(11, nodes.size());
}

TEST_F(NodeCacheFetcherFixtureTests, TestGetNodesFileNotExists)
{
    int retVal = cacheFetcher.getNodes(nodes, nodeIndexesByUuid, stationIndexesByUuid, BASE_CUSTOM_PATH);
    ASSERT_EQ(-ENOENT, retVal);
    ASSERT_EQ(0, nodes.size());
}

