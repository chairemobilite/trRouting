
#ifndef TR_PATHS_CACHE_FETCHER
#define TR_PATHS_CACHE_FETCHER

#include <string>
#include <vector>
#include <errno.h>
#include <fcntl.h>
#include <kj/exception.h>
#include <boost/uuid/string_generator.hpp>
#include <capnp/serialize-packed.h>
#include "cache_fetcher.hpp"
#include "path.hpp"
#include "capnp/pathCollection.capnp.h"
#include "json.hpp"
#include "spdlog/spdlog.h"

namespace TrRouting
{

  int CacheFetcher::getPaths(
    std::vector<std::unique_ptr<Path>>& ts,
    std::map<boost::uuids::uuid, int>& tIndexesByUuid,
    std::map<boost::uuids::uuid, int>& lineIndexesByUuid,
    std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
    Parameters& params,
    std::string customPath
  )
  {

    using T           = Path;
    using TCollection = pathCollection::PathCollection;
    using cT          = pathCollection::Path;
    int ret = 0;

    ts.clear();
    tIndexesByUuid.clear();

    std::string tStr  = "paths";
    std::string TStr  = "Paths";

    std::string cacheFileName{tStr};
    boost::uuids::string_generator uuidGenerator;
    
    spdlog::info("Fetching {} from cache... {}", tStr, customPath);

    std::string cacheFilePath = CacheFetcher::getFilePath(cacheFileName, params, customPath) + ".capnpbin";
    int fd = open(cacheFilePath.c_str(), O_RDWR);
    if (fd < 0)
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
      ::capnp::PackedFdMessageReader capnpTCollectionMessage(fd, {64 * 1024 * 1024});
      TCollection::Reader capnpTCollection = capnpTCollectionMessage.getRoot<TCollection>();
      for (cT::Reader capnpT : capnpTCollection.getPaths())
      {
        std::string uuid     {capnpT.getUuid()};
        std::string lineUuid {capnpT.getLineUuid()};
        std::vector<int> nodesIdx;
        std::vector<int> tripsIdx;
        std::vector<int> distancesMeters;
        std::vector<int> travelTimesSeconds;
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

        auto jsonData = nlohmann::json::parse(capnpT.getData());

        for (int i=0; i < t->nodesIdx.size(); i++)
        {
          if (jsonData["segments"][i]["distanceMeters"] != nullptr)
          {
            distancesMeters.push_back(jsonData["segments"][i]["distanceMeters"]);
          }
          if (jsonData["segments"][i]["travelTimeSeconds"] != nullptr)
          {
            travelTimesSeconds.push_back(jsonData["segments"][i]["travelTimeSeconds"]);
          }
        }
        
        t->segmentsDistanceMeters    = distancesMeters;
        t->segmentsTravelTimeSeconds = travelTimesSeconds;
        
        tIndexesByUuid[t->uuid] = ts.size();
        ts.push_back(std::move(t));
      }
    }
    catch (const kj::Exception& e)
    {
      spdlog::error("Error opening cache file {}: {}", tStr, e.getDescription().cStr());
      ret = -EBADMSG;
    }
    catch (...)
    {
      spdlog::error("Unknown error occurred {} ", tStr);
      ret = -EINVAL;
    }

    close(fd);
    return ret;
  }

}

#endif // TR_PATHS_CACHE_FETCHER
