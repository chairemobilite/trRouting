
#ifndef TR_PLACES_CACHE_FETCHER
#define TR_PLACES_CACHE_FETCHER

#include <string>
#include <vector>

#include "cache_fetcher.hpp"
#include "place.hpp"
#include "point.hpp"
#include "capnp/placeCollection.capnp.h"
#include "capnp/place.capnp.h"
//#include "toolbox.hpp"

namespace TrRouting
{

  int CacheFetcher::getPlaces(
    std::vector<std::unique_ptr<Place>>& ts,
    std::map<boost::uuids::uuid, int>& tIndexesByUuid,
    std::map<boost::uuids::uuid, int>& dataSourceIndexesByUuid,
    std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
    Parameters& params,
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

    std::cout << "Fetching " << tStr << " from cache..." << std::endl;

    for(std::map<boost::uuids::uuid, int>::iterator iter = dataSourceIndexesByUuid.begin(); iter != dataSourceIndexesByUuid.end(); ++iter)
    {
      boost::uuids::uuid dataSourceUuid = iter->first;

      std::string cacheFilePath {"dataSources/" + boost::uuids::to_string(dataSourceUuid) + "/" + cacheFileName};

      int filesCount {CacheFetcher::getCacheFilesCount(CacheFetcher::getFilePath(cacheFilePath + ".capnpbin.count", params, customPath))};

      std::cout << "files count places: " << filesCount << " path: " << cacheFilePath << std::endl;

      for (int i = 0; i < filesCount; i++)
      {
        std::string filePath {cacheFilePath + ".capnpbin" + (filesCount > 1 ? "." + std::to_string(i) : "")};
        std::string cacheFilePath = CacheFetcher::getFilePath(filePath, params, customPath);

        int fd = open(cacheFilePath.c_str(), O_RDWR);
        if (fd < 0)
        {
          int err = errno;
          if (err == ENOENT)
          {
            std::cerr << "missing " << filePath << " cache file!" << std::endl;
          }
          else
          {
            std::cerr << "Error opening cache file " << filePath << ": " << err << std::endl;
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
            t->dataSourceIdx   = dataSourceUuid.length() > 0 ? dataSourceIndexesByUuid[uuidGenerator(dataSourceUuid)] : -1;

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
              nodesIdx               [i] = nodeIndexesByUuid[uuidGenerator(nodeUuid)];
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
          std::cerr << "Error reading cache file " << filePath << ": " << e.getDescription().cStr() << std::endl;
        }
        catch (...)
        {
          std::cerr << "Unknown error occurred " << filePath << std::endl;
        }

        close(fd);

      }
    }
    return 0;
  }

}

#endif // TR_PLACES_CACHE_FETCHER