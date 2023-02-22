#ifndef TR_CONNECTION_CACHE
#define TR_CONNECTION_CACHE

#include <vector>
#include <optional>
#include <boost/uuid/uuid.hpp>
#include <memory>


namespace TrRouting {

class Connection;
class ConnectionSet;


/**
 * @brief Caches the connection sets used by different transit scenarios.
 *
 * // TODO Currently caches only the last queried scenario. We could make this a
 * real lru cache eventually, or not, if we find a better way to handle scenario
 * query data.
 */
class ScenarioConnectionCache {

  public:
    ScenarioConnectionCache()
    {
       
    };
    virtual ~ScenarioConnectionCache() {}
    
    std::optional<std::shared_ptr<ConnectionSet>> get(boost::uuids::uuid uuid) const;
    void set(boost::uuids::uuid uuid, std::shared_ptr<ConnectionSet> cache);

  private:
    std::optional<boost::uuids::uuid> lastUuid;
    std::shared_ptr<ConnectionSet> lastConnection;
};

}

#endif // TR_CONNECTION_CACHE
