check_PROGRAMS = csa_test

# Consider refactoring or mocking so that files from ../../src are not necessary
csa_test_SOURCES = gtest.cpp \
    ../../src/calculation_time.cpp \
    ../../src/osrm_fetcher.cpp \
    csa_test_base.cpp \
    csa_v1_simple_calculation_test.cpp \
    csa_v1_transfer_and_alternatives_test.cpp \
    csa_v1_access_map_test.cpp \
    csa_v1_odtrips_calculation_test.cpp \
    csa_route_calculation_test.cpp \
    csa_route_transfer_alternatives_test.cpp \
    route_parameters_test.cpp

csa_test_LDADD = $(top_srcdir)/tests/libgtest.la ../../connection_scan_algorithm/src/libcsa.la

csa_test_LDFLAGS = -no-pie -pthread -std=c++17 -lstdc++fs

csa_test_CPPFLAGS = -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -I$(top_srcdir)/connection_scan_algorithm/include -pthread -std=c++17 -lstdc++fs


TESTS = csa_test
