# trRouting
Transit routing server app written in C++ using the Connection Scan or Trip-Based algorithms and including flexible parameters.

## References
[Connection Scan Algorithm (CSA)][1]  
[Trib-Based Algorithm (TBA)][2]

## Dependencies
[Open Source Routing Machine (OSRM)][3] (must be installed separately, see [install and usage instructions in OSRM Wiki][4])

[1]: http://i11www.iti.uni-karlsruhe.de/extra/publications/dpsw-isftr-13.pdf "Intriguingly Simple and Fast Transit Routing"
[2]: https://arxiv.org/pdf/1504.07149 "Trip-Based Public Transit Routing"
[3]: https://github.com/Project-OSRM/osrm-backend/ "Open Source Routing Machine Github Repository"
[4]: https://github.com/Project-OSRM/osrm-backend/wiki "OSRM Wiki"

## Preparation
Create a trRoutingConfig.yml file (copy from the example trRoutingConfig.example.yml provided) for the Connection Scan Algorithm  
Create a trRoutingTripBasedConfig.yml file (copy from the example trRoutingConfig.example.yml provided) for the Trip Based Algorithm


## Mac OS X Install with homebrew
```
brew install boost
brew install libpqxx
brew install yaml-cpp
brew install msgpack
brew tap nlohmann/json
brew install nlohmann/json/nlohmann_json
```

## Compilation
### Connection Scan Algorithm
```
make -f MakeFile
```
### Trip-Based Algorithm
```
make -f TripBasedMakeFile
```

