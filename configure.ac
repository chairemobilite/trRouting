AC_INIT([trRouting], [1.0], [https://github.com/kaligrafy/trRouting])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

AC_CONFIG_MACRO_DIRS([m4])

AC_PROG_CC
AC_PROG_CXX
AM_PROG_AR

LT_INIT

# AX_ macros copied from the autoconf-archive

# TODO Do we need the strict mode or not here ?
AX_CXX_COMPILE_STDCXX_14(noext)

AX_BOOST_BASE
AX_BOOST_FILESYSTEM
AX_BOOST_IOSTREAMS
AX_BOOST_THREAD
AX_BOOST_SERIALIZATION
AX_BOOST_SYSTEM
AX_BOOST_REGEX
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_DATE_TIME
AX_BOOST_FILESYSTEM

AX_WITH_CURSES

PKG_CHECK_MODULES(CAPNP, capnp)
# Check for the capnp binary presence
AC_PATH_PROG(CAPNP_BIN, capnp, no)
AS_IF([test x"$CAPNP_BIN" = x"no"], [AC_MSG_ERROR([Please install capnp binary.])])

# TODO There's probably a cleaner way to add -lrt required by boost::interprocess
AC_CHECK_LIB(rt, shm_open)

# TODO: Add link to doc of this override flag
CPPFLAGS+=" -DBOOST_BIND_GLOBAL_PLACEHOLDERS"

CPPFLAGS+=" $CURSES_CFLAGS $CAPNP_CFLAGS"

# Add BOOST flags
LDFLAGS+=" $BOOST_LDFLAGS $BOOST_REGEX_LIB $BOOST_FILESYSTEM_LIB $BOOST_IOSTREAMS_LIB $BOOST_THREAD_LIB $BOOST_SERIALIZATION_LIB $BOOST_SYSTEM_LIB $BOOST_PROGRAM_OPTIONS_LIB $BOOST_DATE_TIME_LIB"
# Add various flags
LDFLAGS+=" $CURSES_LIBS"
LDFLAGS+=" $CAPNP_LIBS"

AC_CONFIG_FILES([
 Makefile
 src/Makefile
 tests/Makefile
 tests/cache_fetch/Makefile
 include/Makefile
 include/capnp/Makefile
 connection_scan_algorithm/Makefile
 connection_scan_algorithm/src/Makefile
])
AC_OUTPUT
