
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

  void CacheFetcher::getScenarios(
    std::vector<std::unique_ptr<Scenario>>& ts,
    std::map<boost::uuids::uuid, int>& tIndexesByUuid,
    std::map<boost::uuids::uuid, int>& serviceIndexesByUuid,
    std::map<boost::uuids::uuid, int>& lineIndexesByUuid,
    std::map<boost::uuids::uuid, int>& agencyIndexesByUuid,
    std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
    std::map<std::string, int>& modeIndexesByShortname,
    Parameters& params,
    std::string customPath
  ) {

    using T           = Scenario;
    using TCollection = scenarioCollection::ScenarioCollection;
    using cT          = scenarioCollection::Scenario;

    ts.clear();
    tIndexesByUuid.clear();

    std::string tStr  = "scenarios";
    std::string TStr  = "Scenarios";

    std::string cacheFileName{tStr};
    boost::uuids::string_generator uuidGenerator;
    boost::uuids::nil_generator    uuidNilGenerator;

    std::cout << "Fetching " << tStr << " from cache..." << std::endl;
    
    if (CacheFetcher::capnpCacheFileExists(cacheFileName + ".capnpbin", params, customPath))
    {
      int fd = open((CacheFetcher::getFilePath(cacheFileName, params, customPath) + ".capnpbin").c_str(), O_RDWR);
      ::capnp::PackedFdMessageReader capnpTCollectionMessage(fd, {16 * 1024 * 1024});
      TCollection::Reader capnpTCollection = capnpTCollectionMessage.getRoot<TCollection>();
      for (cT::Reader capnpT : capnpTCollection.getScenarios())
      {
        std::string uuid           {capnpT.getUuid()};
        std::string simulationUuid {capnpT.getSimulationUuid()};

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

        std::unique_ptr<T> t = std::make_unique<T>();

        t->uuid           = uuidGenerator(uuid);
        t->name           = capnpT.getName();
        t->simulationUuid = simulationUuid.empty() ? uuidNilGenerator() : uuidGenerator(simulationUuid);
        
        for (std::string serviceUuidStr : capnpT.getServicesUuids())
        {
          serviceUuid = uuidGenerator(serviceUuidStr);
          if (serviceIndexesByUuid.count(serviceUuid) != 0)
          {
            servicesIdx.push_back(serviceIndexesByUuid[serviceUuid]);
          }
        }
        t->servicesIdx = servicesIdx;
        for (std::string lineUuidStr : capnpT.getOnlyLinesUuids())
        {
          lineUuid = uuidGenerator(lineUuidStr);
          if (lineIndexesByUuid.count(lineUuid) != 0)
          {
            onlyLinesIdx.push_back(lineIndexesByUuid[lineUuid]);
          }
        }
        t->onlyLinesIdx = onlyLinesIdx;
        for (std::string agencyUuidStr : capnpT.getOnlyAgenciesUuids())
        {
          agencyUuid = uuidGenerator(agencyUuidStr);
          if (agencyIndexesByUuid.count(agencyUuid) != 0)
          {
            onlyAgenciesIdx.push_back(agencyIndexesByUuid[agencyUuid]);
          }
        }
        t->onlyAgenciesIdx = onlyAgenciesIdx;
        for (std::string nodeUuidStr : capnpT.getOnlyNodesUuids())
        {
          nodeUuid = uuidGenerator(nodeUuidStr);
          if (nodeIndexesByUuid.count(nodeUuid) != 0)
          {
            onlyNodesIdx.push_back(nodeIndexesByUuid[nodeUuid]);
          }
        }
        t->onlyNodesIdx = onlyNodesIdx;
        for (std::string modeShortnameStr : capnpT.getOnlyModesShortnames())
        {
          if (modeIndexesByShortname.count(modeShortnameStr) != 0)
          {
            onlyModesIdx.push_back(modeIndexesByShortname[modeShortnameStr]);
          }
        }
        t->onlyModesIdx = onlyModesIdx;

        for (std::string lineUuidStr : capnpT.getExceptLinesUuids())
        {
          lineUuid = uuidGenerator(lineUuidStr);
          if (lineIndexesByUuid.count(lineUuid) != 0)
          {
            exceptLinesIdx.push_back(lineIndexesByUuid[lineUuid]);
          }
        }
        t->exceptLinesIdx = exceptLinesIdx;
        for (std::string agencyUuidStr : capnpT.getExceptAgenciesUuids())
        {
          agencyUuid = uuidGenerator(agencyUuidStr);
          if (agencyIndexesByUuid.count(agencyUuid) != 0)
          {
            exceptAgenciesIdx.push_back(agencyIndexesByUuid[agencyUuid]);
          }
        }
        t->exceptAgenciesIdx = exceptAgenciesIdx;
        for (std::string nodeUuidStr : capnpT.getExceptNodesUuids())
        {
          nodeUuid = uuidGenerator(nodeUuidStr);
          if (nodeIndexesByUuid.count(nodeUuid) != 0)
          {
            exceptNodesIdx.push_back(nodeIndexesByUuid[nodeUuid]);
          }
        }
        t->exceptNodesIdx = exceptNodesIdx;
        for (std::string modeShortnameStr : capnpT.getExceptModesShortnames())
        {
          if (modeIndexesByShortname.count(modeShortnameStr) != 0)
          {
            exceptModesIdx.push_back(modeIndexesByShortname[modeShortnameStr]);
          }
        }
        t->exceptModesIdx = exceptModesIdx;

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

#endif // TR_SCENARIOS_CACHE_FETCHER