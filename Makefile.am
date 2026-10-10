ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -Iinclude -Iconnection_scan_algorithm/include

SUBDIRS = include \
	  connection_scan_algorithm \
	  src \
	  tests

# Unit tests of the batched CSA crate
check-local:
	$(CARGO) test --manifest-path $(srcdir)/batched_csa/Cargo.toml


