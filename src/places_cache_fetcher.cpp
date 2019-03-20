
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

  const std::pair<std::vector<Place>, std::map<boost::uuids::uuid, int>> CacheFetcher::getPlaces(std::map<boost::uuids::uuid, int> dataSourceIndexesByUuid, Parameters& params)
  { 

    using T           = Place;
    using TCollection = placeCollection::PlaceCollection;
    using cT          = place::Place;

    std::string tStr  = "places";
    std::string TStr  = "Places";

    std::vector<T> ts;
    std::string cacheFileName{tStr};
    std::map<boost::uuids::uuid, int> tIndexesByUuid;
    boost::uuids::string_generator uuidGenerator;

    std::cout << "Fetching " << tStr << " from cache..." << std::endl;

    for(std::map<boost::uuids::uuid, int>::iterator iter = dataSourceIndexesByUuid.begin(); iter != dataSourceIndexesByUuid.end(); ++iter)
    {
      boost::uuids::uuid dataSourceUuid = iter->first;

      std::string cacheFilePath {"dataSources/" + boost::uuids::to_string(dataSourceUuid) + "/places/" + cacheFileName};

      int filesCount {CacheFetcher::getCacheFilesCount(cacheFilePath + ".capnpbin.count", params)};

      std::cout << "files count places: " << filesCount << " path: " << cacheFilePath << std::endl;

      for (int i = 0; i < filesCount; i++)
      {
        std::string filePath {cacheFilePath + ".capnpbin" + (filesCount > 1 ? "." + std::to_string(i) : "")};

        if (CacheFetcher::capnpCacheFileExists(filePath, params))
        {
          int fd = open((CacheFetcher::getFilePath(filePath, params)).c_str(), O_RDWR);
          ::capnp::PackedFdMessageReader capnpTCollectionMessage(fd, {64 * 1024 * 1024});
          TCollection::Reader capnpTCollection = capnpTCollectionMessage.getRoot<TCollection>();
          for (cT::Reader capnpT : capnpTCollection.getPlaces())
          {
            std::string uuid           {capnpT.getUuid()};
            std::string dataSourceUuid {capnpT.getDataSourceUuid()};
            Point * point      = new Point();
            T     * t          = new T();
            t->uuid            = uuidGenerator(uuid);
            t->id              = capnpT.getId();
            t->shortname       = capnpT.getShortname();
            t->name            = capnpT.getName();
            t->osmFeatureKey   = capnpT.getOsmFeatureKey();
            t->osmFeatureValue = capnpT.getOsmFeatureValue();
            t->internalId      = capnpT.getInternalId();
            t->dataSourceIdx   = dataSourceUuid.length() > 0 ? dataSourceIndexesByUuid[uuidGenerator(dataSourceUuid)] : -1;
            t->point           = *point;
            t->point.latitude  = ((double)capnpT.getLatitude())  / 1000000.0;
            t->point.longitude = ((double)capnpT.getLongitude()) / 1000000.0;

            const unsigned int nodesCount {capnpT.getNodesIdx().size()};
            std::vector<int> nodesIdx(nodesCount);
            std::vector<int> nodesTravelTimesSeconds(nodesCount);
            std::vector<int> nodesDistancesMeters(nodesCount);
            for (int i = 0; i < nodesCount; i++)
            {
              nodesIdx               [i] = capnpT.getNodesIdx()[i];
              nodesTravelTimesSeconds[i] = capnpT.getNodesTravelTimes()[i];
              nodesDistancesMeters   [i] = capnpT.getNodesDistances()[i];
            }
            t->nodesIdx                = nodesIdx;
            t->nodesTravelTimesSeconds = nodesTravelTimesSeconds;
            t->nodesDistancesMeters    = nodesDistancesMeters;

            ts.push_back(*t);
            tIndexesByUuid[t->uuid] = ts.size() - 1;
          }
          //std::cout << TStr << ":\n" << Toolbox::prettyPrintStructVector(ts) << std::endl;
          close(fd);
        }
        else
        {
          std::cerr << "missing " << filePath << " cache file!" << std::endl;
        }

      }
    }
    
    return std::make_pair(ts, tIndexesByUuid);
  }

}

#endif // TR_PLACES_CACHE_FETCHER