#include "memcachedgeofilter.hpp"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <functional>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/filesystem.hpp>
#include <thread>
#include "point.hpp"
#include "node.hpp"
#include "spdlog/spdlog.h"

namespace TrRouting {

  // Cache file magic header and version
  static const char CACHE_MAGIC[4] = {'T', 'R', 'F', 'C'}; // TRFC stands for TrRouting Footpaths Cache
  // Version 2: Added nodes hash for validation
  static const uint32_t CACHE_VERSION = 2;

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
                                         size_t poolSize,
                                         const std::string& persistPathParam
                                         ) : baseGeoFilter(baseGeoFilter),
                                             cacheExpirySeconds(cacheExpiry),
                                             persistPath(persistPathParam)
  {
    // Initialize the connection pool
    initializePool(poolSize, memcachedServers);

    // Load persisted cache from disk on startup
    if (isPersistenceEnabled()) {
      loadCacheFromFile();
    }
  }

  // Destructor
  MemcachedGeoFilter::~MemcachedGeoFilter() {
    std::lock_guard<std::mutex> lock(poolMutex);
    if (memcPool) {
      memcached_pool_destroy(memcPool);
    }
  }

  // Compute a hash signature from all nodes
  // This creates a unique fingerprint based on node count, UUIDs, and locations
  uint64_t MemcachedGeoFilter::computeNodesHash(const std::map<boost::uuids::uuid, Node> &nodes) {
    uint64_t hash = nodes.size(); // Start with count

    // Use a hash combining technique based on boost::hash_combine
    // 0x9e3779b97f4a7c15 is the 64-bit golden ratio constant (2^64 / φ),
    // which provides optimal bit dispersion for hash mixing
    for (const auto& pair : nodes) {
      const Node& node = pair.second;

      // Hash the UUID bytes
      for (const auto& byte : node.uuid) {
        hash ^= static_cast<uint64_t>(byte) + 0x9e3779b97f4a7c15ULL + (hash << 6) + (hash >> 2);
      }

      // Hash the location (convert to fixed-point for consistency)
      // Use 6 decimal places precision (multiply by 1000000)
      int64_t latFixed = static_cast<int64_t>(node.point->latitude * 1000000.0);
      int64_t lonFixed = static_cast<int64_t>(node.point->longitude * 1000000.0);

      hash ^= static_cast<uint64_t>(latFixed) + 0x9e3779b97f4a7c15ULL + (hash << 6) + (hash >> 2);
      hash ^= static_cast<uint64_t>(lonFixed) + 0x9e3779b97f4a7c15ULL + (hash << 6) + (hash >> 2);
    }

    return hash;
  }

  // Validate loaded cache against current nodes
  void MemcachedGeoFilter::validateCacheWithNodes(const std::map<boost::uuids::uuid, Node> &nodes) {
    std::lock_guard<std::mutex> lock(validationMutex);

    if (cacheValidated) {
      return; // Already validated
    }

    cacheValidated = true;

    // If we have no loaded metadata, nothing to validate
    if (loadedNodesCount == 0 && loadedNodesHash == 0) {
      return;
    }

    // Compute current nodes hash
    uint64_t currentHash = computeNodesHash(nodes);
    uint32_t currentCount = static_cast<uint32_t>(nodes.size());

    // Check for mismatch
    if (currentCount != loadedNodesCount || currentHash != loadedNodesHash) {
      spdlog::warn("Cache invalidated: nodes have changed (count: {} -> {}, hash: {:016x} -> {:016x})",
                   loadedNodesCount, currentCount, loadedNodesHash, currentHash);
      spdlog::info("Clearing cached footpaths data - will recalculate on demand");

      // Clear the local cache
      {
        std::lock_guard<std::mutex> cacheLock(localCacheMutex);
        localCache.clear();
      }

      // Also flush memcached
      flushCache();

      // Reset loaded metadata
      loadedNodesCount = 0;
      loadedNodesHash = 0;
    } else {
      spdlog::info("Cache validated: {} nodes match stored signature", currentCount);
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

  // Save all cache entries to a single file
  bool MemcachedGeoFilter::saveCacheToFile(const std::map<boost::uuids::uuid, Node> &nodes) {
    if (!isPersistenceEnabled()) {
      spdlog::warn("Cannot save cache: no persist path configured");
      return false;
    }

    std::lock_guard<std::mutex> lock(localCacheMutex);

    if (localCache.empty()) {
      spdlog::info("No cache entries to save");
      return true;
    }

    try {
      // Ensure directory exists
      namespace fs = boost::filesystem;
      fs::path filePath(persistPath);
      if (filePath.has_parent_path() && !fs::exists(filePath.parent_path())) {
        fs::create_directories(filePath.parent_path());
      }

      std::ofstream file(persistPath, std::ios::binary);
      if (!file.is_open()) {
        spdlog::error("Failed to open cache file for writing: {}", persistPath);
        return false;
      }

      // Write header
      file.write(CACHE_MAGIC, 4);
      file.write(reinterpret_cast<const char*>(&CACHE_VERSION), sizeof(CACHE_VERSION));

      // Write nodes metadata for validation on reload
      uint32_t nodesCount = static_cast<uint32_t>(nodes.size());
      uint64_t nodesHash = computeNodesHash(nodes);
      file.write(reinterpret_cast<const char*>(&nodesCount), sizeof(nodesCount));
      file.write(reinterpret_cast<const char*>(&nodesHash), sizeof(nodesHash));

      // Write entry count
      uint32_t entryCount = static_cast<uint32_t>(localCache.size());
      file.write(reinterpret_cast<const char*>(&entryCount), sizeof(entryCount));

      // Write each entry
      for (const auto& entry : localCache) {
        // Write key
        uint32_t keyLen = static_cast<uint32_t>(entry.first.length());
        file.write(reinterpret_cast<const char*>(&keyLen), sizeof(keyLen));
        file.write(entry.first.c_str(), keyLen);

        // Write data
        uint32_t dataLen = static_cast<uint32_t>(entry.second.length());
        file.write(reinterpret_cast<const char*>(&dataLen), sizeof(dataLen));
        file.write(entry.second.c_str(), dataLen);
      }

      file.close();
      spdlog::info("Saved {} cache entries to {} (nodes: {}, hash: {:016x})",
                   entryCount, persistPath, nodesCount, nodesHash);
      return true;

    } catch (const std::exception& e) {
      spdlog::error("Error saving cache to file: {}", e.what());
      return false;
    }
  }

  // Load cache entries from single file and populate memcached
  // Note: Actual validation against nodes happens on first getAccessibleNodesFootpathsFromPoint call
  bool MemcachedGeoFilter::loadCacheFromFile() {
    if (!isPersistenceEnabled()) {
      return false;
    }

    namespace fs = boost::filesystem;
    if (!fs::exists(persistPath)) {
      spdlog::info("No cache file found at {}, starting with empty cache", persistPath);
      return true;
    }

    try {
      std::ifstream file(persistPath, std::ios::binary);
      if (!file.is_open()) {
        spdlog::warn("Failed to open cache file: {}", persistPath);
        return false;
      }

      // Read and verify header
      char magic[4];
      file.read(magic, 4);
      if (std::memcmp(magic, CACHE_MAGIC, 4) != 0) {
        spdlog::warn("Invalid cache file format (bad magic)");
        return false;
      }

      uint32_t version;
      file.read(reinterpret_cast<char*>(&version), sizeof(version));
      if (version != CACHE_VERSION) {
        spdlog::warn("Cache file version mismatch (expected {}, got {}). Cache will be regenerated.",
                     CACHE_VERSION, version);
        return false;
      }

      // Read nodes metadata for validation
      file.read(reinterpret_cast<char*>(&loadedNodesCount), sizeof(loadedNodesCount));
      file.read(reinterpret_cast<char*>(&loadedNodesHash), sizeof(loadedNodesHash));
      spdlog::debug("Cache file nodes metadata: count={}, hash={:016x}", loadedNodesCount, loadedNodesHash);

      // Read entry count
      uint32_t entryCount;
      file.read(reinterpret_cast<char*>(&entryCount), sizeof(entryCount));

      // Get memcached connection
      memcached_return_t rc;
      memcached_st* memc = memcached_pool_fetch(memcPool, NULL, &rc);
      if (memc == nullptr) {
        spdlog::warn("Cannot load cache: memcached connection unavailable");
        return false;
      }

      int loadedCount = 0;
      int errorCount = 0;

      // Read each entry
      for (uint32_t i = 0; i < entryCount; i++) {
        // Read key
        uint32_t keyLen;
        file.read(reinterpret_cast<char*>(&keyLen), sizeof(keyLen));
        std::string cacheKey(keyLen, '\0');
        file.read(&cacheKey[0], keyLen);

        // Read data
        uint32_t dataLen;
        file.read(reinterpret_cast<char*>(&dataLen), sizeof(dataLen));
        std::string data(dataLen, '\0');
        file.read(&data[0], dataLen);

        // Store in memcached
        rc = memcached_set(
          memc,
          cacheKey.c_str(),
          cacheKey.length(),
          data.c_str(),
          data.length(),
          cacheExpirySeconds,
          0
        );

        if (rc == MEMCACHED_SUCCESS) {
          // Also store in local cache
          {
            std::lock_guard<std::mutex> lock(localCacheMutex);
            localCache[cacheKey] = data;
          }
          loadedCount++;
        } else {
          errorCount++;
        }
      }

      // Return connection to pool
      memcached_pool_release(memcPool, memc);
      file.close();

      spdlog::info("Loaded {} cache entries from {} ({} errors). Validation pending.",
                   loadedCount, persistPath, errorCount);
      return true;

    } catch (const std::exception& e) {
      spdlog::error("Error loading cache from file: {}", e.what());
      return false;
    }
  }

  // Get number of cached entries
  size_t MemcachedGeoFilter::getCacheSize() {
    std::lock_guard<std::mutex> lock(localCacheMutex);
    return localCache.size();
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
    // Validate cache on first call (when we have access to nodes)
    if (!cacheValidated) {
      validateCacheWithNodes(nodes);
    }

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

        if (rc == MEMCACHED_SUCCESS) {
          // Also store in local cache for persistence
          {
            std::lock_guard<std::mutex> lock(localCacheMutex);
            localCache[cacheKey] = serializedResults;
          }
        } else {
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

    // Flush memcached
    memcached_return_t rc = memcached_flush(mcConnectionGuard.get(), 0);
    bool success = (rc == MEMCACHED_SUCCESS);

    if (!success) {
      spdlog::error("Failed to flush cache: {}", memcached_strerror(mcConnectionGuard.get(), rc));
    }

    // Also clear local cache
    {
      std::lock_guard<std::mutex> localLock(localCacheMutex);
      localCache.clear();
    }

    return success;
  }

  // Reset cache completely: flush memcached, clear local cache, and delete cache file
  bool MemcachedGeoFilter::resetCache() {
    // First flush memcached and clear local cache
    bool flushSuccess = flushCache();

    // Delete the cache file if it exists
    bool fileDeleted = false;
    if (isPersistenceEnabled()) {
      namespace fs = boost::filesystem;
      if (fs::exists(persistPath)) {
        try {
          fs::remove(persistPath);
          fileDeleted = true;
          spdlog::info("Deleted cache file: {}", persistPath);
        } catch (const std::exception& e) {
          spdlog::error("Failed to delete cache file {}: {}", persistPath, e.what());
        }
      } else {
        fileDeleted = true; // File doesn't exist, consider it "deleted"
      }
    } else {
      fileDeleted = true; // No persist path configured
    }

    // Reset validation state so next query will re-validate
    {
      std::lock_guard<std::mutex> lock(validationMutex);
      cacheValidated = false;
      loadedNodesCount = 0;
      loadedNodesHash = 0;
    }

    return flushSuccess && fileDeleted;
  }

} // namespace TrRouting
