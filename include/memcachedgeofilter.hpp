#pragma once

#include <vector>
#include <map>
#include <string>
#include <mutex>
#include <libmemcached/memcached.h>
#include <libmemcached/util.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_serialize.hpp> //This need to be included before any other serialization includes
#include <boost/serialization/vector.hpp>

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
    std::string persistPath; // Path to single cache file

    // In-memory cache for persistence (mirrors what's in memcached)
    std::map<std::string, std::string> localCache;
    std::mutex localCacheMutex;

    // Nodes validation - stored from cache file, validated on first use
    // Any change to any node (add/delete, change location or uuid) invalidates the cache
    uint32_t loadedNodesCount = 0;
    uint64_t loadedNodesHash = 0;
    bool cacheValidated = false;
    std::mutex validationMutex;

    // Helper function to generate a cache key
    std::string generateCacheKey(const Point &point,
                                int maxWalkingTravelTime,
                                float walkingSpeedMetersPerSecond,
                                bool reversed);

    // Compute a hash signature from all nodes (count + UUIDs + locations)
    static uint64_t computeNodesHash(const std::map<boost::uuids::uuid, Node> &nodes);

    // Validate loaded cache against current nodes, clear cache if mismatch
    void validateCacheWithNodes(const std::map<boost::uuids::uuid, Node> &nodes);

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
        uint32_t cacheExpiry = 0,  // 0 means never expire
        size_t poolSize = 0,  // 0 means auto-detect thread count
        const std::string& persistPath = ""  // Optional path for disk persistence
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

    // Flush the cache (clears memcached only)
    bool flushCache();

    // Reset cache completely: flush memcached, clear local cache, and delete cache file
    bool resetCache();

    // Save cache to single file (for persistence)
    // Requires nodes to compute and store the nodes hash for validation on reload
    bool saveCacheToFile(const std::map<boost::uuids::uuid, Node> &nodes);

    // Load cache from single file (called on startup if persistence is enabled)
    // Validation against actual nodes happens on first getAccessibleNodesFootpathsFromPoint call
    bool loadCacheFromFile();

    // Get the number of cached entries
    size_t getCacheSize();

    // Check if the cache has been validated against nodes
    bool isCacheValidated() const { return cacheValidated; }

    // Check if disk persistence is enabled
    bool isPersistenceEnabled() const { return !persistPath.empty(); }
};

} // namespace TrRouting
