AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/connection_scan_algorithm/include


bin_PROGRAMS = trRouting

trRouting_SOURCES = agencies_cache_fetcher.cpp \
		    cache_fetcher.cpp \
calculation_time.cpp \
csv_fetcher.cpp \
data_sources_cache_fetcher.cpp \
gtfs_fetcher.cpp \
households_cache_fetcher.cpp \
lines_cache_fetcher.cpp \
modes_initialization.cpp \
networks_cache_fetcher.cpp \
nodes_cache_fetcher.cpp \
od_trips_cache_fetcher.cpp \
osrm_fetcher.cpp \
paths_cache_fetcher.cpp \
persons_cache_fetcher.cpp \
places_cache_fetcher.cpp \
scenarios_cache_fetcher.cpp \
services_cache_fetcher.cpp \
stations_cache_fetcher.cpp \
stops_cache_fetcher.cpp \
trips_and_connections_cache_fetcher.cpp


trRouting_LDADD = ../connection_scan_algorithm/src/libcsa.a
