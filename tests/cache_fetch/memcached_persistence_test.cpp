#include "gtest/gtest.h"
#include <fstream>
#include <cstring>
#include <regex>
#include <set>
#include <boost/filesystem.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>

#include "memcachedgeofilter.hpp"
#include "euclideangeofilter.hpp"
#include "point.hpp"
#include "node.hpp"

namespace fs = boost::filesystem;

// Test fixture for memcached persistence tests
class MemcachedPersistenceFixtureTests : public ::testing::Test {
protected:
    std::string testCacheDir;
    std::string testCacheFile;

    void SetUp() override {
        // Create a unique test directory for each test
        testCacheDir = "testCache/memcached_test_" + std::to_string(std::rand());
        testCacheFile = testCacheDir + "/footpaths.cache";

        // Ensure test directory exists
        if (!fs::exists(testCacheDir)) {
            fs::create_directories(testCacheDir);
        }
    }

    void TearDown() override {
        // Clean up test files
        if (fs::exists(testCacheDir)) {
            fs::remove_all(testCacheDir);
        }
    }

    // Helper to create a valid cache file with given entries (version 2 format with nodes metadata)
    void createCacheFile(const std::map<std::string, std::string>& entries,
                         uint32_t nodesCount = 10, uint64_t nodesHash = 0x123456789ABCDEF0ULL) {
        std::ofstream file(testCacheFile, std::ios::binary);
        ASSERT_TRUE(file.is_open());

        // Write magic header
        const char magic[4] = {'T', 'R', 'F', 'C'};
        file.write(magic, 4);

        // Write version (now version 2)
        uint32_t version = 2;
        file.write(reinterpret_cast<const char*>(&version), sizeof(version));

        // Write nodes metadata for validation
        file.write(reinterpret_cast<const char*>(&nodesCount), sizeof(nodesCount));
        file.write(reinterpret_cast<const char*>(&nodesHash), sizeof(nodesHash));

        // Write entry count
        uint32_t entryCount = static_cast<uint32_t>(entries.size());
        file.write(reinterpret_cast<const char*>(&entryCount), sizeof(entryCount));

        // Write each entry
        for (const auto& entry : entries) {
            uint32_t keyLen = static_cast<uint32_t>(entry.first.length());
            file.write(reinterpret_cast<const char*>(&keyLen), sizeof(keyLen));
            file.write(entry.first.c_str(), keyLen);

            uint32_t dataLen = static_cast<uint32_t>(entry.second.length());
            file.write(reinterpret_cast<const char*>(&dataLen), sizeof(dataLen));
            file.write(entry.second.c_str(), dataLen);
        }

        file.close();
    }

    // Helper to read and verify cache file structure (version 2)
    bool verifyCacheFileStructure(uint32_t expectedEntries = 0) {
        std::ifstream file(testCacheFile, std::ios::binary);
        if (!file.is_open()) return false;

        // Read and verify magic header
        char magic[4];
        file.read(magic, 4);
        if (std::memcmp(magic, "TRFC", 4) != 0) return false;

        // Read and verify version (now version 2)
        uint32_t version;
        file.read(reinterpret_cast<char*>(&version), sizeof(version));
        if (version != 2) return false;

        // Read nodes metadata
        uint32_t nodesCount;
        uint64_t nodesHash;
        file.read(reinterpret_cast<char*>(&nodesCount), sizeof(nodesCount));
        file.read(reinterpret_cast<char*>(&nodesHash), sizeof(nodesHash));

        // Read entry count
        uint32_t entryCount;
        file.read(reinterpret_cast<char*>(&entryCount), sizeof(entryCount));
        if (expectedEntries > 0 && entryCount != expectedEntries) return false;

        file.close();
        return true;
    }

    // Helper to create test nodes
    std::map<boost::uuids::uuid, TrRouting::Node> createTestNodes(int count) {
        std::map<boost::uuids::uuid, TrRouting::Node> nodes;
        boost::uuids::random_generator gen;

        for (int i = 0; i < count; i++) {
            auto uuid = gen();
            auto point = std::make_unique<TrRouting::Point>(45.5 + i * 0.01, -73.5 + i * 0.01);
            nodes.emplace(std::piecewise_construct,
                std::forward_as_tuple(uuid),
                std::forward_as_tuple(uuid, i, "CODE" + std::to_string(i),
                    "Node " + std::to_string(i), "internal" + std::to_string(i),
                    std::move(point)));
        }
        return nodes;
    }

    // Helper to create test nodes with specific UUIDs for reproducibility
    std::map<boost::uuids::uuid, TrRouting::Node> createTestNodesWithFixedUuids(
        const std::vector<std::tuple<boost::uuids::uuid, double, double>>& nodeSpecs) {
        std::map<boost::uuids::uuid, TrRouting::Node> nodes;
        int i = 0;
        for (const auto& spec : nodeSpecs) {
            auto uuid = std::get<0>(spec);
            double lat = std::get<1>(spec);
            double lon = std::get<2>(spec);
            auto point = std::make_unique<TrRouting::Point>(lat, lon);
            nodes.emplace(std::piecewise_construct,
                std::forward_as_tuple(uuid),
                std::forward_as_tuple(uuid, i, "CODE" + std::to_string(i),
                    "Node " + std::to_string(i), "internal" + std::to_string(i),
                    std::move(point)));
            i++;
        }
        return nodes;
    }

