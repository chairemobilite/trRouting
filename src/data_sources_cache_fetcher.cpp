
#ifndef TR_DATA_SOURCES_CACHE_FETCHER
#define TR_DATA_SOURCES_CACHE_FETCHER

#include <string>
#include <vector>

#include "cache_fetcher.hpp"
#include "data_source.hpp"
#include "capnp/dataSourceCollection.capnp.h"
//#include "toolbox.hpp"

namespace TrRouting
{

  const std::pair<std::vector<DataSource>, std::map<boost::uuids::uuid, int>> CacheFetcher::getDataSources(Parameters& params)
  { 

    using T           = DataSource;
    using TCollection = dataSourceCollection::DataSourceCollection;
    using cT          = dataSource::DataSource;

    std::string tStr  = "dataSources";
    std::string TStr  = "DataSources";

    std::vector<T> ts;
    std::string cacheFileName{tStr};
    std::map<boost::uuids::uuid, int> tIndexesByUuid;
    boost::uuids::string_generator uuidGenerator;

    std::cout << "Fetching " << tStr << " from cache..." << std::endl;
    if (CacheFetcher::capnpCacheFileExists(cacheFileName + ".capnpbin", params))
    {
      int fd = open((params.cacheDirectoryPath + params.projectShortname + "/" + cacheFileName + ".capnpbin").c_str(), O_RDWR);
      ::capnp::PackedFdMessageReader capnpTCollectionMessage(fd, {16 * 1024 * 1024});
      TCollection::Reader capnpTCollection = capnpTCollectionMessage.getRoot<TCollection>();
      for (cT::Reader capnpT : capnpTCollection.getDataSources())
      {
        std::string uuid {capnpT.getUuid()};
        std::vector<int> servicesIdx;
        boost::uuids::uuid serviceUuid;
        T * t   = new T();
        t->uuid = uuidGenerator(uuid);
        t->name = capnpT.getName();
        switch (capnpT.getType()) {
          case dataSource::DataSource::Type::NONE                     : t->type = "none";                    break;
          case dataSource::DataSource::Type::OTHER                    : t->type = "other";                   break;
          case dataSource::DataSource::Type::GTFS                     : t->type = "gtfs";                    break;
          case dataSource::DataSource::Type::OD_TRIPS                 : t->type = "odTrips";                 break;
          case dataSource::DataSource::Type::TRANSIT_SMART_CARD_DATA  : t->type = "transitSmartCardData";    break;
          case dataSource::DataSource::Type::TRANSIT_OPERATIONAL_DATA : t->type = "transitOperationalData";  break;
          case dataSource::DataSource::Type::TAXI_TRANSACTIONS        : t->type = "taxiTransactions";        break;
          case dataSource::DataSource::Type::CAR_SHARING_TRANSACTIONS : t->type = "carSharingTransactions";  break;
          case dataSource::DataSource::Type::BIKE_SHARING_TRANSACTIONS: t->type = "bikeSharingTransactions"; break;
          case dataSource::DataSource::Type::GPS_TRACES               : t->type = "gpsTraces";               break;
          case dataSource::DataSource::Type::STREET_SEGMENT_SPEEDS    : t->type = "streetSegmentSpeeds";     break;
          case dataSource::DataSource::Type::UNKNOWN                  : t->type = "unknown";                 break;
        }
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

}

#endif // TR_DATA_SOURCES_CACHE_FETCHER