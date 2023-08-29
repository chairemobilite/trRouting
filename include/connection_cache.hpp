#ifndef TR_CONNECTION_CACHE
#define TR_CONNECTION_CACHE

#include <vector>
#include <optional>
#include <boost/uuid/uuid.hpp>
#include <memory>
#include <map>

namespace TrRouting {

class Connection;
class ConnectionSet;


/**
 * @brief Interface for the caches of the connection sets used by different transit scenarios.
 *
 * // TODO Currently caches only the last queried scenario or all of them. We could make this a
 * real lru cache eventually, or not, if we find a better way to handle scenario
 * query data.
 */
class ScenarioConnectionCache {

  public:
    virtual ~ScenarioConnectionCache() {}
    
    virtual std::optional<std::shared_ptr<ConnectionSet>> get(boost::uuids::uuid uuid) const = 0;
    virtual void set(boost::uuids::uuid uuid, std::shared_ptr<ConnectionSet> cache) = 0;
};

/**
 * @brief Caches the last connection sets used by different transit scenarios.
 *
 */
class ScenarioConnectionCacheOne : public ScenarioConnectionCache {

  public:
    ScenarioConnectionCacheOne()
    {

    }
    virtual ~ScenarioConnectionCacheOne() {}

    virtual std::optional<std::shared_ptr<ConnectionSet>> get(boost::uuids::uuid uuid) const;
    virtual void set(boost::uuids::uuid uuid, std::shared_ptr<ConnectionSet> cache);

  private:
    std::optional<boost::uuids::uuid> lastUuid;
    std::shared_ptr<ConnectionSet> lastConnection;
};


/**
 * @brief Caches the all connection sets used by different transit scenarios.
 *
 */
class ScenarioConnectionCacheAll : public ScenarioConnectionCache {

  public:
    ScenarioConnectionCacheAll()
    {

    }
    virtual ~ScenarioConnectionCacheAll(){}

    virtual std::optional<std::shared_ptr<ConnectionSet>> get(boost::uuids::uuid uuid) const;
    virtual void set(boost::uuids::uuid uuid, std::shared_ptr<ConnectionSet> cache);

  private:
    std::map<boost::uuids::uuid, std::shared_ptr<ConnectionSet> > connectionSets;
};

}

#endif // TR_CONNECTION_CACHE
