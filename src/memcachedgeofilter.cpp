#include "memcachedgeofilter.hpp"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <thread>
#include "point.hpp"
#include "node.hpp"
#include "spdlog/spdlog.h"

namespace TrRouting {

  // Initialize the connection pool with specified size
  void MemcachedGeoFilter::initializePool(size_t poolSize, const std::string& memcachedServersStr) {
    std::lock_guard<std::mutex> lock(poolMutex);

    // Auto-detect thread count if poolSize is 0
    if (poolSize == 0) {
      unsigned int detected = std::thread::hardware_concurrency();
      // Ensure at least 2 connections
      poolSize = std::max(static_cast<size_t>(detected), static_cast<size_t>(2));
      spdlog::info("Auto-detected thread count for connection pool: {}", poolSize);
    }

    // Configure connection string with all servers
    // Format: --SERVER=server1:port1 --SERVER=server2:port2
    // Calculate minimum pool size with half the pool size, with at least one
    size_t minPoolSize = std::max(poolSize/2, static_cast<size_t>(1));
    std::string poolOptions = "--POOL-MIN=" + std::to_string(minPoolSize) + " --POOL-MAX=" + std::to_string(poolSize);

    // Add all servers from the servers string
    std::istringstream serverStream(memcachedServersStr);
    std::string server;
    while (std::getline(serverStream, server, ',')) {
      // Trim whitespace if any
      server.erase(0, server.find_first_not_of(" \t"));
      server.erase(server.find_last_not_of(" \t") + 1);

      if (!server.empty()) {
        poolOptions += " --SERVER=" + server;
      }
    }

    spdlog::debug("Creating memcached pool with options: {}", poolOptions);

    // Create the pool
    memcPool = memcached_pool(poolOptions.c_str(), poolOptions.length());

    if (memcPool == NULL) {
      spdlog::error("Failed to create memcached connection pool");
    } else {
      spdlog::info("Created memcached connection pool with initial size {} and max size {}",
                  minPoolSize, poolSize);
    }
  }

  // MemcachedGeoFilter constructor
  MemcachedGeoFilter::MemcachedGeoFilter(
                                         GeoFilter* baseGeoFilter,
                                         const std::string& memcachedServers,
                                         uint32_t cacheExpiry,
                                         size_t poolSize
                                         ) : baseGeoFilter(baseGeoFilter),
                                             cacheExpirySeconds(cacheExpiry)
  {
    // Initialize the connection pool
    initializePool(poolSize, memcachedServers);
  }

  // Destructor
  MemcachedGeoFilter::~MemcachedGeoFilter() {
    std::lock_guard<std::mutex> lock(poolMutex);
    if (memcPool) {
      memcached_pool_destroy(memcPool);
    }
  }

  // Helper function to generate a cache key
  std::string MemcachedGeoFilter::generateCacheKey(
                                                   const Point &point, 
                                                   int maxWalkingTravelTime,
                                                   float walkingSpeedMetersPerSecond,
                                                   bool reversed
                                                   )
  {
    std::stringstream ss;
    // Create a unique key based on the function parameters
    ss << "footpaths:" 
       << std::fixed << std::setprecision(6) << point.latitude << ":"  // Using actual field names
       << std::fixed << std::setprecision(6) << point.longitude << ":" // from Point class
       << maxWalkingTravelTime << ":"
       << walkingSpeedMetersPerSecond << ":"
       << (reversed ? "1" : "0");
    return ss.str();
  }

  // Serialize a vector of SerializableNodeTimeDistance to a string
  std::string MemcachedGeoFilter::serializeResults(
                                                   const std::vector<SerializableNodeTimeDistance> &results
                                                   )
  {
    try {
      std::stringstream ss;
      boost::archive::binary_oarchive oa(ss);
      oa << results;
      return ss.str();
    } catch (const std::exception& e) {
      spdlog::error("Serialization error: {}", e.what());
      return "";
    }
  }

