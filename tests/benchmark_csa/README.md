## Running the benchmarks

The benchmarks require cache data to be available in the tests/benchmark_csa/cache/demo_transition directory. The cache data to match the benchmark can be found here:

https://nuage.facil.services/s/Gs7bwfEGaBjHCLg 

Simply unzip in the tests/benchmark_csa/cache directory. It corresponds to the STM's fall 2018 18S_S service (Société des Transports de Montréal).

To run, either run `make check` after having configured the repository with `./configure --enable-benchmark`, or simply run the `gtest` application in this directory after a first run of `make check`.