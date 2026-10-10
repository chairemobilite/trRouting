AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/connection_scan_algorithm/include -I$(top_srcdir)/batched_csa/include

noinst_LTLIBRARIES = libcsa.la

libcsa_la_SOURCES = alternative_filter.cpp \
	           alternatives_routing.cpp \
		   batched_forward_calculation.cpp \
		   calculator.cpp \
		   compact_connection_set.cpp \
		   compact_transfers.cpp \
		   forward_calculation.cpp \
		   forward_journey.cpp \
		   initializations.cpp \
		   optimize_journey.cpp \
		   parameters/common_parameters.cpp \
		   parameters/route_parameters.cpp \
		   parameters/accessibility_parameters.cpp \
		   program_options.cpp \
		   resets.cpp \
		   reverse_calculation.cpp \
		   reverse_journey.cpp \
		   result_to_v2.cpp \
		   result_to_v2_summary.cpp \
		   result_to_v2_accessibility.cpp \
		   transit_routing_http_server.cpp

# The batched_csa Rust crate, built by cargo which tracks its own sources, so it always runs.
# libtool does not bundle it in libcsa.la: programs linking libcsa.la also link $(BATCHED_CSA_LIB)
EXTRA_libcsa_la_DEPENDENCIES = $(BATCHED_CSA_LIB)

$(BATCHED_CSA_LIB): FORCE
	$(CARGO) build --release --manifest-path $(top_srcdir)/batched_csa/Cargo.toml --target-dir $(abs_top_builddir)/batched_csa/target

FORCE:
.PHONY: FORCE
