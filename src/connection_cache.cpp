#include "connection_cache.hpp"
#include "connection_set.hpp"
#include "spdlog/spdlog.h"
#include "connection.hpp"
#include <boost/uuid/uuid_io.hpp>


namespace TrRouting {
  // ScenarioConnectionCacheOne
  std::optional<std::shared_ptr<ConnectionSet>> ScenarioConnectionCacheOne::get(boost::uuids::uuid uuid) const {
    std::shared_lock lock(mutex); //Sharing lock when reading
    if (uuid == lastUuid) {
      return std::optional(lastConnection);
    } else {
      return std::nullopt;
    }
  }

  void ScenarioConnectionCacheOne::set(boost::uuids::uuid uuid, std::shared_ptr<ConnectionSet> cache) {
    std::unique_lock lock(mutex); //Exclusive lock when writing
    lastUuid = uuid;
    lastConnection = cache;
  }

  // ScenarioConnectionCacheAll
  std::optional<std::shared_ptr<ConnectionSet>> ScenarioConnectionCacheAll::get(boost::uuids::uuid uuid) const {
    std::shared_lock lock(mutex); //Sharing lock when reading
    // Lookup the scenario uuid in the map. If found, returns it, if not, return a null_opt
    auto connectionSetItr = connectionSets.find(uuid);
    if (connectionSetItr != connectionSets.end()) {
      return std::optional(connectionSetItr->second);
    } else {
      return std::nullopt;
    }
  }

  void ScenarioConnectionCacheAll::set(boost::uuids::uuid uuid, std::shared_ptr<ConnectionSet> cache) {
    spdlog::debug("Caching connection set for scenario {}", boost::uuids::to_string(uuid));
    std::unique_lock lock(mutex); //Exclusive lock when writing
    connectionSets[uuid] = cache;
  }
}
