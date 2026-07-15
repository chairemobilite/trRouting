AC_INIT([trRouting], [1.0], [https://github.com/chairemobilite/trRouting])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

AC_CONFIG_MACRO_DIRS([m4])

AC_PROG_CC
AC_PROG_CXX
AM_PROG_AR

LT_INIT

# AX_ macros copied from the autoconf-archive

# TODO Do we need the strict mode or not here ?
AX_CXX_COMPILE_STDCXX(17,noext,mandatory)

AX_BOOST_BASE
AX_BOOST_SYSTEM # For include/asio_compatibility.hpp
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_DATE_TIME # For include/service.hpp
AX_BOOST_SERIALIZATION # For memcached serialization

AX_PTHREAD # Explicitely checking, needed by std::thread

PKG_CHECK_MODULES([JEMALLOC], [jemalloc]) # For performance enhancement

PKG_CHECK_MODULES(CAPNP, capnp)
# Check for the capnp binary presence
AC_PATH_PROG(CAPNP_BIN, capnp, no)
AS_IF([test x"$CAPNP_BIN" = x"no"], [AC_MSG_ERROR([Please install capnp binary.])])

PKG_CHECK_MODULES(SPDLOG, spdlog)

PKG_CHECK_MODULES(JSON, nlohmann_json)

# Check for libmemcached and libmemcachedutil
AC_ARG_WITH([memcached],
    [AS_HELP_STRING([--with-memcached],
        [enable memcached support @<:@default=check@:>@])],
    [],
    [with_memcached=check])

# Default to not having memcached
have_memcached=no

AS_IF(
    [test "x$with_memcached" != xno],
    [PKG_CHECK_MODULES([MEMCACHED], [libmemcached],
        [# Now check for libmemcachedutil
         AC_CHECK_LIB([memcachedutil], [memcached_pool_create],
            [have_memcached=yes
             AC_DEFINE([HAVE_MEMCACHED], [1], [Define if libmemcached and libmemcachedutil are available])
             MEMCACHED_LIBS="$MEMCACHED_LIBS -lmemcachedutil"],
            [# Both libraries are required
             AS_IF([test "x$with_memcached" != xcheck],
                   [AC_MSG_FAILURE([--with-memcached was given, but libmemcachedutil was not found])])])],
        [AS_IF([test "x$with_memcached" != xcheck],
               [AC_MSG_FAILURE([--with-memcached was given, but libmemcached was not found])])])])

# Set memcached conditional
AM_CONDITIONAL([HAVE_MEMCACHED], [test "x$have_memcached" = xyes])

# Required by drogon
PKG_CHECK_MODULES([JSONCPP], [jsoncpp])
CPPFLAGS="$CPPFLAGS $JSONCPP_CFLAGS"
# Drogon (no official .pc file; check header + libs directly)
# Fiddle with include path for macOS brew package
if test -x "$(command -v brew)"; then
  BREW_PREFIX=`brew --prefix`
  CPPFLAGS="$CPPFLAGS -I$BREW_PREFIX/include"
  LDFLAGS="$LDFLAGS -L$BREW_PREFIX/lib"
fi
AC_LANG_PUSH([C++])
AC_CHECK_HEADER([drogon/drogon.h], [], [AC_MSG_ERROR([drogon headers not found])])
AC_LANG_POP([C++])
# TODO The trantor and drogon should check actual lib functions
AC_CHECK_LIB([trantor], [main], [], [AC_MSG_ERROR([libtrantor not found])])
AC_CHECK_LIB([drogon], [main], [], [AC_MSG_ERROR([libdrogon not found])])

# TODO: Add link to doc of this override flag
CPPFLAGS+=" -DBOOST_BIND_GLOBAL_PLACEHOLDERS"

CPPFLAGS+=" $BOOST_CPPFLAGS $CAPNP_CFLAGS $PTHREAD_CFLAGS $SPDLOG_CFLAGS $JSON_CLAGS"
# Add memcached flags if available
AS_IF([test "x$have_memcached" = xyes],
      [CPPFLAGS+=" $MEMCACHED_CFLAGS"])

# Add BOOST flags
LDFLAGS+=" $BOOST_LDFLAGS $BOOST_SYSTEM_LIB $BOOST_PROGRAM_OPTIONS_LIB $BOOST_DATE_TIME_LIB $BOOST_SERIALIZATION_LIB"
# Add various flags
LDFLAGS+=" $PTHREAD_LIBS"
LDFLAGS+=" $JEMALLOC_LIBS"
LDFLAGS+=" $CAPNP_LIBS"
LDFLAGS+=" $SPDLOG_LIBS"
LDFLAGS+=" $JSONCPP_LIBS"
# Add memcached flags if available
AS_IF([test "x$have_memcached" = xyes],
      [LDFLAGS+=" $MEMCACHED_LIBS"])

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
