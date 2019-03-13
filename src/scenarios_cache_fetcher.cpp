
#ifndef TR_SCENARIOS_CACHE_FETCHER
#define TR_SCENARIOS_CACHE_FETCHER

#include <string>
#include <vector>

#include "cache_fetcher.hpp"
#include "scenario.hpp"
#include "capnp/scenarioCollection.capnp.h"
//#include "toolbox.hpp"

namespace TrRouting
{

  const std::pair<std::vector<Scenario>, std::map<boost::uuids::uuid, int>> CacheFetcher::getScenarios(std::map<boost::uuids::uuid, int> serviceIndexesByUuid, Parameters& params)
  { 

    using T           = Scenario;
    using TCollection = scenarioCollection::ScenarioCollection;
    using cT          = scenarioCollection::Scenario;

    std::string tStr  = "scenarios";
    std::string TStr  = "Scenarios";

    std::vector<T> ts;
    std::string cacheFileName{tStr};
    std::map<boost::uuids::uuid, int> tIndexesByUuid;
    boost::uuids::string_generator uuidGenerator;

    std::cout << "Fetching " << tStr << " from cache..." << std::endl;
    if (CacheFetcher::capnpCacheFileExists(cacheFileName, params))
    {
      int fd = open((params.cacheDirectoryPath + params.projectShortname + "/" + cacheFileName + ".capnpbin").c_str(), O_RDWR);
      ::capnp::PackedFdMessageReader capnpTCollectionMessage(fd, {16 * 1024 * 1024});
      TCollection::Reader capnpTCollection = capnpTCollectionMessage.getRoot<TCollection>();
      for (cT::Reader capnpT : capnpTCollection.getScenarios())
      {
        std::string uuid {capnpT.getUuid()};
        std::vector<int> servicesIdx;
        boost::uuids::uuid serviceUuid;
        T * t   = new T();
        t->uuid = uuidGenerator(uuid);
        t->name = capnpT.getName();
        for (std::string serviceUuidStr : capnpT.getServicesUuids())
        {
          serviceUuid = uuidGenerator(serviceUuidStr);
          servicesIdx.push_back(serviceIndexesByUuid[serviceUuid]);
        }
        t->servicesIdx = servicesIdx;
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

#endif // TR_SCENARIOS_CACHE_FETCHER