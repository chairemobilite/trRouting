
check_PROGRAMS = gtest

gtest_SOURCES = gtest.cpp \
    ../../src/cache_fetcher.cpp cache_fetcher_test.cpp

gtest_LDADD = $(top_srcdir)/tests/libgtest.la

gtest_LDFLAGS = -pthread

gtest_CPPFLAGS = -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -I$(top_srcdir)/connection_scan_algorithm/include -pthread

TESTS = gtest