  // Deserialize a string to a vector of SerializableNodeTimeDistance
  std::vector<SerializableNodeTimeDistance> MemcachedGeoFilter::deserializeResults(
                                                                                   const std::string &data
                                                                                   )
  {
    try {
      std::vector<SerializableNodeTimeDistance> results;
      std::stringstream ss(data);
      boost::archive::binary_iarchive ia(ss);
      ia >> results;
      return results;
    } catch (const std::exception& e) {
      spdlog::error("Deserialization error: {}", e.what());
      return std::vector<SerializableNodeTimeDistance>();
    }
  }

  // Convert NodeTimeDistance to SerializableNodeTimeDistance for caching
  std::vector<SerializableNodeTimeDistance> MemcachedGeoFilter::convertToSerializable(
                                                                                      const std::vector<NodeTimeDistance> &results
                                                                                      )
  {
    std::vector<SerializableNodeTimeDistance> serializableResults;
    serializableResults.reserve(results.size());
    
    for (const auto& result : results) {
      serializableResults.emplace_back(result);
    }
    
    return serializableResults;
  }

  // Convert SerializableNodeTimeDistance back to NodeTimeDistance
  std::vector<NodeTimeDistance> MemcachedGeoFilter::convertFromSerializable(
                                                                            const std::vector<SerializableNodeTimeDistance> &serializableResults,
                                                                            const std::map<boost::uuids::uuid, Node> &nodes
                                                                            )
  {
    std::vector<NodeTimeDistance> results;
    results.reserve(serializableResults.size());
    
    for (const auto& serialResult : serializableResults) {
      // Find the node in the nodes map
      auto nodeIter = nodes.find(serialResult.nodeUuid);
      if (nodeIter != nodes.end()) {
        // Create a new NodeTimeDistance using the node reference constructor
        results.emplace_back(nodeIter->second, serialResult.time, serialResult.distance);
      }
    }
    
    return results;
  }

  /**
   * @brief RAII guard for automatic Memcached connection pool management.
   *
   * Acquires a connection from the pool on construction and automatically releases it
   * on destruction, ensuring proper cleanup even when exceptions occur. Non-copyable
   * for safety. Should be the only we way that we call memcached_pool_fetch and
   * memcached_pool_release, to insure proper release of each connection.
   *
   * Usage:
   * @code
   * MemcachedConnectionGuard guard(memcPool);
   * if (guard.valid()) {
   *     memcached_set(guard.get(), ...);
   * }
   * // Connection automatically released when guard goes out of scope
   * @endcode
   *
   * @warning Do not manually release the connection - the guard handles this automatically
   */
  class MemcachedConnectionGuard {
  private:
    memcached_pool_st* pool;
    memcached_st* connection;

  public:
    MemcachedConnectionGuard(memcached_pool_st* pool) : pool(pool), connection(nullptr) {
      if (pool) {
        memcached_return_t rc;
        connection = memcached_pool_fetch(pool, NULL, &rc);
        if (connection == nullptr) {
          spdlog::error("Failed to acquire connection from pool: {}",
                        memcached_strerror(NULL, rc));
        }
      }
    }

    ~MemcachedConnectionGuard() {
      if (connection && pool) {
        memcached_return_t rc = memcached_pool_release(pool, connection);
        if (rc != MEMCACHED_SUCCESS) {
          spdlog::warn("Failed to return connection to pool: {}",
                       memcached_strerror(NULL, rc));
          memcached_free(connection);
        }
      }
    }

    memcached_st* get() { return connection; }
    bool valid() const { return connection != nullptr; }

    // Non-copyable
    MemcachedConnectionGuard(const MemcachedConnectionGuard&) = delete;
    MemcachedConnectionGuard& operator=(const MemcachedConnectionGuard&) = delete;
  };

