AC_INIT([trRouting], [1.0], [https://github.com/chairemobilite/trRouting])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

AC_CONFIG_MACRO_DIRS([m4])

AC_PROG_CC
AC_PROG_CXX
AM_PROG_AR

LT_INIT

# AX_ macros copied from the autoconf-archive

# TODO Do we need the strict mode or not here ?
AX_CXX_COMPILE_STDCXX(17,noext)

AX_BOOST_BASE
AX_BOOST_SYSTEM # For include/asio_compatibility.hpp
AX_BOOST_REGEX # For include/server_http.hpp
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_DATE_TIME # For include/service.hpp

AX_PTHREAD # Explicitely checking, needed by std::thread

PKG_CHECK_MODULES(CAPNP, capnp)
# Check for the capnp binary presence
AC_PATH_PROG(CAPNP_BIN, capnp, no)
AS_IF([test x"$CAPNP_BIN" = x"no"], [AC_MSG_ERROR([Please install capnp binary.])])

PKG_CHECK_MODULES(SPDLOG, spdlog)

PKG_CHECK_MODULES(JSON, nlohmann_json)

# TODO: Add link to doc of this override flag
CPPFLAGS+=" -DBOOST_BIND_GLOBAL_PLACEHOLDERS"

CPPFLAGS+=" $BOOST_CPPFLAGS $CAPNP_CFLAGS $PTHREAD_CFLAGS $SPDLOG_CFLAGS $JSON_CLAGS"

# Add BOOST flags
LDFLAGS+=" $BOOST_LDFLAGS $BOOST_REGEX_LIB $BOOST_SYSTEM_LIB $BOOST_PROGRAM_OPTIONS_LIB $BOOST_DATE_TIME_LIB"
# Add various flags
LDFLAGS+=" $PTHREAD_LIBS"
LDFLAGS+=" $CAPNP_LIBS"
LDFLAGS+=" $SPDLOG_LIBS"

CXXFLAGS+=" -Wall -Wextra"

# Do not enable benchmark in the make check by default
AC_ARG_ENABLE([benchmark],
    [  --enable-benchmark      Benchmarks will be run when running make check],
    [case "${enableval}" in
        yes) benchmark=true ;;
        no)  benchmark=false ;;
        *) AC_MSG_ERROR([bad value ${enableval} for --enable-benchmark]) ;;
    esac],[benchmark=false])
AS_IF([test x"$benchmark" = xfalse], [AC_MSG_NOTICE([Benchmarks are disabled. Enable them by adding the --enable-benchmark flag to configure.])])
AM_CONDITIONAL([RUN_BENCHMARK], [test x$benchmark = xtrue])

AC_CONFIG_FILES([
 Makefile
 src/Makefile
 tests/Makefile
 tests/cache_fetch/Makefile
 tests/benchmark_csa/Makefile
 tests/connection_scan_algorithm/Makefile
 include/Makefile
 include/capnp/Makefile
 connection_scan_algorithm/Makefile
 connection_scan_algorithm/src/Makefile
])
AC_OUTPUT
