
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
    std::map<boost::uuids::uuid, Path>& ts,
    const std::map<boost::uuids::uuid, Line>& lines,
    const std::map<boost::uuids::uuid, Node>& nodes,
    std::string customPath
  )
  {

    using T           = Path;
    using TCollection = pathCollection::PathCollection;
    using cT          = pathCollection::Path;
    int ret = 0;

    ts.clear();

    std::string tStr  = "paths";
    std::string TStr  = "Paths";

    std::string cacheFileName{tStr};
    boost::uuids::string_generator uuidGenerator;
    
    spdlog::info("Fetching {} from cache... {}", tStr, customPath);

    std::string cacheFilePath = getFilePath(cacheFileName, customPath) + ".capnpbin";
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
        std::vector<std::reference_wrapper<const Node>> nodesRef;
        std::vector<std::reference_wrapper<const Trip>> tripsRef;
        std::vector<int> distancesMeters;
        std::vector<int> travelTimesSeconds;
        boost::uuids::uuid nodeUuid;
        boost::uuids::uuid pathUuid = uuidGenerator(uuid);
        for (std::string nodeUuidStr : capnpT.getNodesUuids())
        {
          nodeUuid = uuidGenerator(nodeUuidStr);
          nodesRef.push_back(nodes.at(nodeUuid));
        }

        auto jsonData = nlohmann::json::parse(capnpT.getData());

        for (int i=0; i < nodesRef.size(); i++)
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
        
        ts.emplace(pathUuid, T(pathUuid,
                               lines.at(uuidGenerator(lineUuid)),
                               capnpT.getDirection(),
                               capnpT.getInternalId(),
                               nodesRef,
                               tripsRef, //TODO This is empty
                               travelTimesSeconds,
                               distancesMeters));
      }
    }
    catch (const kj::Exception& e)
    {
      spdlog::error("Error opening cache file {}: {}", tStr, e.getDescription().cStr());
      ret = -EBADMSG;
    }
    catch (const std::exception& e)
    {
      spdlog::error("Unknown error occurred {} {}", tStr, e.what());
      ret = -EINVAL;
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