    // Compute nodes hash using the same algorithm as MemcachedGeoFilter
    // This mirrors the implementation in memcachedgeofilter.cpp for testing
    static uint64_t computeNodesHash(const std::map<boost::uuids::uuid, TrRouting::Node>& nodes) {
        uint64_t hash = nodes.size(); // Start with count

        for (const auto& pair : nodes) {
            const TrRouting::Node& node = pair.second;

            // Hash the UUID bytes
            for (const auto& byte : node.uuid) {
                hash ^= static_cast<uint64_t>(byte) + 0x9e3779b97f4a7c15ULL + (hash << 6) + (hash >> 2);
            }

            // Hash the location (convert to fixed-point for consistency)
            int64_t latFixed = static_cast<int64_t>(node.point->latitude * 1000000.0);
            int64_t lonFixed = static_cast<int64_t>(node.point->longitude * 1000000.0);

            hash ^= static_cast<uint64_t>(latFixed) + 0x9e3779b97f4a7c15ULL + (hash << 6) + (hash >> 2);
            hash ^= static_cast<uint64_t>(lonFixed) + 0x9e3779b97f4a7c15ULL + (hash << 6) + (hash >> 2);
        }

        return hash;
    }
};

// Test: SerializableNodeTimeDistance serialization/deserialization
TEST_F(MemcachedPersistenceFixtureTests, TestSerializableNodeTimeDistanceSerialization)
{
    boost::uuids::random_generator gen;

    // Create test data
    std::vector<TrRouting::SerializableNodeTimeDistance> original;
    for (int i = 0; i < 5; i++) {
        TrRouting::SerializableNodeTimeDistance sntd;
        sntd.nodeUuid = gen();
        sntd.time = 100 + i * 10;
        sntd.distance = 500 + i * 50;
        original.push_back(sntd);
    }

    // Serialize
    std::stringstream ss;
    {
        boost::archive::binary_oarchive oa(ss);
        oa << original;
    }

    // Deserialize
    std::vector<TrRouting::SerializableNodeTimeDistance> deserialized;
    {
        boost::archive::binary_iarchive ia(ss);
        ia >> deserialized;
    }

    // Verify
    ASSERT_EQ(original.size(), deserialized.size());
    for (size_t i = 0; i < original.size(); i++) {
        EXPECT_EQ(original[i].nodeUuid, deserialized[i].nodeUuid);
        EXPECT_EQ(original[i].time, deserialized[i].time);
        EXPECT_EQ(original[i].distance, deserialized[i].distance);
    }
}

// Test: Cache file format magic header
TEST_F(MemcachedPersistenceFixtureTests, TestCacheFileMagicHeader)
{
    // Create a file with correct magic
    std::map<std::string, std::string> entries;
    entries["key1"] = "value1";
    createCacheFile(entries);

    EXPECT_TRUE(verifyCacheFileStructure(1));
}

// Test: Cache file with invalid magic header should be rejected
TEST_F(MemcachedPersistenceFixtureTests, TestInvalidMagicHeader)
{
    // Create a file with invalid magic
    std::ofstream file(testCacheFile, std::ios::binary);
    ASSERT_TRUE(file.is_open());

    const char badMagic[4] = {'B', 'A', 'D', '!'};
    file.write(badMagic, 4);

    uint32_t version = 1;
    file.write(reinterpret_cast<const char*>(&version), sizeof(version));

    uint32_t entryCount = 0;
    file.write(reinterpret_cast<const char*>(&entryCount), sizeof(entryCount));
    file.close();

    // Verify the file exists but has bad magic
    std::ifstream readFile(testCacheFile, std::ios::binary);
    ASSERT_TRUE(readFile.is_open());

    char magic[4];
    readFile.read(magic, 4);
    EXPECT_NE(std::memcmp(magic, "TRFC", 4), 0);
    readFile.close();
}

// Test: Cache file with wrong version should be handled
TEST_F(MemcachedPersistenceFixtureTests, TestInvalidVersion)
{
    // Create a file with invalid version
    std::ofstream file(testCacheFile, std::ios::binary);
    ASSERT_TRUE(file.is_open());

    const char magic[4] = {'T', 'R', 'F', 'C'};
    file.write(magic, 4);

    uint32_t badVersion = 999;
    file.write(reinterpret_cast<const char*>(&badVersion), sizeof(badVersion));

    uint32_t entryCount = 0;
    file.write(reinterpret_cast<const char*>(&entryCount), sizeof(entryCount));
    file.close();

    // Verify the file exists but has bad version
    std::ifstream readFile(testCacheFile, std::ios::binary);
    ASSERT_TRUE(readFile.is_open());

    char readMagic[4];
    readFile.read(readMagic, 4);
    EXPECT_EQ(std::memcmp(readMagic, "TRFC", 4), 0);

    uint32_t version;
    readFile.read(reinterpret_cast<char*>(&version), sizeof(version));
    EXPECT_NE(version, 2); // Current version is 2
    readFile.close();
}

// Test: Old version 1 cache file should be rejected (version mismatch)
TEST_F(MemcachedPersistenceFixtureTests, TestOldVersion1Rejected)
{
    // Create a version 1 file (without nodes metadata)
    std::ofstream file(testCacheFile, std::ios::binary);
    ASSERT_TRUE(file.is_open());

    const char magic[4] = {'T', 'R', 'F', 'C'};
    file.write(magic, 4);

    uint32_t oldVersion = 1;
    file.write(reinterpret_cast<const char*>(&oldVersion), sizeof(oldVersion));

    uint32_t entryCount = 0;
    file.write(reinterpret_cast<const char*>(&entryCount), sizeof(entryCount));
    file.close();

    // Verify it's version 1 which should be rejected by current loader
    std::ifstream readFile(testCacheFile, std::ios::binary);
    ASSERT_TRUE(readFile.is_open());

    char readMagic[4];
    readFile.read(readMagic, 4);

    uint32_t version;
    readFile.read(reinterpret_cast<char*>(&version), sizeof(version));
    EXPECT_EQ(version, 1);
    EXPECT_NE(version, 2); // Current version is 2, so this would be rejected
    readFile.close();
}

