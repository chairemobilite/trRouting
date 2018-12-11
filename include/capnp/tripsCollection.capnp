@0xf56b3972f9b3be3d;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("tripsCollection");

struct TripsCollection {
  trips @0 :List(Trip);
}

struct Trip {
  id @0 :Int64;
  lineId @1 :Int32;
  pathId @2 :Int32;
  modeId @3 :Int32;
  agencyId @4 :Int32;
  serviceId @5 :Int32;
  scenarioId @6 :Int32;
}