
#ifndef TR_PLACES_CACHE_FETCHER
#define TR_PLACES_CACHE_FETCHER

#include <string>
#include <vector>
#include <fcntl.h>
#include <boost/uuid/string_generator.hpp>
#include <capnp/serialize-packed.h>
#include "cache_fetcher.hpp"
#include "place.hpp"
#include "point.hpp"
#include "capnp/placeCollection.capnp.h"
#include "capnp/place.capnp.h"
#include "spdlog/spdlog.h"

namespace TrRouting
{

  int CacheFetcher::getPlaces(
    std::vector<std::unique_ptr<Place>>& ts,
    std::map<boost::uuids::uuid, int>& tIndexesByUuid,
    const std::map<boost::uuids::uuid, int>& dataSourceIndexesByUuid,
    const std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
    std::string customPath
  )
  {

    using T           = Place;
    using TCollection = placeCollection::PlaceCollection;
    using cT          = place::Place;

    std::string tStr  = "places";
    std::string TStr  = "Places";

    std::string cacheFileName{tStr};
    boost::uuids::string_generator uuidGenerator;

    spdlog::info("Fetching {} from cache... {}", tStr, customPath);

    for(std::map<boost::uuids::uuid, int>::const_iterator iter = dataSourceIndexesByUuid.begin(); iter != dataSourceIndexesByUuid.end(); ++iter)
    {
      boost::uuids::uuid dataSourceUuid = iter->first;

      std::string cacheFilePath {"dataSources/" + boost::uuids::to_string(dataSourceUuid) + "/" + cacheFileName};

      int filesCount {CacheFetcher::getCacheFilesCount(getFilePath(cacheFilePath + ".capnpbin.count", customPath))};

      spdlog::info("files count places: {} path: {}", filesCount, cacheFilePath);

      for (int i = 0; i < filesCount; i++)
      {
        std::string filePath {cacheFilePath + ".capnpbin" + (filesCount > 1 ? "." + std::to_string(i) : "")};
        std::string cacheFilePath = getFilePath(filePath, customPath);

        int fd = open(cacheFilePath.c_str(), O_RDWR);
        if (fd < 0)
        {
          int err = errno;
          if (err == ENOENT)
          {
            spdlog::error("missing {} cache files!", filePath);
          }
          else
          {
            spdlog::error("Error opening cache file {} : {} ", filePath, err);
          }
          continue;
        }

        try
        {
          ::capnp::PackedFdMessageReader capnpTCollectionMessage(fd, {64 * 1024 * 1024});
          TCollection::Reader capnpTCollection = capnpTCollectionMessage.getRoot<TCollection>();
          for (cT::Reader capnpT : capnpTCollection.getPlaces())
          {
            std::string uuid           {capnpT.getUuid()};
            std::string dataSourceUuid {capnpT.getDataSourceUuid()};

            std::unique_ptr<T> t         = std::make_unique<T>();
            std::unique_ptr<Point> point = std::make_unique<Point>();

            t->uuid            = uuidGenerator(uuid);
            t->id              = capnpT.getId();
            t->shortname       = capnpT.getShortname();
            t->name            = capnpT.getName();
            t->internalId      = capnpT.getInternalId();
            t->dataSourceIdx   = dataSourceUuid.length() > 0 ? dataSourceIndexesByUuid.at(uuidGenerator(dataSourceUuid)) : -1;

            point->latitude  = ((double)capnpT.getLatitude())  / 1000000.0;
            point->longitude = ((double)capnpT.getLongitude()) / 1000000.0;
            t->point         = std::move(point);

            const unsigned int nodesCount {capnpT.getNodesUuids().size()};
            std::vector<int> nodesIdx(nodesCount);
            std::vector<int> nodesTravelTimesSeconds(nodesCount);
            std::vector<int> nodesDistancesMeters(nodesCount);
            for (int i = 0; i < nodesCount; i++)
            {
              std::string nodeUuid {capnpT.getNodesUuids()[i]};
              nodesIdx               [i] = nodeIndexesByUuid.at(uuidGenerator(nodeUuid));
              nodesTravelTimesSeconds[i] = capnpT.getNodesTravelTimes()[i];
              nodesDistancesMeters   [i] = capnpT.getNodesDistances()[i];
            }
            t->nodesIdx                = nodesIdx;
            t->nodesTravelTimesSeconds = nodesTravelTimesSeconds;
            t->nodesDistancesMeters    = nodesDistancesMeters;

            tIndexesByUuid[t->uuid] = ts.size();
            ts.push_back(std::move(t));
          }
        }
        catch (const kj::Exception& e)
        {
          spdlog::error("Error opening cache file {}: {}", filePath, e.getDescription().cStr());
        }
        catch (...)
        {
          spdlog::error("Unknown error occurred {} ", filePath);
        }

        close(fd);

      }
    }
    return 0;
  }

}

#endif // TR_PLACES_CACHE_FETCHER