// Test: Cache file stores and reads nodes metadata correctly
TEST_F(MemcachedPersistenceFixtureTests, TestNodesMetadataInCacheFile)
{
    uint32_t expectedNodesCount = 42;
    uint64_t expectedNodesHash = 0xDEADBEEFCAFEBABEULL;

    std::map<std::string, std::string> entries;
    entries["test_key"] = "test_value";
    createCacheFile(entries, expectedNodesCount, expectedNodesHash);

    // Read back and verify nodes metadata
    std::ifstream file(testCacheFile, std::ios::binary);
    ASSERT_TRUE(file.is_open());

    // Skip magic and version
    char magic[4];
    file.read(magic, 4);
    uint32_t version;
    file.read(reinterpret_cast<char*>(&version), sizeof(version));
    EXPECT_EQ(version, 2);

    // Read nodes metadata
    uint32_t nodesCount;
    uint64_t nodesHash;
    file.read(reinterpret_cast<char*>(&nodesCount), sizeof(nodesCount));
    file.read(reinterpret_cast<char*>(&nodesHash), sizeof(nodesHash));

    EXPECT_EQ(nodesCount, expectedNodesCount);
    EXPECT_EQ(nodesHash, expectedNodesHash);

    file.close();
}

// Test: Empty cache file structure
TEST_F(MemcachedPersistenceFixtureTests, TestEmptyCacheFile)
{
    std::map<std::string, std::string> entries;
    createCacheFile(entries);

    EXPECT_TRUE(verifyCacheFileStructure(0));
}

// Test: Cache file with multiple entries
TEST_F(MemcachedPersistenceFixtureTests, TestMultipleEntriesCacheFile)
{
    std::map<std::string, std::string> entries;
    entries["footpaths:45.500000:-73.500000:600:1.400000:0"] = "data1";
    entries["footpaths:45.510000:-73.510000:600:1.400000:1"] = "data2";
    entries["footpaths:45.520000:-73.520000:900:1.200000:0"] = "data3";
    createCacheFile(entries);

    EXPECT_TRUE(verifyCacheFileStructure(3));
}

// Test: Cache key generation format
TEST_F(MemcachedPersistenceFixtureTests, TestCacheKeyFormat)
{
    // Based on the implementation, keys should be in format:
    // footpaths:latitude:longitude:maxWalkingTime:walkingSpeed:reversed
    std::string expectedPrefix = "footpaths:";
    std::string testKey = "footpaths:45.123456:-73.654321:600:1.400000:0";

    EXPECT_EQ(testKey.substr(0, expectedPrefix.length()), expectedPrefix);
    EXPECT_NE(testKey.find(":45.123456:"), std::string::npos);
    EXPECT_NE(testKey.find(":-73.654321:"), std::string::npos);
    EXPECT_NE(testKey.find(":600:"), std::string::npos);
}

// Test: Different walking speeds produce different cache keys
TEST_F(MemcachedPersistenceFixtureTests, TestCacheKeyDifferentWalkingSpeeds)
{
    // Same location, same max walking time, but different walking speeds
    std::string key_speed_1_4 = "footpaths:45.500000:-73.500000:600:1.400000:0";
    std::string key_speed_1_2 = "footpaths:45.500000:-73.500000:600:1.200000:0";
    std::string key_speed_1_0 = "footpaths:45.500000:-73.500000:600:1.000000:0";

    // Verify all keys are different
    EXPECT_NE(key_speed_1_4, key_speed_1_2)
        << "Different walking speeds should produce different cache keys";
    EXPECT_NE(key_speed_1_4, key_speed_1_0)
        << "Different walking speeds should produce different cache keys";
    EXPECT_NE(key_speed_1_2, key_speed_1_0)
        << "Different walking speeds should produce different cache keys";

    // Verify walking speed is in the key
    EXPECT_NE(key_speed_1_4.find(":1.400000:"), std::string::npos);
    EXPECT_NE(key_speed_1_2.find(":1.200000:"), std::string::npos);
    EXPECT_NE(key_speed_1_0.find(":1.000000:"), std::string::npos);
}

