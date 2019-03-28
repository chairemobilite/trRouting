
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

  const std::pair<std::vector<Scenario>, std::map<boost::uuids::uuid, int>> CacheFetcher::getScenarios(std::map<boost::uuids::uuid, int> serviceIndexesByUuid, std::map<boost::uuids::uuid, int> lineIndexesByUuid, std::map<boost::uuids::uuid, int> agencyIndexesByUuid, std::map<boost::uuids::uuid, int> nodeIndexesByUuid, std::map<std::string, int> modeIndexesByShortname, Parameters& params)
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
    
    if (CacheFetcher::capnpCacheFileExists(cacheFileName + ".capnpbin", params))
    {
      int fd = open((CacheFetcher::getFilePath(cacheFileName, params) + ".capnpbin").c_str(), O_RDWR);
      ::capnp::PackedFdMessageReader capnpTCollectionMessage(fd, {16 * 1024 * 1024});
      TCollection::Reader capnpTCollection = capnpTCollectionMessage.getRoot<TCollection>();
      for (cT::Reader capnpT : capnpTCollection.getScenarios())
      {
        std::string uuid {capnpT.getUuid()};
        std::vector<int> servicesIdx;
        std::vector<int> onlyLinesIdx;
        std::vector<int> onlyAgenciesIdx;
        std::vector<int> onlyNodesIdx;
        std::vector<int> onlyModesIdx;
        std::vector<int> exceptLinesIdx;
        std::vector<int> exceptAgenciesIdx;
        std::vector<int> exceptNodesIdx;
        std::vector<int> exceptModesIdx;
        boost::uuids::uuid serviceUuid;
        boost::uuids::uuid lineUuid;
        boost::uuids::uuid agencyUuid;
        boost::uuids::uuid nodeUuid;
        T * t   = new T();
        t->uuid = uuidGenerator(uuid);
        t->name = capnpT.getName();
        for (std::string serviceUuidStr : capnpT.getServicesUuids())
        {
          serviceUuid = uuidGenerator(serviceUuidStr);
          servicesIdx.push_back(serviceIndexesByUuid[serviceUuid]);
        }
        t->servicesIdx = servicesIdx;
        for (std::string lineUuidStr : capnpT.getOnlyLinesUuids())
        {
          lineUuid = uuidGenerator(lineUuidStr);
          onlyLinesIdx.push_back(lineIndexesByUuid[lineUuid]);
        }
        t->onlyLinesIdx = onlyLinesIdx;
        for (std::string agencyUuidStr : capnpT.getOnlyAgenciesUuids())
        {
          agencyUuid = uuidGenerator(agencyUuidStr);
          onlyAgenciesIdx.push_back(agencyIndexesByUuid[agencyUuid]);
        }
        t->onlyAgenciesIdx = onlyAgenciesIdx;
        for (std::string nodeUuidStr : capnpT.getOnlyNodesUuids())
        {
          nodeUuid = uuidGenerator(nodeUuidStr);
          onlyNodesIdx.push_back(nodeIndexesByUuid[nodeUuid]);
        }
        t->onlyNodesIdx = onlyNodesIdx;
        for (std::string modeShortnameStr : capnpT.getOnlyModesShortnames())
        {
          onlyModesIdx.push_back(modeIndexesByShortname[modeShortnameStr]);
        }
        t->onlyModesIdx = onlyModesIdx;

        for (std::string lineUuidStr : capnpT.getExceptLinesUuids())
        {
          lineUuid = uuidGenerator(lineUuidStr);
          exceptLinesIdx.push_back(lineIndexesByUuid[lineUuid]);
        }
        t->exceptLinesIdx = exceptLinesIdx;
        for (std::string agencyUuidStr : capnpT.getExceptAgenciesUuids())
        {
          agencyUuid = uuidGenerator(agencyUuidStr);
          exceptAgenciesIdx.push_back(agencyIndexesByUuid[agencyUuid]);
        }
        t->exceptAgenciesIdx = exceptAgenciesIdx;
        for (std::string nodeUuidStr : capnpT.getExceptNodesUuids())
        {
          nodeUuid = uuidGenerator(nodeUuidStr);
          exceptNodesIdx.push_back(nodeIndexesByUuid[nodeUuid]);
        }
        t->exceptNodesIdx = exceptNodesIdx;
        for (std::string modeShortnameStr : capnpT.getExceptModesShortnames())
        {
          exceptModesIdx.push_back(modeIndexesByShortname[modeShortnameStr]);
        }
        t->exceptModesIdx = exceptModesIdx;

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