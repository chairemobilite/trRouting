# trRouting
Transit routing server app written in C++ using the Connection Scan Algorithm including flexible parameters.

## Performance
With random origin and destination (multiple accessible stops at origin and destination): ~150 ms for access and egress footpaths calculation, ~8 ms for CSA two-way calculation (tested with montreal area GTFS data including all urban and suburban transit agencies, with transfer footpaths between stops of 10 minutes walking or less) on a MacPro 2013 with single thread used (you can start multiple servers and execute parallel requests).

Une vrai phrase

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
trRouting have the ability to cache some of the accessible node calculation results. To do so, it can
use an external memcached daemon.
You tell trRouting to do this by passing the --useMemcached parameter. By default it will
try to access memcached on the default port, 11211 on the localhost. You can pass an hostname/port
to the useMemcached parameter like so: --useMemcached=localhost:11111

The memcached support will only be compiled if the configure script can detect libmemcached on
the system, hence why it's marked as optional in the instructions bellow

## Mac OS X Install with homebrew
```
brew install boost
brew install jemalloc
brew install capnp
brew install spdlog
brew install nlohmann-json
brew install libmemcached
```

libmemcached is optional
## Ubuntu 24.04 Install

```
sudo apt install libboost-all-dev libjemalloc-dev libcapnp-dev capnproto libexpat1-dev libjsoncpp-dev libspdlog-dev nlohmann-json3-dev
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
