
#ifndef TR_STATIONS_CACHE_FETCHER
#define TR_STATIONS_CACHE_FETCHER

#include <string>
#include <vector>
#include <errno.h>
#include <kj/exception.h>

#include "cache_fetcher.hpp"
#include "station.hpp"
#include "point.hpp"
#include "capnp/stationCollection.capnp.h"
//#include "toolbox.hpp"

namespace TrRouting
{

  int CacheFetcher::getStations(
    std::vector<std::unique_ptr<Station>>& ts,
    std::map<boost::uuids::uuid, int>& tIndexesByUuid,
    Parameters& params,
    std::string customPath
  ) 
  {

    using T           = Station;
    using TCollection = stationCollection::StationCollection;
    using cT          = stationCollection::Station;

    ts.clear();
    tIndexesByUuid.clear();

    std::string tStr  = "stations";
    std::string TStr  = "Stations";

    std::string cacheFileName{tStr};
    boost::uuids::string_generator uuidGenerator;

    std::cout << "Fetching " << tStr << " from cache..." << std::endl;
    
    std::string cacheFilePath = CacheFetcher::getFilePath(cacheFileName, params, customPath) + ".capnpbin";
    int fd = open(cacheFilePath.c_str(), O_RDWR);
    if (fd < 0)
    {
      int err = errno;
      if (err == ENOENT)
      {
        std::cerr << "missing " << tStr << " cache file!" << std::endl;
      }
      else
      {
        std::cerr << "Error opening cache file " << tStr << ": " << err << std::endl;
      }
      return -err;
    }

    try
    {
      ::capnp::PackedFdMessageReader capnpTCollectionMessage(fd, {64 * 1024 * 1024});
      TCollection::Reader capnpTCollection = capnpTCollectionMessage.getRoot<TCollection>();
      for (cT::Reader capnpT : capnpTCollection.getStations())
      {

        // not yet implemented:
        std::string uuid        {capnpT.getUuid()};
        
        std::unique_ptr<T> t = std::make_unique<T>();

        t->uuid            = uuidGenerator(uuid);
        //t->id              = capnpT.getId();
        t->code            = capnpT.getCode();
        t->name            = capnpT.getName();
        t->internalId      = capnpT.getInternalId();

        tIndexesByUuid[t->uuid] = ts.size();
        ts.push_back(std::move(t));
      }
      //std::cout << TStr << ":\n" << Toolbox::prettyPrintStructVector(ts) << std::endl;
      close(fd);
      return 0;
    }
    catch (const kj::Exception& e)
    {
      std::cerr << "Error opening cache file " << tStr << ": " << e.getDescription().cStr() << std::endl;
      close(fd);
      return -EBADMSG;
    }
    catch (...)
    {
      std::cerr << "Unknown error occurred " << tStr << std::endl;
      close(fd);
      return -EINVAL;
    }
  }

}

#endif // TR_STATIONS_CACHE_FETCHER