
#ifndef TR_HOUSEHOLDS_CACHE_FETCHER
#define TR_HOUSEHOLDS_CACHE_FETCHER

#include <string>
#include <vector>

#include "cache_fetcher.hpp"
#include "household.hpp"
#include "point.hpp"
#include "capnp/householdCollection.capnp.h"
#include "capnp/household.capnp.h"
//#include "toolbox.hpp"

namespace TrRouting
{

  const std::pair<std::vector<Household>, std::map<boost::uuids::uuid, int>> CacheFetcher::getHouseholds(std::map<boost::uuids::uuid, int> dataSourceIndexesByUuid, Parameters& params)
  { 

    using T           = Household;
    using TCollection = householdCollection::HouseholdCollection;
    using cT          = household::Household;

    std::string tStr  = "households";
    std::string TStr  = "Households";

    std::vector<T> ts;
    std::string cacheFileName{tStr};
    std::map<boost::uuids::uuid, int> tIndexesByUuid;
    boost::uuids::string_generator uuidGenerator;

    std::cout << "Fetching " << tStr << " from cache..." << std::endl;

    std::string cacheFilePath {params.cacheDirectoryPath + params.projectShortname + "/households/" + cacheFileName};

    int filesCount {CacheFetcher::getCacheFilesCount(cacheFilePath, params)};

    for (int i = 0; i < filesCount; i++)
    {
      std::string filePath {cacheFilePath + ".capnpbin" + (filesCount > 1 ? std::to_string(i) : "")};

      if (CacheFetcher::capnpCacheFileExists(cacheFileName, params))
      {
        int fd = open((filePath).c_str(), O_RDWR);
        ::capnp::PackedFdMessageReader capnpTCollectionMessage(fd, {64 * 1024 * 1024});
        TCollection::Reader capnpTCollection = capnpTCollectionMessage.getRoot<TCollection>();
        for (cT::Reader capnpT : capnpTCollection.getHouseholds())
        {
          std::string uuid           {capnpT.getUuid()};
          std::string dataSourceUuid {capnpT.getDataSourceUuid()};
          Point * point      = new Point();
          T     * t          = new T();
          t->uuid            = uuidGenerator(uuid);
          t->id              = capnpT.getId();
          t->dataSourceIdx   = dataSourceUuid.length() > 0 ? dataSourceIndexesByUuid[uuidGenerator(dataSourceUuid)] : -1;
          t->point           = *point;
          t->point.latitude  = ((double)capnpT.getHomeLatitude())  / 1000000.0;
          t->point.longitude = ((double)capnpT.getHomeLongitude()) / 1000000.0;

          const unsigned int homeNodesCount {capnpT.getHomeNodesIdx().size()};
          std::vector<int> homeNodesIdx(homeNodesCount);
          std::vector<int> homeNodesTravelTimesSeconds(homeNodesCount);
          std::vector<int> homeNodesDistancesMeters(homeNodesCount);
          for (int i = 0; i < homeNodesCount; i++)
          {
            homeNodesIdx               [i] = capnpT.getHomeNodesIdx()[i];
            homeNodesTravelTimesSeconds[i] = capnpT.getHomeNodesTravelTimes()[i];
            homeNodesDistancesMeters   [i] = capnpT.getHomeNodesDistances()[i];
          }
          t->homeNodesIdx                = homeNodesIdx;
          t->homeNodesTravelTimesSeconds = homeNodesTravelTimesSeconds;
          t->homeNodesDistancesMeters    = homeNodesDistancesMeters;

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
    
    return std::make_pair(ts, tIndexesByUuid);
  }

}

#endif // TR_HOUSEHOLDS_CACHE_FETCHER