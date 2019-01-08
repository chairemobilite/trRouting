
#ifndef TR_AGENCIES_CACHE_FETCHER
#define TR_AGENCIES_CACHE_FETCHER

#include <string>
#include <vector>

#include "cache_fetcher.hpp"
#include "agency.hpp"
#include "capnp/agenciesCollection.capnp.h"


namespace TrRouting
{

  const std::pair<std::vector<Agency>, std::map<boost::uuids::uuid, int>> CacheFetcher::getAgencies(std::string projectShortname, Parameters& params)
  {
    std::vector<Agency> agencies;
    std::string cacheFileName{"agencies"};
    std::map<boost::uuids::uuid, int> agencyIndexesByUuid;
    boost::uuids::string_generator uuidGenerator;

    std::cout << "Fetching routes from cache..." << std::endl;
    if (CacheFetcher::capnpCacheFileExists(projectShortname, cacheFileName, params))
    {
      int fd = open((projectShortname + "/" + cacheFileName + ".capnpbin").c_str(), O_RDWR);
      ::capnp::PackedFdMessageReader capnpAgenciesCollectionMessage(fd, {16 * 1024 * 1024});
      agenciesCollection::AgenciesCollection::Reader capnpAgenciesCollection = capnpAgenciesCollectionMessage.getRoot<agenciesCollection::AgenciesCollection>();
      for (agenciesCollection::Agency::Reader capnpAgency : capnpAgenciesCollection.getAgencies())
      {
        Agency * agency          = new Agency();
        std::string uuid {capnpAgency.getUuid()};
        agency->uuid             = uuidGenerator(uuid);
        agency->acronym          = capnpAgency.getAcronym();
        agency->name             = capnpAgency.getName();
        agencies.push_back(*agency);
        agencyIndexesByUuid[agency->uuid] = agencies.size() - 1;
      }
      close(fd);
    }
    else
    {
      std::cerr << "missing agencies cache file!" << std::endl;
    }
    return std::make_pair(agencies, agencyIndexesByUuid);
  }

}

#endif // TR_AGENCIES_CACHE_FETCHER