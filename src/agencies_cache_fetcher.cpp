
#ifndef TR_AGENCIES_CACHE_FETCHER
#define TR_AGENCIES_CACHE_FETCHER

#include <string>
#include <vector>
#include <errno.h>
#include <fcntl.h>
#include <kj/exception.h>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/string_generator.hpp>
#include "spdlog/spdlog.h"

#include <capnp/message.h>
#include <capnp/serialize-packed.h>

#include "cache_fetcher.hpp"
#include "agency.hpp"
#include "capnp/agencyCollection.capnp.h"

namespace TrRouting
{

  int CacheFetcher::getAgencies(
    std::vector<std::unique_ptr<Agency>>& ts,
    std::map<boost::uuids::uuid, int>& tIndexesByUuid,
    std::string customPath
  )
  {

    using T           = Agency;
    using TCollection = agencyCollection::AgencyCollection;
    using cT          = agencyCollection::Agency;
    int ret = 0;

    ts.clear();
    tIndexesByUuid.clear();

    std::string tStr  = "agencies";
    std::string TStr  = "Agencies";

    std::string cacheFileName{tStr};
    boost::uuids::string_generator uuidGenerator;
    boost::uuids::nil_generator    uuidNilGenerator;

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
      for (cT::Reader capnpT : capnpTCollection.getAgencies())
      {
        std::string uuid           {capnpT.getUuid()};
        std::string simulationUuid {capnpT.getSimulationUuid()};

        std::unique_ptr<T> t = std::make_unique<T>();

        t->uuid           = uuidGenerator(uuid);
        t->acronym        = capnpT.getAcronym();
        t->name           = capnpT.getName();
        t->internalId     = capnpT.getInternalId();
        t->simulationUuid = simulationUuid.empty() ? uuidNilGenerator() : uuidGenerator(simulationUuid);
        
        tIndexesByUuid[t->uuid] = ts.size();
        ts.push_back(std::move(t));
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

#endif // TR_AGENCIES_CACHE_FETCHER
