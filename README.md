# trRouting
Transit routing server app written in C++ using the Connection Scan Algorithm including flexible parameters.

## Performance
With random origin and destination (multiple accessible stops at origin and destination): ~150 ms for access and egress footpaths calculation, ~8 ms for CSA two-way calculation (tested with montreal area GTFS data including all urban and suburban transit agencies, with transfer footpaths between stops of 10 minutes walking or less) on a MacPro 2013 with single thread used (you can start multiple servers and execute parallel requests).

## References
[Connection Scan Algorithm (CSA)][1] (working version)  
[Trib-Based Algorithm (TBA)][2] (not yet released)

## API Documentation

Documentation of the trRouting API can be found here: 
https://chairemobilite.github.io/trRouting/

## Dependencies
[Open Source Routing Machine (OSRM)][3] (an osrm server with a walking profile must be running for the transit region while making queries to the trRouting server, see [OSRM profiles][5] for more profile info and [Running OSRM][6] to know how to prepare osm data for OSRM and start the server)

[1]: https://i11www.iti.kit.edu/extra/publications/dpsw-isftr-13.pdf "Intriguingly Simple and Fast Transit Routing"
[2]: https://arxiv.org/pdf/1504.07149v2.pdf "Trip-Based Public Transit Routing"
[3]: https://github.com/Project-OSRM/osrm-backend/ "Open Source Routing Machine Github Repository"
[4]: https://github.com/Project-OSRM/osrm-backend/wiki "OSRM Wiki"
[5]: https://github.com/Project-OSRM/osrm-backend/blob/master/docs/profiles.md "OSRM profiles"
[6]: https://github.com/Project-OSRM/osrm-backend/wiki/Running-OSRM "Running OSRM"

### Memcached support
trRouting can cache accessible node calculation results using an external memcached daemon.
This significantly speeds up repeated queries with the same origin/destination locations.

#### Installing memcached

**macOS (Homebrew):**
```bash
brew install memcached
```

**Ubuntu/Debian:**
```bash
sudo apt install memcached
```

#### Running memcached

**macOS - Start as a background service:**
```bash
brew services start memcached
```

**macOS - Run manually (foreground):**
```bash
memcached -l localhost -p 11211
```

**Ubuntu - Start as a service:**
```bash
sudo systemctl start memcached
sudo systemctl enable memcached  # To start on boot
```

**Ubuntu - Run manually:**
```bash
memcached -l 127.0.0.1 -p 11211 -m 64
```

The `-m` flag sets the maximum memory in MB (default is 64MB). Increase this if you have many unique origin/destination pairs.

#### Using memcached with trRouting

Enable caching by passing the `--useMemcached` parameter:
```bash
./trRouting --useMemcached
```

By default, trRouting connects to `localhost:11211`. To use a different server:
```bash
./trRouting --useMemcached=localhost:11211
```

#### Cache persistence

trRouting can persist the cache to disk so it survives restarts. By default, the cache is saved to `footpaths.cache`:
```bash
./trRouting --useMemcached --memcachedPersistPath=footpaths.cache
```

To disable persistence, pass an empty path:
```bash
./trRouting --useMemcached --memcachedPersistPath=""
```

**What is cached:**

The cache stores the walking travel times and distances from geographic coordinates to nearby transit nodes (and vice versa). These are computed by OSRM and can be expensive to calculate repeatedly.

**Automatic behavior:**
- **On startup**: The cache file is loaded and validated against current transit nodes
- **On shutdown**: Cache is saved when receiving Ctrl+C (SIGINT) or SIGTERM signals
- **On first query**: Node validation occurs to ensure cache validity

**API endpoints for manual control:**
- `GET /saveCache` - Save cache to disk immediately
- `GET /loadCache` - Reload cache from disk
- `GET /resetCache` - Clear all cache (memcached + local + delete cache file)
- `GET /cacheStatus` - Get current cache status and entry count

#### When the cache is invalidated

**Transit node changes:**

The cache file stores a hash signature of all transit nodes (their UUIDs and locations). On startup, this signature is validated against the current nodes. If any nodes have been:
- Added or removed
- Relocated (latitude/longitude changed)

The cache is automatically invalidated and cleared. You'll see a log message like:
```
Cache invalidated: nodes have changed (count: 1000 -> 1005, hash: abc123 -> def456)
Clearing cached footpaths data - will recalculate on demand
```

**OSRM data changes:**

**Important:** The cache does NOT detect changes to OSRM routing data. If you update your OSRM data (e.g., new OpenStreetMap extract, updated walking network), you must manually invalidate the cache:

1. **Delete the cache file:**
   ```bash
   rm footpaths.cache
   ```

2. **Or restart with a fresh cache:**
   ```bash
   ./trRouting --useMemcached --memcachedPersistPath=""  # Run without persistence once
   ```

3. **Or flush memcached:**
   ```bash
   echo "flush_all" | nc localhost 11211
   ```

Failure to invalidate the cache after OSRM updates will result in stale walking times that don't reflect the new routing data.

#### Build requirements

The memcached support will only be compiled if the configure script can detect libmemcached on
the system, hence why it's marked as optional in the instructions below

## Mac OS X Install with homebrew
```
brew install boost
brew install capnp
brew install spdlog
brew install nlohmann-json
brew install libmemcached
```

libmemcached is optional
## Ubuntu 24.04 Install

```
sudo apt install libboost-all-dev libcapnp-dev capnproto libexpat1-dev libjsoncpp-dev libspdlog-dev nlohmann-json3-dev
```
You if you haven't installed other basic build dependencies, like autoconf, you will need to install them:
```
sudo apt install build-essential autoconf pkg-config
```

libmemcached-dev is optional
## Compilation
trRouting use autoconf/automake as its build system. A recap of the usual commands: 

If you are running out of a git checkout: 
```
autoreconf -i
```

Then: 
```
./configure
make
```

## Test

trRouting uses [Googletest](https://github.com/google/googletest) to unit test the application. To run the tests, you must first fetch the googletest submodule once into the repo:

```
git submodule init
git submodule update
```

Then, to run the unit tests individually, simply run `make check`.

If you get the following error when running `make check`, that's because the submodule initiation and update was done after the compilation configuration. Simply run again `autoreconf -i && ./configure` and it should work.

```
Makefile:443: ../googletest/googletest/src/.deps/libgtest_la-gtest-all.Plo: No such file or directory
make[1]: *** No rule to make target '../googletest/googletest/src/.deps/libgtest_la-gtest-all.Plo'.  Stop.
```

### Benchmarks

This repo also contains benchmarks to run various calculations. The benchmarks are under the `tests/` along, with the unit tests. To automatically run them with the `make check` command, configure the repo by running `./configure --enable-benchmark`. Otherwise, benchmarks can be executed manually by running the executable in the benchmark directory.

See the README in the benchmark's directory for additional instructions to run them.

## Docker
A provided dockerfile allows to easily build an image

### Build
`docker build -t LOCAL_IMAGE_NAME .`

### Running as a deamon
`docker run -t LOCAL_IMAGE_NAME`


[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2Fkaligrafy%2FtrRouting.svg?type=large)](https://app.fossa.io/projects/git%2Bgithub.com%2Fkaligrafy%2FtrRouting?ref=badge_large)
