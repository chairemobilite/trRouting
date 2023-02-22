#include "connection_cache.hpp"
#include "connection_set.hpp"
#include "spdlog/spdlog.h"
#include "connection.hpp"


namespace TrRouting {

  std::optional<std::shared_ptr<ConnectionSet>> ScenarioConnectionCache::get(boost::uuids::uuid uuid) const {
    if (uuid == lastUuid) {
      return std::optional(lastConnection);
    } else {
      return std::nullopt;
    }
  }

  void ScenarioConnectionCache::set(boost::uuids::uuid uuid, std::shared_ptr<ConnectionSet> cache) {
    lastUuid = uuid;
    lastConnection = cache;
  }
}