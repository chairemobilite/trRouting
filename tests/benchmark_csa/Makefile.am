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
    ../../src/paths_cache_fetcher.cpp \
    ../../src/calculation_time.cpp \
    ../../src/trips_and_connections_cache_fetcher.cpp \
    ../../src/persons_cache_fetcher.cpp \
    ../../src/modes_initialization.cpp \
    ../../src/od_trips_cache_fetcher.cpp \
    ../../src/connection_set.cpp \
    ../../src/connection_cache.cpp \
    ../../src/transit_data.cpp \
    ../../src/geofilter.cpp \
    ../../src/euclideangeofilter.cpp

#TODO #167 Place/Household removed while refactoring
#../../src/households_cache_fetcher.cpp \
#../../src/places_cache_fetcher.cpp

gtest_LDADD = $(top_srcdir)/tests/libgtest.la ../../connection_scan_algorithm/src/libcsa.la

gtest_LDFLAGS = -no-pie -pthread

gtest_CPPFLAGS = -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -I$(top_srcdir)/connection_scan_algorithm/include -pthread

if RUN_BENCHMARK
TESTS = gtest
else
TESTS =
endif



