
#ifndef TR_STATIONS_CACHE_FETCHER
#define TR_STATIONS_CACHE_FETCHER

#include <string>
#include <vector>

#include "cache_fetcher.hpp"
#include "station.hpp"
#include "point.hpp"
#include "capnp/stationCollection.capnp.h"
//#include "toolbox.hpp"

namespace TrRouting
{

  const std::pair<std::vector<Station>, std::map<boost::uuids::uuid, int>> CacheFetcher::getStations(Parameters& params, std::string customPath)
  { 

    using T           = Station;
    using TCollection = stationCollection::StationCollection;
    using cT          = stationCollection::Station;

    std::string tStr  = "stations";
    std::string TStr  = "Stations";

    std::vector<T> ts;
    std::string cacheFileName{tStr};
    std::map<boost::uuids::uuid, int> tIndexesByUuid;
    boost::uuids::string_generator uuidGenerator;

    std::cout << "Fetching " << tStr << " from cache..." << std::endl;
    
    if (CacheFetcher::capnpCacheFileExists(cacheFileName + ".capnpbin", params, customPath))
    {
      int fd = open((CacheFetcher::getFilePath(cacheFileName, params, customPath) + ".capnpbin").c_str(), O_RDWR);
      ::capnp::PackedFdMessageReader capnpTCollectionMessage(fd, {64 * 1024 * 1024});
      TCollection::Reader capnpTCollection = capnpTCollectionMessage.getRoot<TCollection>();
      for (cT::Reader capnpT : capnpTCollection.getStations())
      {

        // not yet implemented:
        std::string uuid        {capnpT.getUuid()};
        T     * t          = new T();
        t->uuid            = uuidGenerator(uuid);
        //t->id              = capnpT.getId();
        t->code            = capnpT.getCode();
        t->name            = capnpT.getName();
        ts.push_back(*t);
        tIndexesByUuid[t->uuid] = ts.size() - 1;
      }
      std::cout << TStr << ":\n" << Toolbox::prettyPrintStructVector(ts) << std::endl;
      close(fd);
    }
    else
    {
      std::cerr << "missing " << tStr << " cache file!" << std::endl;
    }
    return std::make_pair(ts, tIndexesByUuid);
  }

}

#endif // TR_STATIONS_CACHE_FETCHER