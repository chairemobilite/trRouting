check_PROGRAMS = csa_test

# Consider refactoring or mocking so that files from ../../src are not necessary
csa_test_SOURCES = gtest.cpp \
    ../../src/calculation_time.cpp \
    ../../src/euclideangeofilter.cpp \
    ../../src/geofilter.cpp \
    ../../src/connection_set.cpp \
    ../../src/connection_cache.cpp \
    ../../src/transit_data.cpp \
    csa_test_base.cpp \
    csa_test_data_fetcher.cpp \
    connection_cache_test.cpp \
    csa_route_calculation_test.cpp \
    csa_access_map_test.cpp \
    csa_route_transfer_alternatives_test.cpp \
    csa_result_to_response_test.cpp \
    csa_result_to_v2_test.cpp \
    csa_result_to_v2_summary_test.cpp \
    csa_result_to_v2_accessibility_test.cpp \
    parameters/route_param_test.cpp \
    parameters/accessibility_param_test.cpp \
    combinations_test.cpp

csa_test_LDADD = $(top_srcdir)/tests/libgtest.la ../../connection_scan_algorithm/src/libcsa.la

csa_test_LDFLAGS = -no-pie -pthread

csa_test_CPPFLAGS = -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -I$(top_srcdir)/connection_scan_algorithm/include -pthread

TESTS = csa_test