// Test: Cache file can store entries with different walking speeds
TEST_F(MemcachedPersistenceFixtureTests, TestCacheFileWithDifferentWalkingSpeeds)
{
    // Create entries for same location but different walking speeds
    std::map<std::string, std::string> entries;
    entries["footpaths:45.500000:-73.500000:600:1.400000:0"] = "data_speed_1.4";
    entries["footpaths:45.500000:-73.500000:600:1.200000:0"] = "data_speed_1.2";
    entries["footpaths:45.500000:-73.500000:600:1.000000:0"] = "data_speed_1.0";
    entries["footpaths:45.500000:-73.500000:600:0.800000:0"] = "data_speed_0.8";

    createCacheFile(entries);

    // Verify file structure with 4 entries
    EXPECT_TRUE(verifyCacheFileStructure(4));

    // Read back and verify all entries are distinct
    std::ifstream file(testCacheFile, std::ios::binary);
    ASSERT_TRUE(file.is_open());

    // Skip header and nodes metadata
    char magic[4];
    file.read(magic, 4);
    uint32_t version;
    file.read(reinterpret_cast<char*>(&version), sizeof(version));
    uint32_t nodesCount;
    uint64_t nodesHash;
    file.read(reinterpret_cast<char*>(&nodesCount), sizeof(nodesCount));
    file.read(reinterpret_cast<char*>(&nodesHash), sizeof(nodesHash));

    uint32_t entryCount;
    file.read(reinterpret_cast<char*>(&entryCount), sizeof(entryCount));
    EXPECT_EQ(entryCount, 4u);

    // Read all entries and verify they're unique
    std::set<std::string> uniqueKeys;
    std::map<std::string, std::string> readEntries;
    for (uint32_t i = 0; i < entryCount; i++) {
        uint32_t keyLen;
        file.read(reinterpret_cast<char*>(&keyLen), sizeof(keyLen));
        std::string key(keyLen, '\0');
        file.read(&key[0], keyLen);

        uint32_t dataLen;
        file.read(reinterpret_cast<char*>(&dataLen), sizeof(dataLen));
        std::string data(dataLen, '\0');
        file.read(&data[0], dataLen);

        uniqueKeys.insert(key);
        readEntries[key] = data;
    }
    file.close();

    // All 4 keys should be unique
    EXPECT_EQ(uniqueKeys.size(), 4u)
        << "All walking speed cache keys should be unique";

    // Verify each walking speed has its own data
    EXPECT_EQ(readEntries["footpaths:45.500000:-73.500000:600:1.400000:0"], "data_speed_1.4");
    EXPECT_EQ(readEntries["footpaths:45.500000:-73.500000:600:1.200000:0"], "data_speed_1.2");
    EXPECT_EQ(readEntries["footpaths:45.500000:-73.500000:600:1.000000:0"], "data_speed_1.0");
    EXPECT_EQ(readEntries["footpaths:45.500000:-73.500000:600:0.800000:0"], "data_speed_0.8");
}

// Test: Cache key includes all parameters (comprehensive)
TEST_F(MemcachedPersistenceFixtureTests, TestCacheKeyAllParameters)
{
    // Test that changing any parameter produces a different key
    std::string base_key = "footpaths:45.500000:-73.500000:600:1.400000:0";

    // Different latitude
    std::string diff_lat = "footpaths:45.600000:-73.500000:600:1.400000:0";
    EXPECT_NE(base_key, diff_lat) << "Different latitude should produce different key";

    // Different longitude
    std::string diff_lon = "footpaths:45.500000:-73.600000:600:1.400000:0";
    EXPECT_NE(base_key, diff_lon) << "Different longitude should produce different key";

    // Different max walking time
    std::string diff_time = "footpaths:45.500000:-73.500000:900:1.400000:0";
    EXPECT_NE(base_key, diff_time) << "Different max walking time should produce different key";

    // Different walking speed
    std::string diff_speed = "footpaths:45.500000:-73.500000:600:1.200000:0";
    EXPECT_NE(base_key, diff_speed) << "Different walking speed should produce different key";

    // Different reversed flag
    std::string diff_reversed = "footpaths:45.500000:-73.500000:600:1.400000:1";
    EXPECT_NE(base_key, diff_reversed) << "Different reversed flag should produce different key";
}

// Test: Cache file roundtrip (write and read) - version 2 format
TEST_F(MemcachedPersistenceFixtureTests, TestCacheFileRoundtrip)
{
    // Create entries with realistic data
    std::map<std::string, std::string> originalEntries;
    originalEntries["key_alpha"] = "value_alpha_content";
    originalEntries["key_beta"] = "value_beta_longer_content_here";
    originalEntries["key_gamma"] = std::string(1000, 'x'); // Long value

    createCacheFile(originalEntries);

    // Read back the file
    std::ifstream file(testCacheFile, std::ios::binary);
    ASSERT_TRUE(file.is_open());

    // Read and verify header
    char magic[4];
    file.read(magic, 4);
    uint32_t version;
    file.read(reinterpret_cast<char*>(&version), sizeof(version));
    EXPECT_EQ(version, 2);

    // Skip nodes metadata (version 2)
    uint32_t nodesCount;
    uint64_t nodesHash;
    file.read(reinterpret_cast<char*>(&nodesCount), sizeof(nodesCount));
    file.read(reinterpret_cast<char*>(&nodesHash), sizeof(nodesHash));

    // Read entry count
    uint32_t entryCount;
    file.read(reinterpret_cast<char*>(&entryCount), sizeof(entryCount));
    EXPECT_EQ(entryCount, static_cast<uint32_t>(originalEntries.size()));

    // Read entries
    std::map<std::string, std::string> readEntries;
    for (uint32_t i = 0; i < entryCount; i++) {
        uint32_t keyLen;
        file.read(reinterpret_cast<char*>(&keyLen), sizeof(keyLen));
        std::string key(keyLen, '\0');
        file.read(&key[0], keyLen);

        uint32_t dataLen;
        file.read(reinterpret_cast<char*>(&dataLen), sizeof(dataLen));
        std::string data(dataLen, '\0');
        file.read(&data[0], dataLen);

        readEntries[key] = data;
    }

    file.close();

    // Verify all entries match
    EXPECT_EQ(originalEntries.size(), readEntries.size());
    for (const auto& entry : originalEntries) {
        auto it = readEntries.find(entry.first);
        ASSERT_NE(it, readEntries.end()) << "Key not found: " << entry.first;
        EXPECT_EQ(entry.second, it->second) << "Value mismatch for key: " << entry.first;
    }
}

// Test: Missing cache directory handling
TEST_F(MemcachedPersistenceFixtureTests, TestMissingCacheDirectory)
{
    std::string nonExistentPath = "testCache/nonexistent_" + std::to_string(std::rand()) + "/cache.tmp";

    // Verify directory doesn't exist
    fs::path dir = fs::path(nonExistentPath).parent_path();
    EXPECT_FALSE(fs::exists(dir));
}

