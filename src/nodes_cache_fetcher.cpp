
#ifndef TR_NODES_CACHE_FETCHER
#define TR_NODES_CACHE_FETCHER

#include <string>
#include <vector>
#include <numeric>


#include "cache_fetcher.hpp"
#include "node.hpp"
#include "point.hpp"
#include "capnp/nodeCollection.capnp.h"
#include "capnp/node.capnp.h"
#include "calculation_time.hpp"
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

        //std::cout << TStr << ":\n" << t->toString() << std::endl;

        tIndexesByUuid[t->uuid] = ts.size();
        ts.push_back(std::move(t));
        
      }

      /*CalculationTime algorithmCalculationTime = CalculationTime();
      algorithmCalculationTime.start();
      long long       calculationTime;
      calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();*/

      auto nodesCount {ts.size()};
      //std::vector<int>::iterator nodeIndex;
      // find reverse transferable nodes:
      //std::cout << "-- start node with nodesCount -- " << nodesCount << "\n";
      for (int i = 0; i < nodesCount; i++)
      {

        Node * t = ts[i].get();

        nodeCacheFileName = "nodes/node_" + boost::uuids::to_string(t->uuid);
        if (CacheFetcher::capnpCacheFileExists(nodeCacheFileName + ".capnpbin", params, customPath))
        {
          int fd = open((CacheFetcher::getFilePath(nodeCacheFileName, params, customPath) + ".capnpbin").c_str(), O_RDWR);
          ::capnp::PackedFdMessageReader capnpTMessage(fd, {32 * 1024 * 1024});
          cNode::Reader capnpT = capnpTMessage.getRoot<cNode>();
          const unsigned int transferableNodesCount {capnpT.getTransferableNodesUuids().size()};

          std::vector<int> transferableNodesIdx(transferableNodesCount);
          std::vector<int> transferableTravelTimesSeconds(transferableNodesCount);
          std::vector<int> transferableDistancesMeters(transferableNodesCount);
          std::vector<int> reverseTransferableNodesIdx(transferableNodesCount);
          std::vector<int> reverseTransferableTravelTimesSeconds(transferableNodesCount);
          std::vector<int> reverseTransferableDistancesMeters(transferableNodesCount);

          for (int j = 0; j < transferableNodesCount; j++)
          {
            std::string nodeUuid {capnpT.getTransferableNodesUuids()[j]};
            transferableNodesIdx          [j] = tIndexesByUuid[uuidGenerator(nodeUuid)];
            transferableTravelTimesSeconds[j] = capnpT.getTransferableNodesTravelTimes()[j];
            transferableDistancesMeters   [j] = capnpT.getTransferableNodesDistances()[j];
          }
          t->transferableNodesIdx           = transferableNodesIdx;
          t->transferableTravelTimesSeconds = transferableTravelTimesSeconds;
          t->transferableDistancesMeters    = transferableDistancesMeters;

          // put same node transfer at the beginning:
          t->reverseTransferableNodesIdx.push_back(i);
          t->reverseTransferableTravelTimesSeconds.push_back(0);
          t->reverseTransferableDistancesMeters.push_back(0);
          close(fd);
        }

      }

      //std::cout << "generate reverse transferable nodes " << std::endl;

      // generate reverse transferable nodes 
      // travel time is not the same in both direction so we need to reverse these, 
      // especially with modes like car and bicycle which must follow one way and restrictions
      for (int i = 0; i < nodesCount; i++)
      {
        int transferableNodesCount = ts[i]->transferableNodesIdx.size();
        for (int j = 0; j < transferableNodesCount; j++)
        {

          int transferableNodeIdx = ts[i]->transferableNodesIdx[j];

          if (transferableNodeIdx == i) // we already put same node transfer at the beginning
          {
            continue;
          }

          ts[transferableNodeIdx]->reverseTransferableNodesIdx.push_back(i);
          ts[transferableNodeIdx]->reverseTransferableTravelTimesSeconds.push_back(ts[i]->transferableTravelTimesSeconds[j]);
          ts[transferableNodeIdx]->reverseTransferableDistancesMeters.push_back(ts[i]->transferableDistancesMeters[j]);
        }
      }

      //std::cout << "sort by increasing travel times " << std::endl;

      // sort by increasing travel times so we don't get longer transfers when shorter exists:
      for (auto & node : ts)
      {
        
        std::vector<size_t> sortedIdx(node->transferableNodesIdx.size());
        std::iota(sortedIdx.begin(), sortedIdx.end(), 0);
        std::stable_sort(sortedIdx.begin(), sortedIdx.end(),
          [&](int i1, int i2) {
            return node->transferableTravelTimesSeconds[i1] < node->transferableTravelTimesSeconds[i2];
        });

        std::vector<int> transferableNodesIdx(node->transferableNodesIdx.size());
        std::vector<int> transferableTravelTimesSeconds(node->transferableNodesIdx.size());
        std::vector<int> transferableDistancesMeters(node->transferableNodesIdx.size());
        transferableNodesIdx           = node->transferableNodesIdx;
        transferableTravelTimesSeconds = node->transferableTravelTimesSeconds;
        transferableDistancesMeters    = node->transferableDistancesMeters;

        for (int i = 0; i < sortedIdx.size(); i++)
        {
          node->transferableNodesIdx[i]           = transferableNodesIdx[sortedIdx[i]];
          node->transferableTravelTimesSeconds[i] = transferableTravelTimesSeconds[sortedIdx[i]];
          node->transferableDistancesMeters[i]    = transferableDistancesMeters[sortedIdx[i]];
        }

        std::vector<size_t> sortedReverseIdx(node->reverseTransferableNodesIdx.size());
        std::iota(sortedReverseIdx.begin(), sortedReverseIdx.end(), 0);
        std::stable_sort(sortedReverseIdx.begin(), sortedReverseIdx.end(),
          [&](int i1, int i2) {
            return node->reverseTransferableTravelTimesSeconds[i1] < node->reverseTransferableTravelTimesSeconds[i2];
        });

        std::vector<int> reverseTransferableNodesIdx(node->reverseTransferableNodesIdx.size());
        std::vector<int> reverseTransferableTravelTimesSeconds(node->reverseTransferableNodesIdx.size());
        std::vector<int> reverseTransferableDistancesMeters(node->reverseTransferableNodesIdx.size());
        reverseTransferableNodesIdx           = node->reverseTransferableNodesIdx;
        reverseTransferableTravelTimesSeconds = node->reverseTransferableTravelTimesSeconds;
        reverseTransferableDistancesMeters    = node->reverseTransferableDistancesMeters;

        for (int i = 0; i < sortedReverseIdx.size(); i++)
        {
          node->reverseTransferableNodesIdx[i]           = reverseTransferableNodesIdx[sortedReverseIdx[i]];
          node->reverseTransferableTravelTimesSeconds[i] = reverseTransferableTravelTimesSeconds[sortedReverseIdx[i]];
          node->reverseTransferableDistancesMeters[i]    = reverseTransferableDistancesMeters[sortedReverseIdx[i]];
        }

      }

      
      //std::cerr << "-- node reverse transferable nodes preparation -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";

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