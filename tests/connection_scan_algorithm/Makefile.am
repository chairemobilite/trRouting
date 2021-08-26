check_PROGRAMS = gtest

gtest_SOURCES = gtest.cpp \
    benchmark_CSA_test.cpp \
    ../../src/cache_fetcher.cpp \
    ../../src/agencies_cache_fetcher.cpp \
    ../../src/nodes_cache_fetcher.cpp \
    ../../src/services_cache_fetcher.cpp \
    ../../src/scenarios_cache_fetcher.cpp \
    ../../src/lines_cache_fetcher.cpp \
    ../../src/data_sources_cache_fetcher.cpp \
    ../../src/networks_cache_fetcher.cpp \
    ../../src/stations_cache_fetcher.cpp \
    ../../src/paths_cache_fetcher.cpp \
    ../../src/calculation_time.cpp \
    ../../src/trips_and_connections_cache_fetcher.cpp \
    ../../src/households_cache_fetcher.cpp \
    ../../src/persons_cache_fetcher.cpp ../../src/modes_initialization.cpp \
    ../../src/od_trips_cache_fetcher.cpp ../../src/osrm_fetcher.cpp \
    ../../src/places_cache_fetcher.cpp


gtest_LDADD = $(top_srcdir)/tests/libgtest.la ../../connection_scan_algorithm/src/libcsa.a

gtest_LDFLAGS = -no-pie -pthread -std=c++17 -lstdc++fs

gtest_CPPFLAGS = -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -I$(top_srcdir)/connection_scan_algorithm/include -pthread -std=c++17 -lstdc++fs

TESTS = gtest