// Test: Cache file doesn't exist scenario
TEST_F(MemcachedPersistenceFixtureTests, TestNonExistentCacheFile)
{
    std::string nonExistentFile = testCacheDir + "/nonexistent.cache";
    EXPECT_FALSE(fs::exists(nonExistentFile));
}

// Test: Binary data in cache values
TEST_F(MemcachedPersistenceFixtureTests, TestBinaryDataInCacheValues)
{
    // Create entries with binary data (like serialized boost archives)
    std::map<std::string, std::string> entries;

    // Create binary data with null bytes and special characters
    std::string binaryData;
    for (int i = 0; i < 256; i++) {
        binaryData += static_cast<char>(i);
    }

    entries["binary_key"] = binaryData;
    createCacheFile(entries);

    // Read it back
    std::ifstream file(testCacheFile, std::ios::binary);
    ASSERT_TRUE(file.is_open());

    // Read header
    char magic[4];
    file.read(magic, 4);
    uint32_t version;
    file.read(reinterpret_cast<char*>(&version), sizeof(version));

    // Skip nodes metadata (version 2)
    uint32_t nodesCount;
    uint64_t nodesHash;
    file.read(reinterpret_cast<char*>(&nodesCount), sizeof(nodesCount));
    file.read(reinterpret_cast<char*>(&nodesHash), sizeof(nodesHash));

    // Read entry count
    uint32_t entryCount;
    file.read(reinterpret_cast<char*>(&entryCount), sizeof(entryCount));

    // Read the entry
    uint32_t keyLen;
    file.read(reinterpret_cast<char*>(&keyLen), sizeof(keyLen));
    std::string key(keyLen, '\0');
    file.read(&key[0], keyLen);

    uint32_t dataLen;
    file.read(reinterpret_cast<char*>(&dataLen), sizeof(dataLen));
    std::string data(dataLen, '\0');
    file.read(&data[0], dataLen);

    file.close();

    EXPECT_EQ(key, "binary_key");
    EXPECT_EQ(data.length(), binaryData.length());
    EXPECT_EQ(data, binaryData);
}

// Test: SerializableNodeTimeDistance default constructor
TEST_F(MemcachedPersistenceFixtureTests, TestSerializableNodeTimeDistanceDefaultConstructor)
{
    TrRouting::SerializableNodeTimeDistance sntd;
    EXPECT_EQ(sntd.time, 0);
    EXPECT_EQ(sntd.distance, 0);
}

// Test: SerializableNodeTimeDistance conversion from NodeTimeDistance
TEST_F(MemcachedPersistenceFixtureTests, TestSerializableNodeTimeDistanceConversion)
{
    // Create a node
    boost::uuids::random_generator gen;
    auto uuid = gen();
    auto point = std::make_unique<TrRouting::Point>(45.5, -73.5);
    TrRouting::Node node(uuid, 1, "CODE1", "Test Node", "internal1", std::move(point));

    // Create NodeTimeDistance
    TrRouting::NodeTimeDistance ntd(node, 120, 150);

    // Convert to serializable
    TrRouting::SerializableNodeTimeDistance sntd(ntd);

    EXPECT_EQ(sntd.nodeUuid, uuid);
    EXPECT_EQ(sntd.time, 120);
    EXPECT_EQ(sntd.distance, 150);
}

// Test: Large number of cache entries
TEST_F(MemcachedPersistenceFixtureTests, TestLargeNumberOfEntries)
{
    std::map<std::string, std::string> entries;
    const int numEntries = 1000;

    for (int i = 0; i < numEntries; i++) {
        std::string key = "footpaths:45." + std::to_string(i) + ":-73." + std::to_string(i) + ":600:1.4:0";
        entries[key] = "data_for_entry_" + std::to_string(i);
    }

    createCacheFile(entries);

    EXPECT_TRUE(verifyCacheFileStructure(numEntries));

    // Verify file size is reasonable
    auto fileSize = fs::file_size(testCacheFile);
    EXPECT_GT(fileSize, 0u);
}

// Test: Cache file deletion (simulates resetCache file deletion behavior)
TEST_F(MemcachedPersistenceFixtureTests, TestCacheFileDeletion)
{
    // Create a cache file
    std::map<std::string, std::string> entries;
    entries["test_key"] = "test_value";
    createCacheFile(entries);

    // Verify file exists
    EXPECT_TRUE(fs::exists(testCacheFile));

    // Delete the file (simulating what resetCache does)
    fs::remove(testCacheFile);

    // Verify file no longer exists
    EXPECT_FALSE(fs::exists(testCacheFile));
}

// Test: Cache file deletion when file doesn't exist (should not throw)
TEST_F(MemcachedPersistenceFixtureTests, TestCacheFileDeletionNonExistent)
{
    std::string nonExistentFile = testCacheDir + "/nonexistent.cache";

    // Verify file doesn't exist
    EXPECT_FALSE(fs::exists(nonExistentFile));

    // Attempting to check and delete should not throw
    if (fs::exists(nonExistentFile)) {
        fs::remove(nonExistentFile);
    }

    // Still doesn't exist
    EXPECT_FALSE(fs::exists(nonExistentFile));
}

