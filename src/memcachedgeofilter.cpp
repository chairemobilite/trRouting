#include "memcachedgeofilter.hpp"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include "point.hpp"
#include "node.hpp"
#include "spdlog/spdlog.h"

namespace TrRouting {

  // MemcachedGeoFilter constructor
  MemcachedGeoFilter::MemcachedGeoFilter(
                                         GeoFilter* baseGeoFilter,
                                         const std::string& memcachedServers,
                                         uint32_t cacheExpiry
                                         ) : baseGeoFilter(baseGeoFilter), cacheExpirySeconds(cacheExpiry)
  {
    // Initialize memcached
    memc = memcached_create(NULL);
    
    // Parse server list
    memcached_server_st *servers = NULL;
    servers = memcached_servers_parse(memcachedServers.c_str());
    
    // Add servers to the memcached handle
    memcached_server_push(memc, servers);
    memcached_server_list_free(servers);
    
    // Configure behaviors for optimal performance
    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NO_BLOCK, 1);
    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_TCP_NODELAY, 1);
  }

  // Destructor
  MemcachedGeoFilter::~MemcachedGeoFilter() {
    memcached_free(memc);
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

  // Implementation of the main interface method
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
    
    // Try to get from cache
    memcached_return rc;
    size_t valueLength;
    uint32_t flags;
    
    char *cachedResult = memcached_get(
                                       memc, 
                                       cacheKey.c_str(), 
                                       cacheKey.length(), 
                                       &valueLength, 
                                       &flags, 
                                       &rc
                                       );
    
    if (rc == MEMCACHED_SUCCESS && cachedResult != NULL) {
      spdlog::debug("Memcached cache hit ({}) - {}", cacheKey, valueLength);
      // Cache hit
      std::string resultData(cachedResult, valueLength);
      free(cachedResult);
      
      try {
        // Deserialize the cached result
        auto serializableResults = deserializeResults(resultData);
        
        // Convert back to NodeTimeDistance objects with proper references
        return convertFromSerializable(serializableResults, nodes);
      } catch (const std::exception& e) {
        // If deserialization fails, log error and fall through to compute result
        spdlog::warn("Cache deserialization error: {}", e.what());
      }
    }
    
    // Cache miss or error, compute the result using the base provider
    std::vector<NodeTimeDistance> results = baseGeoFilter->getAccessibleNodesFootpathsFromPoint(
      point, nodes, maxWalkingTravelTime, walkingSpeedMetersPerSecond, reversed);
    
    // Convert to serializable format and serialize
    auto serializableResults = convertToSerializable(results);
    std::string serializedResults = serializeResults(serializableResults);
    
    if (!serializedResults.empty()) {
      // Store in cache
      rc = memcached_set(
                         memc,
                         cacheKey.c_str(),
                         cacheKey.length(),
                         serializedResults.c_str(),
                         serializedResults.length(),
                         cacheExpirySeconds,
                         (uint32_t)0
                         );

      if (rc != MEMCACHED_SUCCESS) {
        spdlog::warn("Failed to store footpaths in cache: {}", memcached_strerror(memc, rc));
      }
    }
    
    return results;
  }

  // Flush the cache
  bool MemcachedGeoFilter::flushCache() {
    memcached_return rc = memcached_flush(memc, 0);
    return (rc == MEMCACHED_SUCCESS);
  }
  
} // namespace TrRouting
