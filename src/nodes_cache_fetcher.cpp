
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

  const std::pair<std::vector<Node>, std::map<boost::uuids::uuid, int>> CacheFetcher::getNodes(std::map<boost::uuids::uuid, int> stationIndexesByUuid, Parameters& params)
  { 

    using T           = Node;
    using TCollection = nodeCollection::NodeCollection;
    using cT          = nodeCollection::Node;

    std::string tStr  = "nodes";
    std::string TStr  = "Nodes";

    std::vector<T> ts;
    std::string cacheFileName{tStr};
    std::map<boost::uuids::uuid, int> tIndexesByUuid;
    boost::uuids::string_generator uuidGenerator;

    std::cout << "Fetching " << tStr << " from cache..." << std::endl;
    if (CacheFetcher::capnpCacheFileExists(cacheFileName, params))
    {
      int fd = open((params.cacheDirectoryPath + params.projectShortname + "/" + cacheFileName + ".capnpbin").c_str(), O_RDWR);
      ::capnp::PackedFdMessageReader capnpTCollectionMessage(fd, {64 * 1024 * 1024});
      TCollection::Reader capnpTCollection = capnpTCollectionMessage.getRoot<TCollection>();
      for (cT::Reader capnpT : capnpTCollection.getNodes())
      {
        std::string uuid        {capnpT.getUuid()};
        std::string stationUuid {capnpT.getStationUuid()};
        Point * point      = new Point();
        T     * t          = new T();
        t->uuid            = uuidGenerator(uuid);
        t->id              = capnpT.getId();
        t->code            = capnpT.getCode();
        t->name            = capnpT.getName();
        t->stationIdx      = stationUuid.length() > 0 ? stationIndexesByUuid[uuidGenerator(stationUuid)] : -1;
        t->point           = *point;
        t->point.latitude  = ((double)capnpT.getLatitude())  / 1000000.0;
        t->point.longitude = ((double)capnpT.getLongitude()) / 1000000.0;
        ts.push_back(*t);
        tIndexesByUuid[t->uuid] = ts.size() - 1;
      }
      //std::cout << TStr << ":\n" << Toolbox::prettyPrintStructVector(ts) << std::endl;
      close(fd);
    }
    else
    {
      std::cerr << "missing " << tStr << " cache file!" << std::endl;
    }
    return std::make_pair(ts, tIndexesByUuid);
  }

  const std::vector<Node> CacheFetcher::getNodeFootpaths(std::vector<Node> nodes, std::map<boost::uuids::uuid, int> nodeIndexesByUuid, Parameters& params)
  { 

    using T           = Node;
    using cT          = node::Node;

    std::string tStr  = "nodes footpaths";
    std::string TStr  = "Nodes";

    std::vector<T> ts;
    std::string cacheFileName;
    std::map<boost::uuids::uuid, int> tIndexesByUuid;
    boost::uuids::string_generator uuidGenerator;

    std::cout << "Fetching " << tStr << " from cache..." << std::endl;

    for (auto & node : nodes)
    {
      cacheFileName = "nodes/node_" + boost::uuids::to_string(node.uuid);
      if (CacheFetcher::capnpCacheFileExists(cacheFileName, params))
      {
        int fd = open((params.cacheDirectoryPath + params.projectShortname + "/" + cacheFileName + ".capnpbin").c_str(), O_RDWR);
        ::capnp::PackedFdMessageReader capnpTMessage(fd, {32 * 1024 * 1024});
        cT::Reader capnpT = capnpTMessage.getRoot<cT>();
        const unsigned int transferableNodesCount {capnpT.getTransferableNodesIdx().size()};
        std::vector<int> transferableNodesIdx(transferableNodesCount);
        std::vector<int> transferableTravelTimesSeconds(transferableNodesCount);
        std::vector<int> transferableDistancesMeters(transferableNodesCount);
        for (int i = 0; i < transferableNodesCount; i++)
        {
          transferableNodesIdx          [i] = capnpT.getTransferableNodesIdx()[i];
          transferableTravelTimesSeconds[i] = capnpT.getTransferableNodesTravelTimes()[i];
          transferableDistancesMeters   [i] = capnpT.getTransferableNodesDistances()[i];
        }
        node.transferableNodesIdx           = transferableNodesIdx;
        node.transferableTravelTimesSeconds = transferableTravelTimesSeconds;
        node.transferableDistancesMeters    = transferableDistancesMeters;
        close(fd);
      }
      else
      {
        std::cerr << "missing " << tStr << " cache file for node " << boost::uuids::to_string(node.uuid) << " !" << std::endl;
      }
    }
    return nodes;
  }

}

#endif // TR_NODES_CACHE_FETCHER