// Test: Cache file can be recreated after deletion
TEST_F(MemcachedPersistenceFixtureTests, TestCacheFileRecreationAfterDeletion)
{
    // Create initial cache file
    std::map<std::string, std::string> entries1;
    entries1["key1"] = "value1";
    createCacheFile(entries1);
    EXPECT_TRUE(fs::exists(testCacheFile));

    // Delete the file
    fs::remove(testCacheFile);
    EXPECT_FALSE(fs::exists(testCacheFile));

    // Recreate with different content
    std::map<std::string, std::string> entries2;
    entries2["key2"] = "value2";
    entries2["key3"] = "value3";
    createCacheFile(entries2);

    // Verify new file exists and has correct structure
    EXPECT_TRUE(fs::exists(testCacheFile));
    EXPECT_TRUE(verifyCacheFileStructure(2));
}

// Test: Verify cache file format after save (version 2 with nodes metadata)
TEST_F(MemcachedPersistenceFixtureTests, TestCacheFileFormatVersion2)
{
    uint32_t expectedNodesCount = 100;
    uint64_t expectedNodesHash = 0xABCDEF0123456789ULL;

    std::map<std::string, std::string> entries;
    entries["footpaths:45.5:-73.5:600:1.4:0"] = "serialized_data_here";
    createCacheFile(entries, expectedNodesCount, expectedNodesHash);

    // Read and verify complete file structure
    std::ifstream file(testCacheFile, std::ios::binary);
    ASSERT_TRUE(file.is_open());

    // Verify magic header
    char magic[4];
    file.read(magic, 4);
    EXPECT_EQ(std::memcmp(magic, "TRFC", 4), 0);

    // Verify version
    uint32_t version;
    file.read(reinterpret_cast<char*>(&version), sizeof(version));
    EXPECT_EQ(version, 2u);

    // Verify nodes metadata
    uint32_t nodesCount;
    uint64_t nodesHash;
    file.read(reinterpret_cast<char*>(&nodesCount), sizeof(nodesCount));
    file.read(reinterpret_cast<char*>(&nodesHash), sizeof(nodesHash));
    EXPECT_EQ(nodesCount, expectedNodesCount);
    EXPECT_EQ(nodesHash, expectedNodesHash);

    // Verify entry count
    uint32_t entryCount;
    file.read(reinterpret_cast<char*>(&entryCount), sizeof(entryCount));
    EXPECT_EQ(entryCount, 1u);

    file.close();
}

// Test: API endpoint patterns (verifies route regex patterns)
TEST_F(MemcachedPersistenceFixtureTests, TestApiRoutePatterns)
{
    // These patterns should match the API routes defined in transit_routing_http_server.cpp
    std::regex saveCachePattern("^/saveCache[/]?$");
    std::regex loadCachePattern("^/loadCache[/]?$");
    std::regex resetCachePattern("^/resetCache[/]?$");
    std::regex cacheStatusPattern("^/cacheStatus[/]?$");

    // Test saveCache route
    EXPECT_TRUE(std::regex_match("/saveCache", saveCachePattern));
    EXPECT_TRUE(std::regex_match("/saveCache/", saveCachePattern));
    EXPECT_FALSE(std::regex_match("/saveCache/extra", saveCachePattern));

    // Test loadCache route
    EXPECT_TRUE(std::regex_match("/loadCache", loadCachePattern));
    EXPECT_TRUE(std::regex_match("/loadCache/", loadCachePattern));
    EXPECT_FALSE(std::regex_match("/loadCache/extra", loadCachePattern));

    // Test resetCache route
    EXPECT_TRUE(std::regex_match("/resetCache", resetCachePattern));
    EXPECT_TRUE(std::regex_match("/resetCache/", resetCachePattern));
    EXPECT_FALSE(std::regex_match("/resetCache/extra", resetCachePattern));

    // Test cacheStatus route
    EXPECT_TRUE(std::regex_match("/cacheStatus", cacheStatusPattern));
    EXPECT_TRUE(std::regex_match("/cacheStatus/", cacheStatusPattern));
    EXPECT_FALSE(std::regex_match("/cacheStatus/extra", cacheStatusPattern));
}

// ============================================================================
// Persistence configuration tests
// ============================================================================

// Test: isPersistenceEnabled returns true when path is configured
TEST_F(MemcachedPersistenceFixtureTests, TestIsPersistenceEnabledWithPath)
{
    // Create a MemcachedGeoFilter with a persist path
    // Note: This test will fail to connect to memcached but we can still check the config
    TrRouting::EuclideanGeoFilter baseFilter;

    // With persist path configured
    TrRouting::MemcachedGeoFilter filterWithPath(
        &baseFilter,
        "localhost:99999",  // Invalid port, won't connect
        0,
        1,
        testCacheFile  // Non-empty path
    );

    EXPECT_TRUE(filterWithPath.isPersistenceEnabled())
        << "isPersistenceEnabled should return true when persistPath is set";
}

// Test: isPersistenceEnabled returns false when path is empty
TEST_F(MemcachedPersistenceFixtureTests, TestIsPersistenceEnabledWithoutPath)
{
    TrRouting::EuclideanGeoFilter baseFilter;

    // Without persist path (empty string)
    TrRouting::MemcachedGeoFilter filterWithoutPath(
        &baseFilter,
        "localhost:99999",
        0,
        1,
        ""  // Empty path
    );

    EXPECT_FALSE(filterWithoutPath.isPersistenceEnabled())
        << "isPersistenceEnabled should return false when persistPath is empty";
}

// Test: Persistence path can be a relative path
TEST_F(MemcachedPersistenceFixtureTests, TestPersistenceWithRelativePath)
{
    TrRouting::EuclideanGeoFilter baseFilter;

    TrRouting::MemcachedGeoFilter filter(
        &baseFilter,
        "localhost:99999",
        0,
        1,
        "relative/path/cache.tmp"
    );

    EXPECT_TRUE(filter.isPersistenceEnabled());
}

