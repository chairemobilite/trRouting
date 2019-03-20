
#ifndef TR_AGENCIES_CACHE_FETCHER
#define TR_AGENCIES_CACHE_FETCHER

#include <string>
#include <vector>

#include "cache_fetcher.hpp"
#include "agency.hpp"
#include "capnp/agencyCollection.capnp.h"

namespace TrRouting
{

  const std::pair<std::vector<Agency>, std::map<boost::uuids::uuid, int>> CacheFetcher::getAgencies(Parameters& params)
  {

    using T           = Agency;
    using TCollection = agencyCollection::AgencyCollection;
    using cT          = agencyCollection::Agency;

    std::string tStr  = "agencies";
    std::string TStr  = "Agencies";

    std::vector<T> ts;
    std::string cacheFileName{tStr};
    std::map<boost::uuids::uuid, int> tIndexesByUuid;
    boost::uuids::string_generator uuidGenerator;

    std::cout << "Fetching " << tStr << " from cache..." << std::endl;
    
    if (CacheFetcher::capnpCacheFileExists(cacheFileName + ".capnpbin", params))
    {
      int fd = open((CacheFetcher::getFilePath(cacheFileName, params) + ".capnpbin").c_str(), O_RDWR);
      ::capnp::PackedFdMessageReader capnpTCollectionMessage(fd, {16 * 1024 * 1024});
      TCollection::Reader capnpTCollection = capnpTCollectionMessage.getRoot<TCollection>();
      for (cT::Reader capnpT : capnpTCollection.getAgencies())
      {
        std::string uuid {capnpT.getUuid()};
        T * t               = new T();
        t->uuid             = uuidGenerator(uuid);
        t->acronym          = capnpT.getAcronym();
        t->name             = capnpT.getName();
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

#endif // TR_AGENCIES_CACHE_FETCHER