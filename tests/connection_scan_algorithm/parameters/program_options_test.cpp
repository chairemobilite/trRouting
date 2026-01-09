#include "gtest/gtest.h"
#include "program_options.hpp"

class ProgramOptionsFixtureTests : public ::testing::Test {
protected:
    TrRouting::ProgramOptions programOptions;
};

// Test: Default memcached persist path should be "footpaths.cache"
TEST_F(ProgramOptionsFixtureTests, TestDefaultMemcachedPersistPath)
{
    const char* argv[] = {"trRouting"};
    programOptions.parseOptions(1, const_cast<char**>(argv));

    EXPECT_EQ(programOptions.memcachedPersistPath, "footpaths.cache");
}

// Test: Custom memcached persist path
TEST_F(ProgramOptionsFixtureTests, TestCustomMemcachedPersistPath)
{
    const char* argv[] = {"trRouting", "--memcachedPersistPath=/custom/path/cache.dat"};
    programOptions.parseOptions(2, const_cast<char**>(argv));

    EXPECT_EQ(programOptions.memcachedPersistPath, "/custom/path/cache.dat");
}

// Test: Empty memcached persist path (using "" explicitly)
// Note: --memcachedPersistPath= doesn't work with boost::program_options
// Users need to pass --memcachedPersistPath "" or use the default
TEST_F(ProgramOptionsFixtureTests, TestExplicitEmptyMemcachedPersistPath)
{
    const char* argv[] = {"trRouting", "--memcachedPersistPath", ""};
    programOptions.parseOptions(3, const_cast<char**>(argv));

    EXPECT_EQ(programOptions.memcachedPersistPath, "");
}

// Test: Default useMemcached should be false
TEST_F(ProgramOptionsFixtureTests, TestDefaultUseMemcached)
{
    const char* argv[] = {"trRouting"};
    programOptions.parseOptions(1, const_cast<char**>(argv));

    EXPECT_FALSE(programOptions.useMemcached);
}

// Test: useMemcached flag without value uses default server
TEST_F(ProgramOptionsFixtureTests, TestUseMemcachedWithoutValue)
{
    const char* argv[] = {"trRouting", "--useMemcached"};
    programOptions.parseOptions(2, const_cast<char**>(argv));

    EXPECT_TRUE(programOptions.useMemcached);
    EXPECT_EQ(programOptions.memcachedServers, "localhost:11211");
}

// Test: useMemcached with custom server
TEST_F(ProgramOptionsFixtureTests, TestUseMemcachedWithCustomServer)
{
    const char* argv[] = {"trRouting", "--useMemcached=192.168.1.100:11211"};
    programOptions.parseOptions(2, const_cast<char**>(argv));

    EXPECT_TRUE(programOptions.useMemcached);
    EXPECT_EQ(programOptions.memcachedServers, "192.168.1.100:11211");
}

// Test: Both memcached options together
TEST_F(ProgramOptionsFixtureTests, TestMemcachedOptionsTogetherWithCustomServer)
{
    const char* argv[] = {
        "trRouting",
        "--useMemcached=cache.example.com:11211",
        "--memcachedPersistPath=/var/cache/trrouting.cache"
    };
    programOptions.parseOptions(3, const_cast<char**>(argv));

    EXPECT_TRUE(programOptions.useMemcached);
    EXPECT_EQ(programOptions.memcachedServers, "cache.example.com:11211");
    EXPECT_EQ(programOptions.memcachedPersistPath, "/var/cache/trrouting.cache");
}

// Test: Memcached persist path works without enabling useMemcached
TEST_F(ProgramOptionsFixtureTests, TestMemcachedPersistPathWithoutUseMemcached)
{
    const char* argv[] = {"trRouting", "--memcachedPersistPath=/some/path.cache"};
    programOptions.parseOptions(2, const_cast<char**>(argv));

    // useMemcached is still false, but persist path is set
    EXPECT_FALSE(programOptions.useMemcached);
    EXPECT_EQ(programOptions.memcachedPersistPath, "/some/path.cache");
}

// Test: Default port value
TEST_F(ProgramOptionsFixtureTests, TestDefaultPort)
{
    const char* argv[] = {"trRouting"};
    programOptions.parseOptions(1, const_cast<char**>(argv));

    EXPECT_EQ(programOptions.port, 4000);
}

// Test: Default number of threads
TEST_F(ProgramOptionsFixtureTests, TestDefaultThreads)
{
    const char* argv[] = {"trRouting"};
    programOptions.parseOptions(1, const_cast<char**>(argv));

    EXPECT_EQ(programOptions.numberOfThreads, 1);
}

// Test: Custom number of threads
TEST_F(ProgramOptionsFixtureTests, TestCustomThreads)
{
    const char* argv[] = {"trRouting", "--threads=8"};
    programOptions.parseOptions(2, const_cast<char**>(argv));

    EXPECT_EQ(programOptions.numberOfThreads, 8);
}

// Test: Default useEuclideanDistance
TEST_F(ProgramOptionsFixtureTests, TestDefaultUseEuclideanDistance)
{
    const char* argv[] = {"trRouting"};
    programOptions.parseOptions(1, const_cast<char**>(argv));

    EXPECT_FALSE(programOptions.useEuclideanDistance);
}

// Test: Default cache path
TEST_F(ProgramOptionsFixtureTests, TestDefaultCachePath)
{
    const char* argv[] = {"trRouting"};
    programOptions.parseOptions(1, const_cast<char**>(argv));

    EXPECT_EQ(programOptions.cachePath, "cache");
}

// Test: Custom cache path
TEST_F(ProgramOptionsFixtureTests, TestCustomCachePath)
{
    const char* argv[] = {"trRouting", "--cachePath=/data/transit_cache"};
    programOptions.parseOptions(2, const_cast<char**>(argv));

    EXPECT_EQ(programOptions.cachePath, "/data/transit_cache");
}

// Test: Multiple servers in useMemcached
TEST_F(ProgramOptionsFixtureTests, TestMultipleMemcachedServers)
{
    const char* argv[] = {"trRouting", "--useMemcached=server1:11211,server2:11211,server3:11211"};
    programOptions.parseOptions(2, const_cast<char**>(argv));

    EXPECT_TRUE(programOptions.useMemcached);
    EXPECT_EQ(programOptions.memcachedServers, "server1:11211,server2:11211,server3:11211");
}
