
#ifndef TR_NETWORKS_CACHE_FETCHER
#define TR_NETWORKS_CACHE_FETCHER

#include <string>
#include <vector>

#include "cache_fetcher.hpp"
#include "network.hpp"
#include "capnp/networkCollection.capnp.h"

namespace TrRouting
{

  void CacheFetcher::getNetworks(
    std::vector<std::unique_ptr<Network>>& ts,
    std::map<boost::uuids::uuid, int>& tIndexesByUuid,
    std::map<boost::uuids::uuid, int>& agencyIndexesByUuid,
    std::map<boost::uuids::uuid, int>& serviceIndexesByUuid,
    std::map<boost::uuids::uuid, int>& scenarioIndexesByUuid,
    Parameters& params,
    std::string customPath
  ) {

    using T           = Network;
    using TCollection = networkCollection::NetworkCollection;
    using cT          = networkCollection::Network;

    ts.clear();
    tIndexesByUuid.clear();

    std::string tStr  = "networks";
    std::string TStr  = "Networks";

    std::string cacheFileName{tStr};
    boost::uuids::string_generator uuidGenerator;
    boost::uuids::nil_generator    uuidNilGenerator;

    std::cout << "Fetching " << tStr << " from cache..." << " " << customPath << std::endl;
    
    if (CacheFetcher::capnpCacheFileExists(cacheFileName + ".capnpbin", params, customPath))
    {
      int fd = open((CacheFetcher::getFilePath(cacheFileName, params, customPath) + ".capnpbin").c_str(), O_RDWR);
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
        t->simulationUuid = simulationUuid.empty() > 0 ? uuidGenerator(simulationUuid) : uuidNilGenerator();
        
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
      //std::cout << TStr << ":\n" << Toolbox::prettyPrintStructVector(ts) << std::endl;
      close(fd);
    }
    else
    {
      std::cerr << "missing " << tStr << " cache file!" << std::endl;
    }
  }

}

#endif // TR_NETWORKS_CACHE_FETCHER