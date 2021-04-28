
#ifndef TR_HOUSEHOLDS_CACHE_FETCHER
#define TR_HOUSEHOLDS_CACHE_FETCHER

#include <string>
#include <vector>
#include <errno.h>
#include <kj/exception.h>

#include "cache_fetcher.hpp"
#include "household.hpp"
#include "point.hpp"
#include "capnp/householdCollection.capnp.h"
#include "capnp/household.capnp.h"
//#include "toolbox.hpp"

namespace TrRouting
{

  int CacheFetcher::getHouseholds(
    std::vector<std::unique_ptr<Household>>& ts,
    std::map<boost::uuids::uuid, int>& tIndexesByUuid,
    std::map<boost::uuids::uuid, int>& dataSourceIndexesByUuid,
    std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
    Parameters& params,
    std::string customPath
  )
  {

    using T           = Household;
    using TCollection = householdCollection::HouseholdCollection;
    using cT          = household::Household;

    std::string tStr  = "households";
    std::string TStr  = "Households";

    ts.clear();
    tIndexesByUuid.clear();

    std::string cacheFileName{tStr};
    boost::uuids::string_generator uuidGenerator;

    std::cout << "Fetching " << tStr << " from cache..." << std::endl;

    for(std::map<boost::uuids::uuid, int>::iterator iter = dataSourceIndexesByUuid.begin(); iter != dataSourceIndexesByUuid.end(); ++iter)
    {
      boost::uuids::uuid dataSourceUuid = iter->first;

      std::string cacheFilePath {"dataSources/" + boost::uuids::to_string(dataSourceUuid) + "/households/" + cacheFileName};

      int filesCount {CacheFetcher::getCacheFilesCount(CacheFetcher::getFilePath(cacheFilePath + ".capnpbin.count", params, customPath))};

      std::cout << "files count households: " << filesCount << " path: " << cacheFilePath << std::endl;

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
            t->dataSourceIdx   = dataSourceUuid.length() > 0 ? dataSourceIndexesByUuid[uuidGenerator(dataSourceUuid)] : -1;

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
              homeNodesIdx               [i] = nodeIndexesByUuid[uuidGenerator(nodeUuid)];
              homeNodesTravelTimesSeconds[i] = capnpT.getHomeNodesTravelTimes()[i];
              homeNodesDistancesMeters   [i] = capnpT.getHomeNodesDistances()[i];
            }
            t->homeNodesIdx                = homeNodesIdx;
            t->homeNodesTravelTimesSeconds = homeNodesTravelTimesSeconds;
            t->homeNodesDistancesMeters    = homeNodesDistancesMeters;

            tIndexesByUuid[t->uuid] = ts.size();
            ts.push_back(std::move(t));
          }
          //std::cout << TStr << ":\n" << Toolbox::prettyPrintStructVector(ts) << std::endl;
          close(fd);
        }
        catch (const kj::Exception& e)
        {
          std::cerr << "Error reading cache file " << filePath << ": " << e.getDescription().cStr() << std::endl;
          close(fd);
        }
        catch (...)
        {
          std::cerr << "Unknown error occurred " << filePath << std::endl;
          close(fd);
        }
      }
    }
    return 0;
    
  }

}

#endif // TR_HOUSEHOLDS_CACHE_FETCHER