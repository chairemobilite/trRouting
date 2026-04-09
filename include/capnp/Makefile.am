SUFFIXES = .capnp .capnp.h

.capnp.capnp.h:
	$(CAPNP_BIN) compile -oc++ $<

clean-local:
	rm -f *.capnp.h *.capnp.c++

# TODO we might need to distribute those headers and change the nodist here
nodist_include_HEADERS = agencyCollection.capnp.h \
line.capnp.h \
lineCollection.capnp.h \
node.capnp.h \
nodeCollection.capnp.h \
pathCollection.capnp.h \
scenarioCollection.capnp.h \
serviceCollection.capnp.h
