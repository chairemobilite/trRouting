
#ifndef TR_PATHS_CACHE_FETCHER
#define TR_PATHS_CACHE_FETCHER

#include <string>
#include <vector>

#include "cache_fetcher.hpp"
#include "path.hpp"
#include "capnp/pathCollection.capnp.h"
//#include "toolbox.hpp"

namespace TrRouting
{

  void CacheFetcher::getPaths(
    std::vector<std::unique_ptr<Path>>& ts,
    std::map<boost::uuids::uuid, int>& tIndexesByUuid,
    std::map<boost::uuids::uuid, int>& lineIndexesByUuid,
    std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
    Parameters& params,
    std::string customPath
  ) { 

    using T           = Path;
    using TCollection = pathCollection::PathCollection;
    using cT          = pathCollection::Path;

    ts.clear();
    tIndexesByUuid.clear();

    std::string tStr  = "paths";
    std::string TStr  = "Paths";

    std::string cacheFileName{tStr};
    boost::uuids::string_generator uuidGenerator;
    
    std::cout << "Fetching " << tStr << " from cache..." << std::endl;
    
    if (CacheFetcher::capnpCacheFileExists(cacheFileName + ".capnpbin", params, customPath))
    {
      int fd = open((CacheFetcher::getFilePath(cacheFileName, params, customPath) + ".capnpbin").c_str(), O_RDWR);
      ::capnp::PackedFdMessageReader capnpTCollectionMessage(fd, {64 * 1024 * 1024});
      TCollection::Reader capnpTCollection = capnpTCollectionMessage.getRoot<TCollection>();
      for (cT::Reader capnpT : capnpTCollection.getPaths())
      {
        std::string uuid     {capnpT.getUuid()};
        std::string lineUuid {capnpT.getLineUuid()};
        std::vector<int> nodesIdx;
        std::vector<int> tripsIdx;
        boost::uuids::uuid nodeUuid;
        
        std::unique_ptr<T> t = std::make_unique<T>();

        t->uuid                   = uuidGenerator(uuid);
        t->direction              = capnpT.getDirection();
        t->internalId             = capnpT.getInternalId();
        t->lineIdx                = lineIndexesByUuid[uuidGenerator(lineUuid)];
        t->tripsIdx               = tripsIdx;
        for (std::string nodeUuidStr : capnpT.getNodesUuids())
        {
          nodeUuid = uuidGenerator(nodeUuidStr);
          nodesIdx.push_back(nodeIndexesByUuid[nodeUuid]);
        }
        t->nodesIdx = nodesIdx;

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

#endif // TR_PATHS_CACHE_FETCHER