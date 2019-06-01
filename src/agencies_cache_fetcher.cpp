
#ifndef TR_AGENCIES_CACHE_FETCHER
#define TR_AGENCIES_CACHE_FETCHER

#include <string>
#include <vector>

#include "cache_fetcher.hpp"
#include "agency.hpp"
#include "capnp/agencyCollection.capnp.h"

namespace TrRouting
{

  void CacheFetcher::getAgencies(
    std::vector<std::unique_ptr<Agency>>& ts,
    std::map<boost::uuids::uuid, int>& tIndexesByUuid,
    Parameters& params,
    std::string customPath
  ) {

    using T           = Agency;
    using TCollection = agencyCollection::AgencyCollection;
    using cT          = agencyCollection::Agency;

    ts.clear();
    tIndexesByUuid.clear();

    std::string tStr  = "agencies";
    std::string TStr  = "Agencies";

    std::string cacheFileName{tStr};
    boost::uuids::string_generator uuidGenerator;

    std::cout << "Fetching " << tStr << " from cache..." << " " << customPath << std::endl;
    
    if (CacheFetcher::capnpCacheFileExists(cacheFileName + ".capnpbin", params, customPath))
    {
      int fd = open((CacheFetcher::getFilePath(cacheFileName, params, customPath) + ".capnpbin").c_str(), O_RDWR);
      ::capnp::PackedFdMessageReader capnpTCollectionMessage(fd, {16 * 1024 * 1024});
      TCollection::Reader capnpTCollection = capnpTCollectionMessage.getRoot<TCollection>();
      for (cT::Reader capnpT : capnpTCollection.getAgencies())
      {
        std::string uuid {capnpT.getUuid()};

        std::unique_ptr<T> t = std::make_unique<T>();

        t->uuid       = uuidGenerator(uuid);
        t->acronym    = capnpT.getAcronym();
        t->name       = capnpT.getName();
        t->internalId = capnpT.getInternalId();
        
        tIndexesByUuid[t->uuid] = ts.size();
        ts.push_back(std::move(t));
      }
      //std::cout << TStr << ":\n" << Toolbox::prettyPrintStructVector(ts) << std::endl;
      close(fd);
    }
    else
    {
      std::cerr << "missing " << tStr << " cache file!" << std::endl;
    }
  }

}

#endif // TR_AGENCIES_CACHE_FETCHER