  // Thread-safe implementation of the main interface method using connection pool
  std::vector<NodeTimeDistance> MemcachedGeoFilter::getAccessibleNodesFootpathsFromPoint(
                                                                                         const Point &point,
                                                                                         const std::map<boost::uuids::uuid, Node> &nodes,
                                                                                         int maxWalkingTravelTime,
                                                                                         float walkingSpeedMetersPerSecond,
                                                                                         bool reversed
                                                                                         ) {
    // Generate cache key
    std::string cacheKey = generateCacheKey(
                                            point, maxWalkingTravelTime, walkingSpeedMetersPerSecond, reversed);
    
    // Check if pool is available
    if (!memcPool) {
      spdlog::error("Memcached connection pool is not initialized");
      return baseGeoFilter->getAccessibleNodesFootpathsFromPoint(
        point, nodes, maxWalkingTravelTime, walkingSpeedMetersPerSecond, reversed);
    }

    // Acquire a connection from the pool, will be freed automatically when out of scope
    MemcachedConnectionGuard mcConnectionGuard(memcPool);

    if (!mcConnectionGuard.valid()) {
      return baseGeoFilter->getAccessibleNodesFootpathsFromPoint(
        point, nodes, maxWalkingTravelTime, walkingSpeedMetersPerSecond, reversed);
    }

    // Try to get from cache
    size_t valueLength;
    uint32_t flags;
    memcached_return_t rc;
    char *cachedResult = memcached_get(
                                       mcConnectionGuard.get(),
                                       cacheKey.c_str(), 
                                       cacheKey.length(), 
                                       &valueLength, 
                                       &flags, 
                                       &rc
                                       );
    
    std::vector<NodeTimeDistance> results;

    if (rc == MEMCACHED_SUCCESS && cachedResult != NULL) {
      spdlog::debug("Memcached cache hit ({}) - {}", cacheKey, valueLength);
      // Cache hit
      std::string resultData(cachedResult, valueLength);
      free(cachedResult);
      
      try {
        // Deserialize the cached result
        auto serializableResults = deserializeResults(resultData);
        
        // Convert back to NodeTimeDistance objects with proper references
        results = convertFromSerializable(serializableResults, nodes);
      } catch (const std::exception& e) {
        // If deserialization fails, log error and fall through to compute result
        spdlog::warn("Cache deserialization error: {}", e.what());
        // Reset results to empty to trigger calculation
        results.clear();
      }
    }
    
    // Cache miss or error, compute the result using the base provider
    if (results.empty()) {
      results = baseGeoFilter->getAccessibleNodesFootpathsFromPoint(
        point, nodes, maxWalkingTravelTime, walkingSpeedMetersPerSecond, reversed);

      // Convert to serializable format and serialize
      auto serializableResults = convertToSerializable(results);
      std::string serializedResults = serializeResults(serializableResults);

      if (!serializedResults.empty()) {
        // Store in cache
        rc = memcached_set(
                           mcConnectionGuard.get(),
                           cacheKey.c_str(),
                           cacheKey.length(),
                           serializedResults.c_str(),
                           serializedResults.length(),
                           cacheExpirySeconds,
                           (uint32_t)0
                           );

        if (rc != MEMCACHED_SUCCESS) {
          spdlog::warn("Failed to store footpaths in cache: {}", memcached_strerror(mcConnectionGuard.get(), rc));
        }
      }
    }
    
    return results;
  }

  // Thread-safe flush the cache
  bool MemcachedGeoFilter::flushCache() {
    std::lock_guard<std::mutex> lock(poolMutex);

    if (!memcPool) {
      spdlog::error("Memcached connection pool is not initialized");
      return false;
    }

    // Acquire a connection from the pool, will be freed automatically when out of scope
    MemcachedConnectionGuard mcConnectionGuard(memcPool);

    if (!mcConnectionGuard.valid()) {
      return false;
    }

    // Flush the cache
    memcached_return_t rc = memcached_flush(mcConnectionGuard.get(), 0);
    bool success = (rc == MEMCACHED_SUCCESS);

    if (!success) {
      spdlog::error("Failed to flush cache: {}", memcached_strerror(mcConnectionGuard.get(), rc));
    }

    return success;
  }
  
} // namespace TrRouting
