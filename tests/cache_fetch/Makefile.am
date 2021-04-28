
check_PROGRAMS = gtest

gtest_SOURCES = gtest.cpp \
    ../../src/cache_fetcher.cpp cache_fetcher_test.cpp \
    ../../src/agencies_cache_fetcher.cpp agencies_cache_fetcher_test.cpp \
    ../../src/nodes_cache_fetcher.cpp nodes_cache_fetcher_test.cpp \
    ../../src/services_cache_fetcher.cpp services_cache_fetcher_test.cpp \
    ../../src/scenarios_cache_fetcher.cpp scenarios_cache_fetcher_test.cpp \
    ../../src/lines_cache_fetcher.cpp lines_cache_fetcher_test.cpp \
    ../../src/data_sources_cache_fetcher.cpp data_sources_cache_fetcher_test.cpp \
    ../../src/networks_cache_fetcher.cpp networks_cache_fetcher_test.cpp \
    ../../src/stations_cache_fetcher.cpp stations_cache_fetcher_test.cpp \
    ../../src/paths_cache_fetcher.cpp paths_cache_fetcher_test.cpp \
    ../../src/calculation_time.cpp \
    ../../src/trips_and_connections_cache_fetcher.cpp schedules_cache_fetcher_test.cpp \
    ../../src/households_cache_fetcher.cpp households_cache_fetcher_test.cpp \
    ../../src/persons_cache_fetcher.cpp persons_cache_fetcher_test.cpp \
    ../../src/od_trips_cache_fetcher.cpp od_trips_cache_fetcher_test.cpp \
    ../../src/places_cache_fetcher.cpp places_cache_fetcher_test.cpp

gtest_LDADD = $(top_srcdir)/tests/libgtest.la

gtest_LDFLAGS = -pthread -std=c++17 -lstdc++fs

gtest_CPPFLAGS = -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -I$(top_srcdir)/connection_scan_algorithm/include -pthread -std=c++17 -lstdc++fs

TESTS = gtest