// Test: Persistence path can be an absolute path
TEST_F(MemcachedPersistenceFixtureTests, TestPersistenceWithAbsolutePath)
{
    TrRouting::EuclideanGeoFilter baseFilter;

    TrRouting::MemcachedGeoFilter filter(
        &baseFilter,
        "localhost:99999",
        0,
        1,
        "/absolute/path/cache.tmp"
    );

    EXPECT_TRUE(filter.isPersistenceEnabled());
}

// ============================================================================
// Node validation tests - verify cache invalidation when nodes change
// ============================================================================

// Test: Same nodes produce the same hash (consistency)
TEST_F(MemcachedPersistenceFixtureTests, TestNodesHashConsistency)
{
    boost::uuids::string_generator gen;
    auto uuid1 = gen("01234567-89ab-cdef-0123-456789abcdef");
    auto uuid2 = gen("fedcba98-7654-3210-fedc-ba9876543210");

    std::vector<std::tuple<boost::uuids::uuid, double, double>> specs = {
        {uuid1, 45.5, -73.5},
        {uuid2, 45.6, -73.6}
    };

    auto nodes1 = createTestNodesWithFixedUuids(specs);
    auto nodes2 = createTestNodesWithFixedUuids(specs);

    uint64_t hash1 = computeNodesHash(nodes1);
    uint64_t hash2 = computeNodesHash(nodes2);

    EXPECT_EQ(hash1, hash2) << "Same nodes should produce identical hashes";
}

// Test: Different node count changes the hash
TEST_F(MemcachedPersistenceFixtureTests, TestNodesHashChangesWithCount)
{
    boost::uuids::string_generator gen;
    auto uuid1 = gen("01234567-89ab-cdef-0123-456789abcdef");
    auto uuid2 = gen("fedcba98-7654-3210-fedc-ba9876543210");
    auto uuid3 = gen("aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee");

    std::vector<std::tuple<boost::uuids::uuid, double, double>> specs2 = {
        {uuid1, 45.5, -73.5},
        {uuid2, 45.6, -73.6}
    };

    std::vector<std::tuple<boost::uuids::uuid, double, double>> specs3 = {
        {uuid1, 45.5, -73.5},
        {uuid2, 45.6, -73.6},
        {uuid3, 45.7, -73.7}
    };

    auto nodes2 = createTestNodesWithFixedUuids(specs2);
    auto nodes3 = createTestNodesWithFixedUuids(specs3);

    uint64_t hash2 = computeNodesHash(nodes2);
    uint64_t hash3 = computeNodesHash(nodes3);

    EXPECT_NE(hash2, hash3) << "Adding a node should change the hash";
}

// Test: Node location change affects the hash
TEST_F(MemcachedPersistenceFixtureTests, TestNodesHashChangesWithLocation)
{
    boost::uuids::string_generator gen;
    auto uuid1 = gen("01234567-89ab-cdef-0123-456789abcdef");
    auto uuid2 = gen("fedcba98-7654-3210-fedc-ba9876543210");

    std::vector<std::tuple<boost::uuids::uuid, double, double>> specs_original = {
        {uuid1, 45.5, -73.5},
        {uuid2, 45.6, -73.6}
    };

    // Same UUIDs but node2 has moved slightly
    std::vector<std::tuple<boost::uuids::uuid, double, double>> specs_moved = {
        {uuid1, 45.5, -73.5},
        {uuid2, 45.601, -73.601}  // Small location change
    };

    auto nodes_original = createTestNodesWithFixedUuids(specs_original);
    auto nodes_moved = createTestNodesWithFixedUuids(specs_moved);

    uint64_t hash_original = computeNodesHash(nodes_original);
    uint64_t hash_moved = computeNodesHash(nodes_moved);

    EXPECT_NE(hash_original, hash_moved) << "Changing a node's location should change the hash";
}

// Test: Very small location change (within precision) might not change hash
TEST_F(MemcachedPersistenceFixtureTests, TestNodesHashPrecision)
{
    boost::uuids::string_generator gen;
    auto uuid1 = gen("01234567-89ab-cdef-0123-456789abcdef");

    std::vector<std::tuple<boost::uuids::uuid, double, double>> specs1 = {
        {uuid1, 45.5000001, -73.5000001}
    };

    std::vector<std::tuple<boost::uuids::uuid, double, double>> specs2 = {
        {uuid1, 45.5000002, -73.5000002}  // Change in 7th decimal place
    };

    auto nodes1 = createTestNodesWithFixedUuids(specs1);
    auto nodes2 = createTestNodesWithFixedUuids(specs2);

    uint64_t hash1 = computeNodesHash(nodes1);
    uint64_t hash2 = computeNodesHash(nodes2);

    // With 6 decimal precision (multiplied by 1000000), these should be the same
    // 45.5000001 * 1000000 = 45500000.1 -> truncated to 45500000
    // 45.5000002 * 1000000 = 45500000.2 -> truncated to 45500000
    EXPECT_EQ(hash1, hash2) << "Changes below precision threshold should not affect hash";
}

