
#ifndef TR_NETWORKS_CACHE_FETCHER
#define TR_NETWORKS_CACHE_FETCHER

#include <string>
#include <vector>
#include <errno.h>
#include <kj/exception.h>

#include "cache_fetcher.hpp"
#include "network.hpp"
#include "capnp/networkCollection.capnp.h"

namespace TrRouting
{

  int CacheFetcher::getNetworks(
    std::vector<std::unique_ptr<Network>>& ts,
    std::map<boost::uuids::uuid, int>& tIndexesByUuid,
    std::map<boost::uuids::uuid, int>& agencyIndexesByUuid,
    std::map<boost::uuids::uuid, int>& serviceIndexesByUuid,
    std::map<boost::uuids::uuid, int>& scenarioIndexesByUuid,
    Parameters& params,
    std::string customPath
  )
  {

    using T           = Network;
    using TCollection = networkCollection::NetworkCollection;
    using cT          = networkCollection::Network;
    int ret = 0;

    ts.clear();
    tIndexesByUuid.clear();

    std::string tStr  = "networks";
    std::string TStr  = "Networks";

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
      for (cT::Reader capnpT : capnpTCollection.getNetworks())
      {
        std::string uuid           {capnpT.getUuid()};
        std::string simulationUuid {capnpT.getSimulationUuid()};

        std::vector<int> agenciesIdx;
        std::vector<int> servicesIdx;
        std::vector<int> scenariosIdx;
        boost::uuids::uuid agencyUuid;
        boost::uuids::uuid serviceUuid;
        boost::uuids::uuid scenarioUuid;

        std::unique_ptr<T> t = std::make_unique<T>();

        t->uuid           = uuidGenerator(uuid);
        t->shortname      = capnpT.getShortname();
        t->name           = capnpT.getName();
        t->internalId     = capnpT.getInternalId();
        t->simulationUuid = simulationUuid.empty() ? uuidNilGenerator() : uuidGenerator(simulationUuid);
        
        for (std::string agencyUuidStr : capnpT.getAgenciesUuids())
        {
          agencyUuid = uuidGenerator(agencyUuidStr);
          agenciesIdx.push_back(agencyIndexesByUuid[agencyUuid]);
        }
        t->agenciesIdx = agenciesIdx;
        
        for (std::string serviceUuidStr : capnpT.getServicesUuids())
        {
          serviceUuid = uuidGenerator(serviceUuidStr);
          servicesIdx.push_back(serviceIndexesByUuid[serviceUuid]);
        }
        t->servicesIdx = servicesIdx;
        
        for (std::string scenarioUuidStr : capnpT.getScenariosUuids())
        {
          scenarioUuid = uuidGenerator(scenarioUuidStr);
          scenariosIdx.push_back(scenarioIndexesByUuid[scenarioUuid]);
        }
        t->scenariosIdx = scenariosIdx;
        
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

#endif // TR_NETWORKS_CACHE_FETCHER