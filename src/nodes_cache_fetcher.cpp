
#ifndef TR_NODES_CACHE_FETCHER
#define TR_NODES_CACHE_FETCHER

#include <string>
#include <vector>
#include <numeric>
#include <errno.h>
#include <fcntl.h>
#include <kj/exception.h>
#include <boost/uuid/string_generator.hpp>
#include <capnp/serialize-packed.h>
#include "cache_fetcher.hpp"
#include "node.hpp"
#include "point.hpp"
#include "capnp/nodeCollection.capnp.h"
#include "capnp/node.capnp.h"
#include "calculation_time.hpp"
#include "spdlog/spdlog.h"

namespace TrRouting
{
  int CacheFetcher::getNodes(
    std::map<boost::uuids::uuid, Node>& ts,
    std::string customPath
  )
  {

    using T           = Node;
    using TCollection = nodeCollection::NodeCollection;
    using cT          = nodeCollection::Node;
    using cNode       = node::Node;

    ts.clear();

    std::string tStr  = "nodes";
    std::string TStr  = "Nodes";

    std::string cacheFileName{tStr};
    std::string nodeCacheFileName{""};
    std::string nodeCacheFileNamePath{""};
    boost::uuids::string_generator uuidGenerator;

    spdlog::info("Fetching {} from cache... {}", tStr, customPath);

    std::string cacheFilePath = getFilePath(cacheFileName, customPath) + ".capnpbin";

    int cacheFd = open(cacheFilePath.c_str(), O_RDWR);
    if (cacheFd < 0)
    {
      int err = errno;
      if (err == ENOENT)
      {
        spdlog::error("missing {} cache files!", tStr);
      }
      else
      {
        spdlog::error("Error opening cache file {} : {} ", tStr, err);
      }
      return -err;
    }

    try
    {
      ::capnp::PackedFdMessageReader capnpTCollectionMessage(cacheFd, {64 * 1024 * 1024});
      TCollection::Reader capnpTCollection = capnpTCollectionMessage.getRoot<TCollection>();
      for (cT::Reader capnpT : capnpTCollection.getNodes())
      {
        std::string uuid        {capnpT.getUuid()};
        //TODO Station related code was removed, keeping this to highlight the capnp data is ignored
        //std::string stationUuid {capnpT.getStationUuid()};

        std::unique_ptr<Point> point = std::make_unique<Point>();
        point->latitude  = ((double)capnpT.getLatitude())  / 1000000.0;
        point->longitude = ((double)capnpT.getLongitude()) / 1000000.0;

        ts.emplace(uuidGenerator(uuid), T(uuidGenerator(uuid),
                             capnpT.getId(),
                             capnpT.getCode(),
                             capnpT.getName(),
                             capnpT.getInternalId(),
                             std::move(point)));
      }
    }
    catch (const kj::Exception& e)
    {
      spdlog::error("Error opening cache file {}: {}", tStr, e.getDescription().cStr());
      close(cacheFd);
      return -EBADMSG;
    }
    catch (...)
    {
      spdlog::error("Unknown error occurred {} ", tStr);
      close(cacheFd);
      return -EINVAL;
    }

    close(cacheFd);

    /*CalculationTime algorithmCalculationTime = CalculationTime();
    algorithmCalculationTime.start();
    long long       calculationTime;
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();*/

    //std::vector<int>::iterator nodeIndex;
    // find reverse transferable nodes:
    for (auto nodeIter = ts.begin(); nodeIter != ts.end(); nodeIter++)
    {
      Node & t = nodeIter->second;

      nodeCacheFileName = "nodes/node_" + boost::uuids::to_string(t.uuid);
      nodeCacheFileNamePath = getFilePath(nodeCacheFileName, customPath) + ".capnpbin";

      int fd = open(nodeCacheFileNamePath.c_str(), O_RDWR);
      if (fd < 0)
      {
        int err = errno;
        // TODO Do something about missing or faulty cache files?
        spdlog::error("Error opening cache file {}: {}", nodeCacheFileNamePath, err);
        continue;
      }

      try
      {
        ::capnp::PackedFdMessageReader capnpTMessage(fd, {32 * 1024 * 1024});

        cNode::Reader capnpT = capnpTMessage.getRoot<cNode>();
        const unsigned int transferableNodesCount {capnpT.getTransferableNodesUuids().size()};

        std::vector<NodeTimeDistance> transferableNodes;
        std::vector<NodeTimeDistance> reverseTransferableNodes;

        for (unsigned int j = 0; j < transferableNodesCount; j++)
        {
          std::string nodeUuidStr {capnpT.getTransferableNodesUuids()[j]};
          boost::uuids::uuid nodeUuid = uuidGenerator(nodeUuidStr);

          if (ts.count(nodeUuid) == 0) {
            spdlog::error("Invalid transferable node UUID ({}) in file {}", nodeUuidStr, nodeCacheFileNamePath);
            //TODO We might want to do more than just print and continue
            //close(fd);
            //return -EINVAL;
            continue;
          }
          
          int travelTime = capnpT.getTransferableNodesTravelTimes()[j];
          int distance = capnpT.getTransferableNodesDistances()[j];
          transferableNodes.push_back(NodeTimeDistance(ts.at(nodeUuid),
                                                       travelTime,
                                                       distance));

          // Fill in reverse transfer information
          ts.at(nodeUuid).reverseTransferableNodes.push_back(NodeTimeDistance(t,
                                                                              travelTime,
                                                                              distance));
        }
        // Save transferable nodes in object
        t.transferableNodes.clear();
        std::copy(transferableNodes.begin(), transferableNodes.end(), std::back_inserter(t.transferableNodes));

        // put same node transfer at the beginning:
        t.reverseTransferableNodes.push_back(NodeTimeDistance(t, 0, 0));

      }
      catch (const kj::Exception& e)
      {
        spdlog::error("-- Error reading node cache file -- {}: {}", nodeCacheFileNamePath, e.getDescription().cStr());
        close(fd);
        return -EBADMSG;
      }
      close(fd);
    }

    try
    {
      // sort by increasing travel times so we don't get longer transfers when shorter exists:
      for(auto ite = ts.begin(); ite != ts.end(); ite++)
      {
        auto & node = ite->second;
        //TODO We can sort the vector<NodeTimeDistance> directly...
        //TODO Could maybe make it a priority queue/sorted list which is always sorted????
        std::vector<size_t> sortedIdx(node.transferableNodes.size());
        // iota will fill array with a serie of int starting at 0 (0,1,2,3,4,...)
        std::iota(sortedIdx.begin(), sortedIdx.end(), 0);
        std::stable_sort(sortedIdx.begin(), sortedIdx.end(),
          [&](int i1, int i2) {
            return node.transferableNodes.at(i1).time < node.transferableNodes.at(i2).time;
        });

        ////TODO This is what it would look like without the const Node & issues
        ////std::stable_sort(node.transferableNodes.begin(), node.transferableNodes.end(),
        ////[&](const NodeTimeDistance &i1, const NodeTimeDistance & i2) {
        ////return i1.time < i2.time;
        ////});

        std::vector<NodeTimeDistance> transferableNodesSorted;
        for (size_t i = 0; i < sortedIdx.size(); i++)
        {
          transferableNodesSorted.push_back( node.transferableNodes.at(i));
        }
        node.transferableNodes.clear();
        std::copy(transferableNodesSorted.begin(), transferableNodesSorted.end(), std::back_inserter(node.transferableNodes));

        ////TODO This is what it would look like without the const Node & issues
        ////std::stable_sort(node.reverseTransferableNodes.begin(), node.reverseTransferableNodes.end(),
        ////[&](NodeTimeDistance &i1, NodeTimeDistance & i2) {
        ////return i1.time < i2.time;
        ////});

        std::vector<size_t> sortedReverseIdx(node.reverseTransferableNodes.size());
        std::iota(sortedReverseIdx.begin(), sortedReverseIdx.end(), 0);
        std::stable_sort(sortedReverseIdx.begin(), sortedReverseIdx.end(),
          [&](int i1, int i2) {
            return node.reverseTransferableNodes.at(i1).time < node.reverseTransferableNodes.at(i2).time;
        });

        std::vector<NodeTimeDistance> reverseTransferableNodesSorted;

        for (size_t i = 0; i < sortedReverseIdx.size(); i++)
        {
          reverseTransferableNodesSorted.push_back( node.reverseTransferableNodes.at(i));

        }
        node.reverseTransferableNodes.clear();
        std::copy(reverseTransferableNodesSorted.begin(), reverseTransferableNodesSorted.end(), std::back_inserter(node.reverseTransferableNodes));              

      }
  
      return 0;
    }
    catch (const std::exception& ex)
    {
      spdlog::error("-- Error processing node data -- {}", ex.what());
      return -EINVAL;
    }
    
  }

}

#endif // TR_NODES_CACHE_FETCHER
