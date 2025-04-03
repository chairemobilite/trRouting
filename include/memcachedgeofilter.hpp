#pragma once

#include <vector>
#include <map>
#include <string>
#include <mutex>
#include <libmemcached/memcached.h>
#include <libmemcached/util.h>
#include <boost/uuid/uuid.hpp>
#include <boost/serialization/vector.hpp>
#include "serialization_boost_uuid.hpp" // Custom serialization for boost::uuid

#include "geofilter.hpp"
#include "node.hpp"

namespace TrRouting {

// Serializable version of NodeTimeDistance for caching
struct SerializableNodeTimeDistance {
    boost::uuids::uuid nodeUuid;  // Using uuid instead of node reference
    int time;
    int distance;
    
    // Default constructor (required for deserialization)
    SerializableNodeTimeDistance() : time(0), distance(0) {}
    
    // Conversion from NodeTimeDistance
    SerializableNodeTimeDistance(const NodeTimeDistance& ntd)
        : nodeUuid(ntd.node.uuid),  // Using the actual uuid field
          time(ntd.time),
          distance(ntd.distance)
    {
    }
    
    // Serialization support
    template<class Archive>
    void serialize(Archive & ar, const unsigned int /* version */) {
        ar & nodeUuid;
        ar & time;
        ar & distance;
    }
};

// Memcached-backed implementation of GeoFilter
class MemcachedGeoFilter : public GeoFilter {
private:
    memcached_pool_st *memcPool;
    GeoFilter* baseGeoFilter;
    uint32_t cacheExpirySeconds;
    std::mutex poolMutex; // Mutex for thread safety of pool operations

    // Helper function to generate a cache key
    std::string generateCacheKey(const Point &point, 
                                int maxWalkingTravelTime,
                                float walkingSpeedMetersPerSecond,
                                bool reversed);
    
    // Serialize a vector of SerializableNodeTimeDistance to a string
    std::string serializeResults(const std::vector<SerializableNodeTimeDistance> &results);
    
    // Deserialize a string to a vector of SerializableNodeTimeDistance
    std::vector<SerializableNodeTimeDistance> deserializeResults(const std::string &data);
    
    // Convert NodeTimeDistance to SerializableNodeTimeDistance for caching
    std::vector<SerializableNodeTimeDistance> convertToSerializable(
        const std::vector<NodeTimeDistance> &results);
    
    // Convert SerializableNodeTimeDistance back to NodeTimeDistance
    std::vector<NodeTimeDistance> convertFromSerializable(
        const std::vector<SerializableNodeTimeDistance> &serializableResults,
        const std::map<boost::uuids::uuid, Node> &nodes);

    // Initialize the connection pool with specified size
  void initializePool(size_t poolSize, const std::string& memcachedServersStr);
public:
    // Constructor with configurable memcached servers and cache expiry
    MemcachedGeoFilter(
        GeoFilter* baseGeoFilter,
        const std::string& memcachedServers = "localhost:11211",
        uint32_t cacheExpiry = 3600,
        size_t poolSize = 0  // 0 means auto-detect thread count
    );
    
    // Destructor
    ~MemcachedGeoFilter();
    
    // Implementation of the GeoFilter interface method
    std::vector<NodeTimeDistance> getAccessibleNodesFootpathsFromPoint(
        const Point &point,
        const std::map<boost::uuids::uuid, Node> &nodes,
        int maxWalkingTravelTime,
        float walkingSpeedMetersPerSecond,
        bool reversed = false) override;
    
    // Flush the cache
    bool flushCache();
};

} // namespace TrRouting
