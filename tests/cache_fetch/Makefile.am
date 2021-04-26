
check_PROGRAMS = gtest

gtest_SOURCES = gtest.cpp \
    ../../src/cache_fetcher.cpp cache_fetcher_test.cpp \
    ../../src/agencies_cache_fetcher.cpp agencies_cache_fetcher_test.cpp \
    ../../src/nodes_cache_fetcher.cpp nodes_cache_fetcher_test.cpp \
    ../../src/services_cache_fetcher.cpp services_cache_fetcher_test.cpp \
    ../../src/lines_cache_fetcher.cpp lines_cache_fetcher_test.cpp

gtest_LDADD = $(top_srcdir)/tests/libgtest.la

gtest_LDFLAGS = -pthread -std=c++17 -lstdc++fs

gtest_CPPFLAGS = -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -I$(top_srcdir)/connection_scan_algorithm/include -pthread -std=c++17 -lstdc++fs

TESTS = gtest