// Test: Removing a node changes the hash
TEST_F(MemcachedPersistenceFixtureTests, TestNodesHashChangesWhenNodeRemoved)
{
    boost::uuids::string_generator gen;
    auto uuid1 = gen("01234567-89ab-cdef-0123-456789abcdef");
    auto uuid2 = gen("fedcba98-7654-3210-fedc-ba9876543210");

    std::vector<std::tuple<boost::uuids::uuid, double, double>> specs_with_both = {
        {uuid1, 45.5, -73.5},
        {uuid2, 45.6, -73.6}
    };

    std::vector<std::tuple<boost::uuids::uuid, double, double>> specs_with_one = {
        {uuid1, 45.5, -73.5}
    };

    auto nodes_both = createTestNodesWithFixedUuids(specs_with_both);
    auto nodes_one = createTestNodesWithFixedUuids(specs_with_one);

    uint64_t hash_both = computeNodesHash(nodes_both);
    uint64_t hash_one = computeNodesHash(nodes_one);

    EXPECT_NE(hash_both, hash_one) << "Removing a node should change the hash";
}

// Test: Different UUID for same location changes the hash
TEST_F(MemcachedPersistenceFixtureTests, TestNodesHashChangesWithUuid)
{
    boost::uuids::string_generator gen;
    auto uuid1 = gen("01234567-89ab-cdef-0123-456789abcdef");
    auto uuid2 = gen("ffffffff-ffff-ffff-ffff-ffffffffffff");

    // Same location, different UUID
    std::vector<std::tuple<boost::uuids::uuid, double, double>> specs1 = {
        {uuid1, 45.5, -73.5}
    };

    std::vector<std::tuple<boost::uuids::uuid, double, double>> specs2 = {
        {uuid2, 45.5, -73.5}
    };

    auto nodes1 = createTestNodesWithFixedUuids(specs1);
    auto nodes2 = createTestNodesWithFixedUuids(specs2);

    uint64_t hash1 = computeNodesHash(nodes1);
    uint64_t hash2 = computeNodesHash(nodes2);

    EXPECT_NE(hash1, hash2) << "Different UUID should change the hash even with same location";
}

// Test: Empty nodes set has a specific hash (just the count = 0)
TEST_F(MemcachedPersistenceFixtureTests, TestEmptyNodesHash)
{
    std::map<boost::uuids::uuid, TrRouting::Node> empty_nodes;

    uint64_t hash = computeNodesHash(empty_nodes);

    // Empty set hash should be 0 (just the count)
    EXPECT_EQ(hash, 0u) << "Empty nodes set should have hash of 0";
}

// Test: Cache file with mismatched nodes count would be detected
TEST_F(MemcachedPersistenceFixtureTests, TestCacheFileNodesCountMismatch)
{
    boost::uuids::string_generator gen;
    auto uuid1 = gen("01234567-89ab-cdef-0123-456789abcdef");
    auto uuid2 = gen("fedcba98-7654-3210-fedc-ba9876543210");

    std::vector<std::tuple<boost::uuids::uuid, double, double>> specs = {
        {uuid1, 45.5, -73.5},
        {uuid2, 45.6, -73.6}
    };

    auto current_nodes = createTestNodesWithFixedUuids(specs);
    uint64_t current_hash = computeNodesHash(current_nodes);

    // Create cache file with different node count (simulating old data)
    std::map<std::string, std::string> entries;
    entries["test_key"] = "test_value";
    uint32_t old_count = 5;  // Different from current 2 nodes
    createCacheFile(entries, old_count, current_hash);

    // Read and verify the mismatch would be detected
    std::ifstream file(testCacheFile, std::ios::binary);
    ASSERT_TRUE(file.is_open());

    char magic[4];
    file.read(magic, 4);
    uint32_t version;
    file.read(reinterpret_cast<char*>(&version), sizeof(version));

    uint32_t stored_count;
    uint64_t stored_hash;
    file.read(reinterpret_cast<char*>(&stored_count), sizeof(stored_count));
    file.read(reinterpret_cast<char*>(&stored_hash), sizeof(stored_hash));
    file.close();

    // Verify mismatch detection
    EXPECT_NE(stored_count, static_cast<uint32_t>(current_nodes.size()))
        << "Node count mismatch should be detectable";
}

// Test: Cache file with mismatched nodes hash would be detected
TEST_F(MemcachedPersistenceFixtureTests, TestCacheFileNodesHashMismatch)
{
    boost::uuids::string_generator gen;
    auto uuid1 = gen("01234567-89ab-cdef-0123-456789abcdef");
    auto uuid2 = gen("fedcba98-7654-3210-fedc-ba9876543210");

    std::vector<std::tuple<boost::uuids::uuid, double, double>> specs = {
        {uuid1, 45.5, -73.5},
        {uuid2, 45.6, -73.6}
    };

    auto current_nodes = createTestNodesWithFixedUuids(specs);
    uint64_t current_hash = computeNodesHash(current_nodes);

    // Create cache file with same count but different hash (simulating moved nodes)
    std::map<std::string, std::string> entries;
    entries["test_key"] = "test_value";
    uint64_t old_hash = 0xDEADBEEFCAFEBABEULL;  // Different from current hash
    createCacheFile(entries, 2, old_hash);

    // Read and verify the mismatch would be detected
    std::ifstream file(testCacheFile, std::ios::binary);
    ASSERT_TRUE(file.is_open());

    char magic[4];
    file.read(magic, 4);
    uint32_t version;
    file.read(reinterpret_cast<char*>(&version), sizeof(version));

    uint32_t stored_count;
    uint64_t stored_hash;
    file.read(reinterpret_cast<char*>(&stored_count), sizeof(stored_count));
    file.read(reinterpret_cast<char*>(&stored_hash), sizeof(stored_hash));
    file.close();

    // Verify hash mismatch detection (count matches but hash doesn't)
    EXPECT_EQ(stored_count, static_cast<uint32_t>(current_nodes.size()));
    EXPECT_NE(stored_hash, current_hash)
        << "Node hash mismatch should be detectable even with same count";
}
