
#ifndef TR_HOUSEHOLDS_CACHE_FETCHER
#define TR_HOUSEHOLDS_CACHE_FETCHER

#include <string>
#include <vector>
#include <errno.h>
#include <fcntl.h>
#include <kj/exception.h>
#include <boost/uuid/string_generator.hpp>
#include <capnp/serialize-packed.h>
#include "cache_fetcher.hpp"
#include "household.hpp"
#include "point.hpp"
#include "capnp/householdCollection.capnp.h"
#include "capnp/household.capnp.h"
#include "spdlog/spdlog.h"

namespace TrRouting
{

  int CacheFetcher::getHouseholds(
    std::vector<std::unique_ptr<Household>>& ts,
    std::map<boost::uuids::uuid, int>& tIndexesByUuid,
    const std::map<boost::uuids::uuid, int>& dataSourceIndexesByUuid,
    const std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
    std::string customPath
  )
  {

    using T           = Household;
    using TCollection = householdCollection::HouseholdCollection;
    using cT          = household::Household;
    int ret = 0;

    std::string tStr  = "households";
    std::string TStr  = "Households";

    ts.clear();
    tIndexesByUuid.clear();

    std::string cacheFileName{tStr};
    boost::uuids::string_generator uuidGenerator;

    spdlog::info("Fetching {} from cache... {}", tStr, customPath);

    for(std::map<boost::uuids::uuid, int>::const_iterator iter = dataSourceIndexesByUuid.begin(); iter != dataSourceIndexesByUuid.end(); ++iter)
    {
      boost::uuids::uuid dataSourceUuid = iter->first;

      std::string cacheFilePath {"dataSources/" + boost::uuids::to_string(dataSourceUuid) + "/" + cacheFileName};

      int filesCount {CacheFetcher::getCacheFilesCount(getFilePath(cacheFilePath + ".capnpbin.count", customPath))};

      spdlog::info("files count households: {} path: {}", filesCount, cacheFilePath);

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
          for (cT::Reader capnpT : capnpTCollection.getHouseholds())
          {
            std::string uuid           {capnpT.getUuid()};
            std::string dataSourceUuid {capnpT.getDataSourceUuid()};
            std::unique_ptr<Point> point = std::make_unique<Point>();
            std::unique_ptr<T>     t     = std::make_unique<T>();
            t->uuid            = uuidGenerator(uuid);
            t->id              = capnpT.getId();
            t->expansionFactor = capnpT.getExpansionFactor();
            t->size            = capnpT.getSize();
            t->carNumber       = capnpT.getCarNumber();
            t->incomeLevel     = capnpT.getIncomeLevel();
            t->internalId      = capnpT.getInternalId();
            t->dataSourceIdx   = dataSourceUuid.length() > 0 ? dataSourceIndexesByUuid.at(uuidGenerator(dataSourceUuid)) : -1;

            switch (capnpT.getIncomeLevelGroup()) {
              case household::Household::IncomeLevelGroup::NONE      : t->incomeLevelGroup = "none";     break;
              case household::Household::IncomeLevelGroup::VERY_LOW  : t->incomeLevelGroup = "veryLow";  break;
              case household::Household::IncomeLevelGroup::LOW       : t->incomeLevelGroup = "low";      break;
              case household::Household::IncomeLevelGroup::MEDIUM    : t->incomeLevelGroup = "medium";   break;
              case household::Household::IncomeLevelGroup::HIGH      : t->incomeLevelGroup = "high";     break;
              case household::Household::IncomeLevelGroup::VERY_HIGH : t->incomeLevelGroup = "veryHigh"; break;
              case household::Household::IncomeLevelGroup::UNKNOWN   : t->incomeLevelGroup = "unknown";  break;
            }

            switch (capnpT.getCategory()) {
              case household::Household::Category::NONE                : t->category = "none";               break;
              case household::Household::Category::SINGLE_PERSON       : t->category = "singlePerson";       break;
              case household::Household::Category::COUPLE              : t->category = "couple";             break;
              case household::Household::Category::MONOPARENTAL_FAMILY : t->category = "monoparentalFamily"; break;
              case household::Household::Category::BIPARENTAL_FAMILY   : t->category = "biparentalFamily";   break;
              case household::Household::Category::OTHER               : t->category = "other";              break;
              case household::Household::Category::UNKNOWN             : t->category = "unknown";            break;
            }

            point->latitude  = ((double)capnpT.getHomeLatitude())  / 1000000.0;
            point->longitude = ((double)capnpT.getHomeLongitude()) / 1000000.0;
            t->point         = std::move(point);
            
            const unsigned int homeNodesCount {capnpT.getHomeNodesUuids().size()};
            std::vector<int> homeNodesIdx(homeNodesCount);
            std::vector<int> homeNodesTravelTimesSeconds(homeNodesCount);
            std::vector<int> homeNodesDistancesMeters(homeNodesCount);
            for (int i = 0; i < homeNodesCount; i++)
            {
              std::string nodeUuid {capnpT.getHomeNodesUuids()[i]};
              homeNodesIdx               [i] = nodeIndexesByUuid.at(uuidGenerator(nodeUuid));
              homeNodesTravelTimesSeconds[i] = capnpT.getHomeNodesTravelTimes()[i];
              homeNodesDistancesMeters   [i] = capnpT.getHomeNodesDistances()[i];
            }
            t->homeNodesIdx                = homeNodesIdx;
            t->homeNodesTravelTimesSeconds = homeNodesTravelTimesSeconds;
            t->homeNodesDistancesMeters    = homeNodesDistancesMeters;

            tIndexesByUuid[t->uuid] = ts.size();
            ts.push_back(std::move(t));
          }
        }
        catch (const kj::Exception& e)
        {
          spdlog::error("Error reading cache file {}: {}", filePath, e.getDescription().cStr());
        }
        catch (...)
        {
          spdlog::error("Unknown error occurred {} ", filePath);
        }

        close(fd);
      }
    }
    return ret;
    
  }

}

#endif // TR_HOUSEHOLDS_CACHE_FETCHER
