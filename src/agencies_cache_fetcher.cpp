
#ifndef TR_AGENCIES_CACHE_FETCHER
#define TR_AGENCIES_CACHE_FETCHER

#include <string>
#include <vector>
#include <errno.h>
#include <kj/exception.h>

#include "cache_fetcher.hpp"
#include "agency.hpp"
#include "capnp/agencyCollection.capnp.h"

namespace TrRouting
{

  int CacheFetcher::getAgencies(
    std::vector<std::unique_ptr<Agency>>& ts,
    std::map<boost::uuids::uuid, int>& tIndexesByUuid,
    ServerParameters& params,
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

    std::cout << "Fetching " << tStr << " from cache..." << " " << customPath << std::endl;
    
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
      std::cerr << "Error opening cache file " << tStr << ": " << e.getDescription().cStr() << std::endl;
      ret = -EBADMSG;
    }
    catch (...)
    {
      std::cerr << "Unknown error occurred " << tStr << std::endl; 
      ret = -EINVAL;
    }

    close(fd);
    return ret;
  }

}

#endif // TR_AGENCIES_CACHE_FETCHER