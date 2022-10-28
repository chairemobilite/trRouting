
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
    std::map<boost::uuids::uuid, Scenario>& ts,
    const std::map<boost::uuids::uuid, Service>& services,
    const std::map<boost::uuids::uuid, Line>& lines,
    const std::map<boost::uuids::uuid, Agency>& agencies,
    const std::map<boost::uuids::uuid, Node>& nodes,
    const std::map<std::string, Mode>& modes,
    std::string customPath
  ) {

    using T           = Scenario;
    using TCollection = scenarioCollection::ScenarioCollection;
    using cT          = scenarioCollection::Scenario;
    int ret = 0;

    ts.clear();

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
        std::vector<std::reference_wrapper<const Line>> onlyLines;
        std::vector<std::reference_wrapper<const Agency>> onlyAgencies;
        std::vector<std::reference_wrapper<const Node>> onlyNodes;
        std::vector<std::reference_wrapper<const Mode>> onlyModes;
        std::vector<std::reference_wrapper<const Line>> exceptLines;
        std::vector<std::reference_wrapper<const Agency>> exceptAgencies;
        std::vector<std::reference_wrapper<const Node>> exceptNodes;
        std::vector<std::reference_wrapper<const Mode>> exceptModes;
        boost::uuids::uuid serviceUuid;
        boost::uuids::uuid agencyUuid;
        boost::uuids::uuid nodeUuid;

        boost::uuids::uuid scenarioUuid = uuidGenerator(uuid);
        ts[scenarioUuid].uuid = scenarioUuid;

        ts[scenarioUuid].name           = capnpT.getName();
        ts[scenarioUuid].simulationUuid = simulationUuid.empty() ? uuidNilGenerator() : uuidGenerator(simulationUuid);
        
        for (std::string serviceUuidStr : capnpT.getServicesUuids())
        {
          serviceUuid = uuidGenerator(serviceUuidStr);
          if (services.count(serviceUuid) != 0)
          {
            servicesList.push_back(services.at(serviceUuid));
          }
        }
        ts[scenarioUuid].servicesList = servicesList;
        for (std::string lineUuidStr : capnpT.getOnlyLinesUuids())
        {
          boost::uuids::uuid lineUuid = uuidGenerator(lineUuidStr);
          if (lines.count(lineUuid) != 0)
          {
            onlyLines.push_back(lines.at(lineUuid));
          }
        }
        ts[scenarioUuid].onlyLines = onlyLines;
        for (std::string agencyUuidStr : capnpT.getOnlyAgenciesUuids())
        {
          agencyUuid = uuidGenerator(agencyUuidStr);
          if (agencies.count(agencyUuid) != 0)
          {
            onlyAgencies.push_back(agencies.at(agencyUuid));
          }
        }
        ts[scenarioUuid].onlyAgencies = onlyAgencies;
        for (std::string nodeUuidStr : capnpT.getOnlyNodesUuids())
        {
          nodeUuid = uuidGenerator(nodeUuidStr);
          if (nodes.count(nodeUuid) != 0)
          {
            onlyNodes.push_back(nodes.at(nodeUuid));
          }
        }
        ts[scenarioUuid].onlyNodes = onlyNodes;
        for (std::string modeShortnameStr : capnpT.getOnlyModesShortnames())
        {
          if (modes.count(modeShortnameStr) != 0)
          {
            onlyModes.push_back(modes.at(modeShortnameStr));
          }
        }
        ts[scenarioUuid].onlyModes = onlyModes;

        for (std::string lineUuidStr : capnpT.getExceptLinesUuids())
        {
          boost::uuids::uuid lineUuid = uuidGenerator(lineUuidStr);
          if (lines.count(lineUuid) != 0)
          {
            exceptLines.push_back(lines.at(lineUuid));
          }
        }
        ts[scenarioUuid].exceptLines = exceptLines;
        for (std::string agencyUuidStr : capnpT.getExceptAgenciesUuids())
        {
          agencyUuid = uuidGenerator(agencyUuidStr);
          if (agencies.count(agencyUuid) != 0)
          {
            exceptAgencies.push_back(agencies.at(agencyUuid));
          }
        }
        ts[scenarioUuid].exceptAgencies = exceptAgencies;
        for (std::string nodeUuidStr : capnpT.getExceptNodesUuids())
        {
          nodeUuid = uuidGenerator(nodeUuidStr);
          if (nodes.count(nodeUuid) != 0)
          {
            exceptNodes.push_back(nodes.at(nodeUuid));
          }
        }
        ts[scenarioUuid].exceptNodes = exceptNodes;
        for (std::string modeShortnameStr : capnpT.getExceptModesShortnames())
        {
          if (modes.count(modeShortnameStr) != 0)
          {
            exceptModes.push_back(modes.at(modeShortnameStr));
          }
        }
        ts[scenarioUuid].exceptModes = exceptModes;

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
