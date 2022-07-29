
#ifndef TR_SCENARIOS_CACHE_FETCHER
#define TR_SCENARIOS_CACHE_FETCHER

#include <string>
#include <vector>
#include <errno.h>
#include <fcntl.h>
#include <kj/exception.h>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <capnp/serialize-packed.h>
#include "cache_fetcher.hpp"
#include "scenario.hpp"
#include "capnp/scenarioCollection.capnp.h"
#include "spdlog/spdlog.h"

namespace TrRouting
{

  int CacheFetcher::getScenarios(
    std::vector<std::unique_ptr<Scenario>>& ts,
    std::map<boost::uuids::uuid, int>& tIndexesByUuid,
    const std::map<boost::uuids::uuid, Service>& services,
    const std::map<boost::uuids::uuid, int>& lineIndexesByUuid,
    const std::map<boost::uuids::uuid, Agency>& agencies,
    const std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
    const std::map<std::string, Mode>& modes,
    std::string customPath
  ) {

    using T           = Scenario;
    using TCollection = scenarioCollection::ScenarioCollection;
    using cT          = scenarioCollection::Scenario;
    int ret = 0;

    ts.clear();
    tIndexesByUuid.clear();

    std::string tStr  = "scenarios";
    std::string TStr  = "Scenarios";

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
      for (cT::Reader capnpT : capnpTCollection.getScenarios())
      {
        std::string uuid           {capnpT.getUuid()};
        std::string simulationUuid {capnpT.getSimulationUuid()};

        std::vector<std::reference_wrapper<const Service>> servicesList;
        std::vector<int> onlyLinesIdx;
        std::vector<std::reference_wrapper<const Agency>> onlyAgencies;
        std::vector<int> onlyNodesIdx;
        std::vector<std::reference_wrapper<const Mode>> onlyModes;
        std::vector<int> exceptLinesIdx;
        std::vector<std::reference_wrapper<const Agency>> exceptAgencies;
        std::vector<int> exceptNodesIdx;
        std::vector<std::reference_wrapper<const Mode>> exceptModes;
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
          if (services.count(serviceUuid) != 0)
          {
            servicesList.push_back(services.at(serviceUuid));
          }
        }
        t->servicesList = servicesList;
        for (std::string lineUuidStr : capnpT.getOnlyLinesUuids())
        {
          lineUuid = uuidGenerator(lineUuidStr);
          if (lineIndexesByUuid.count(lineUuid) != 0)
          {
            onlyLinesIdx.push_back(lineIndexesByUuid.at(lineUuid));
          }
        }
        t->onlyLinesIdx = onlyLinesIdx;
        for (std::string agencyUuidStr : capnpT.getOnlyAgenciesUuids())
        {
          agencyUuid = uuidGenerator(agencyUuidStr);
          if (agencies.count(agencyUuid) != 0)
          {
            onlyAgencies.push_back(agencies.at(agencyUuid));
          }
        }
        t->onlyAgencies = onlyAgencies;
        for (std::string nodeUuidStr : capnpT.getOnlyNodesUuids())
        {
          nodeUuid = uuidGenerator(nodeUuidStr);
          if (nodeIndexesByUuid.count(nodeUuid) != 0)
          {
            onlyNodesIdx.push_back(nodeIndexesByUuid.at(nodeUuid));
          }
        }
        t->onlyNodesIdx = onlyNodesIdx;
        for (std::string modeShortnameStr : capnpT.getOnlyModesShortnames())
        {
          if (modes.count(modeShortnameStr) != 0)
          {
            onlyModes.push_back(modes.at(modeShortnameStr));
          }
        }
        t->onlyModes = onlyModes;

        for (std::string lineUuidStr : capnpT.getExceptLinesUuids())
        {
          lineUuid = uuidGenerator(lineUuidStr);
          if (lineIndexesByUuid.count(lineUuid) != 0)
          {
            exceptLinesIdx.push_back(lineIndexesByUuid.at(lineUuid));
          }
        }
        t->exceptLinesIdx = exceptLinesIdx;
        for (std::string agencyUuidStr : capnpT.getExceptAgenciesUuids())
        {
          agencyUuid = uuidGenerator(agencyUuidStr);
          if (agencies.count(agencyUuid) != 0)
          {
            exceptAgencies.push_back(agencies.at(agencyUuid));
          }
        }
        t->exceptAgencies = exceptAgencies;
        for (std::string nodeUuidStr : capnpT.getExceptNodesUuids())
        {
          nodeUuid = uuidGenerator(nodeUuidStr);
          if (nodeIndexesByUuid.count(nodeUuid) != 0)
          {
            exceptNodesIdx.push_back(nodeIndexesByUuid.at(nodeUuid));
          }
        }
        t->exceptNodesIdx = exceptNodesIdx;
        for (std::string modeShortnameStr : capnpT.getExceptModesShortnames())
        {
          if (modes.count(modeShortnameStr) != 0)
          {
            exceptModes.push_back(modes.at(modeShortnameStr));
          }
        }
        t->exceptModes = exceptModes;

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

#endif // TR_SCENARIOS_CACHE_FETCHER
