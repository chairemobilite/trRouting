
#ifndef TR_NODES_CACHE_FETCHER
#define TR_NODES_CACHE_FETCHER

#include <string>
#include <vector>

#include "cache_fetcher.hpp"
#include "node.hpp"
#include "point.hpp"
#include "capnp/nodeCollection.capnp.h"
#include "capnp/node.capnp.h"
//#include "toolbox.hpp"

namespace TrRouting
{

  void CacheFetcher::getNodes(
    std::vector<std::unique_ptr<Node>>& ts,
    std::map<boost::uuids::uuid, int>& tIndexesByUuid,
    std::map<boost::uuids::uuid, int>& stationIndexesByUuid,
    Parameters& params,
    std::string customPath
  ) { 

    using T           = Node;
    using TCollection = nodeCollection::NodeCollection;
    using cT          = nodeCollection::Node;
    using cNode       = node::Node;

    ts.clear();
    tIndexesByUuid.clear();

    std::string tStr  = "nodes";
    std::string TStr  = "Nodes";

    std::string cacheFileName{tStr};
    std::string nodeCacheFileName{""};
    boost::uuids::string_generator uuidGenerator;

    std::cout << "Fetching " << tStr << " from cache..." << std::endl;
    
    if (CacheFetcher::capnpCacheFileExists(cacheFileName + ".capnpbin", params, customPath))
    {
      int fd = open((CacheFetcher::getFilePath(cacheFileName, params, customPath) + ".capnpbin").c_str(), O_RDWR);
      ::capnp::PackedFdMessageReader capnpTCollectionMessage(fd, {64 * 1024 * 1024});
      TCollection::Reader capnpTCollection = capnpTCollectionMessage.getRoot<TCollection>();
      for (cT::Reader capnpT : capnpTCollection.getNodes())
      {
        std::string uuid        {capnpT.getUuid()};
        std::string stationUuid {capnpT.getStationUuid()};

        std::unique_ptr<Point> point = std::make_unique<Point>();
        std::unique_ptr<T> t         = std::make_unique<T>();

        t->uuid       = uuidGenerator(uuid);
        t->id         = capnpT.getId();
        t->code       = capnpT.getCode();
        t->name       = capnpT.getName();
        t->internalId = capnpT.getInternalId();

        if (stationUuid.length() > 0 && stationIndexesByUuid.count(uuidGenerator(stationUuid)) != 0)
        {
          t->stationIdx = stationIndexesByUuid[uuidGenerator(stationUuid)];
        }
        else
        {
          t->stationIdx = -1;
        }
        point->latitude  = ((double)capnpT.getLatitude())  / 1000000.0;
        point->longitude = ((double)capnpT.getLongitude()) / 1000000.0;
        t->point         = std::move(point);

        nodeCacheFileName = "nodes/node_" + boost::uuids::to_string(t->uuid);
        if (CacheFetcher::capnpCacheFileExists(nodeCacheFileName + ".capnpbin", params, customPath))
        {
          int fd = open((CacheFetcher::getFilePath(nodeCacheFileName, params, customPath) + ".capnpbin").c_str(), O_RDWR);
          ::capnp::PackedFdMessageReader capnpTMessage(fd, {32 * 1024 * 1024});
          cNode::Reader capnpT = capnpTMessage.getRoot<cNode>();
          const unsigned int transferableNodesCount {capnpT.getTransferableNodesIdx().size()};

          std::vector<int> transferableNodesIdx(transferableNodesCount);
          std::vector<int> transferableTravelTimesSeconds(transferableNodesCount);
          //std::vector<int> transferableDistancesMeters(transferableNodesCount);
          for (int i = 0; i < transferableNodesCount; i++)
          {
            transferableNodesIdx          [i] = capnpT.getTransferableNodesIdx()[i];
            transferableTravelTimesSeconds[i] = capnpT.getTransferableNodesTravelTimes()[i];
            //transferableDistancesMeters   [i] = capnpT.getTransferableNodesDistances()[i];
          }
          t->transferableNodesIdx           = transferableNodesIdx;
          t->transferableTravelTimesSeconds = transferableTravelTimesSeconds;
          //t->transferableDistancesMeters    = transferableDistancesMeters;
          close(fd);
        }

        tIndexesByUuid[t->uuid] = ts.size();
        ts.push_back(std::move(t));
        
      }
      //std::cout << TStr << ":\n" << Toolbox::prettyPrintStructVector(ts) << std::endl;
      close(fd);
    }
    else
    {
      std::cerr << "missing " << tStr << " cache file!" << std::endl;
    }
  }

}

#endif // TR_NODES_CACHE_FETCHER