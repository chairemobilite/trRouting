# trRouting
Transit routing server app written in C++ using the Connection Scan Algorithm including flexible parameters.

## Performance
With random origin and destination (multiple accessible stops at origin and destination): ~150 ms for access and egress footpaths calculation, ~8 ms for CSA two-way calculation (tested with montreal area GTFS data including all urban and suburban transit agencies, with transfer footpaths between stops of 10 minutes walking or less) on a MacPro 2013 with single thread used (you can start multiple servers and execute parallel requests).

## References
[Connection Scan Algorithm (CSA)][1] (working version)  
[Trib-Based Algorithm (TBA)][2] (not yet released)

## Dependencies
[Open Source Routing Machine (OSRM)][3] (must be installed separately, see [install and usage instructions in OSRM Wiki][4])
use -DBOOST_ROOT option to choose boost path if not default (ex: /usr/lib or /usr/local/lib)

[1]: https://arxiv.org/pdf/1703.05997.pdf "Intriguingly Simple and Fast Transit Routing"
[2]: https://arxiv.org/pdf/1504.07149v2.pdf "Trip-Based Public Transit Routing"
[3]: https://github.com/Project-OSRM/osrm-backend/ "Open Source Routing Machine Github Repository"
[4]: https://github.com/Project-OSRM/osrm-backend/wiki "OSRM Wiki"

## Mac OS X Install with homebrew
```
brew install boost
brew install libpqxx
brew install capnp
brew tap nlohmann/json
brew install nlohmann/json/nlohmann_json
```

## Ubuntu 16.04 Install

```
sudo apt-get install clang libboost-all-dev libexpat1-dev libjsoncpp-dev libpqxx-4.0 libpqxx-dev libncurses5-dev
```

## Compilation
### Connection Scan Algorithm

```
make -f MakeFileCSA
```
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2Fkaligrafy%2FtrRouting.svg?type=large)](https://app.fossa.io/projects/git%2Bgithub.com%2Fkaligrafy%2FtrRouting?ref=badge_large)
