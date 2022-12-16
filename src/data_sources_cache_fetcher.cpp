
#ifndef TR_DATA_SOURCES_CACHE_FETCHER
#define TR_DATA_SOURCES_CACHE_FETCHER

#include <string>
#include <vector>
#include <errno.h>
#include <fcntl.h>
#include <kj/exception.h>
#include <boost/uuid/string_generator.hpp>
#include <capnp/serialize-packed.h>
#include "cache_fetcher.hpp"
#include "data_source.hpp"
#include "capnp/dataSourceCollection.capnp.h"
#include "spdlog/spdlog.h"

namespace TrRouting
{

  int CacheFetcher::getDataSources(
    std::map<boost::uuids::uuid, DataSource>& ts,
    std::string customPath
  )
  {

    using T           = DataSource;
    using TCollection = dataSourceCollection::DataSourceCollection;
    using cT          = dataSource::DataSource;
    int ret = 0;

    ts.clear();

    std::string tStr  = "dataSources";
    std::string TStr  = "DataSources";

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
      ::capnp::PackedFdMessageReader capnpTCollectionMessage(fd, {16 * 1024 * 1024});
      TCollection::Reader capnpTCollection = capnpTCollectionMessage.getRoot<TCollection>();
      for (cT::Reader capnpT : capnpTCollection.getDataSources())
      {
        std::string uuid {capnpT.getUuid()};
        std::vector<int> servicesIdx;
        
        T t;

        t.uuid = uuidGenerator(uuid);
        t.name = capnpT.getName();

        switch (capnpT.getType()) {
          case dataSource::DataSource::Type::NONE                     : t.type = "none";                    break;
          case dataSource::DataSource::Type::OTHER                    : t.type = "other";                   break;
          case dataSource::DataSource::Type::GTFS                     : t.type = "gtfs";                    break;
          case dataSource::DataSource::Type::OD_TRIPS                 : t.type = "odTrips";                 break;
          case dataSource::DataSource::Type::TRANSIT_SMART_CARD_DATA  : t.type = "transitSmartCardData";    break;
          case dataSource::DataSource::Type::TRANSIT_OPERATIONAL_DATA : t.type = "transitOperationalData";  break;
          case dataSource::DataSource::Type::TAXI_TRANSACTIONS        : t.type = "taxiTransactions";        break;
          case dataSource::DataSource::Type::CAR_SHARING_TRANSACTIONS : t.type = "carSharingTransactions";  break;
          case dataSource::DataSource::Type::BIKE_SHARING_TRANSACTIONS: t.type = "bikeSharingTransactions"; break;
          case dataSource::DataSource::Type::GPS_TRACES               : t.type = "gpsTraces";               break;
          case dataSource::DataSource::Type::STREET_SEGMENT_SPEEDS    : t.type = "streetSegmentSpeeds";     break;
          case dataSource::DataSource::Type::ZONES                    : t.type = "zones";                   break;
          case dataSource::DataSource::Type::OSM_DATA                 : t.type = "osmData";                 break;
          case dataSource::DataSource::Type::PLACES                   : t.type = "places";                  break;
          case dataSource::DataSource::Type::UNKNOWN                  : t.type = "unknown";                 break;
          default:
            // There may be other types that are not supported in trRouting, consider them as unknowns
            //TODO add type to the message
            spdlog::info("Unsupported data source type for uuid {}: will list as unknown", uuid);
            t.type = "unknown";
            break;
        }

        //tIndexesByUuid[t.uuid] = ts.size();
        ts[t.uuid] = t;
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

#endif // TR_DATA_SOURCES_CACHE_FETCHER
