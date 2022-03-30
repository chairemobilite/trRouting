SUFFIXES = .capnp .capnp.h

.capnp.capnp.h:
	$(CAPNP_BIN) compile -oc++ $<

clean-local:
	rm *.capnp.h *.capnp.c++

# TODO we might need to distribute those headers and change the nodist here
nodist_include_HEADERS = agencyCollection.capnp.h \
dataSource.capnp.h \
dataSourceCollection.capnp.h \
household.capnp.h \
householdCollection.capnp.h \
line.capnp.h \
lineCollection.capnp.h \
networkCollection.capnp.h \
node.capnp.h \
nodeCollection.capnp.h \
odTrip.capnp.h \
odTripCollection.capnp.h \
pathCollection.capnp.h \
person.capnp.h \
personCollection.capnp.h \
place.capnp.h \
placeCollection.capnp.h \
scenarioCollection.capnp.h \
serviceCollection.capnp.h \
stationCollection.capnp.h
