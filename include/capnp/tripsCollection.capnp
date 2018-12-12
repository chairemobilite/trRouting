@0xf56b3972f9b3be3d;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("tripsCollection");

struct TripsCollection {
  trips @0 :List(Trip);
}

struct Trip {
  id @0 :Int32; # we don't use uuids here because trips are created on the fly when exporting from transition
  lineIdx @1 :Int32;
  pathIdx @2 :Int32;
  modeIdx @3 :Int32;
  agencyIdx @4 :Int32;
  serviceIdx @5 :Int32;
  scenarioIdx @6 :Int32;
  totalCapacity @7 :Int16; # total vehicle capacity for the trip
  seatedCapacity @8 :Int16; # total seated capacity for this trip
}