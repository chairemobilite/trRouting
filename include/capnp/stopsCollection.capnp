@0xb37b24859b9278c5;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("stopsCollection");

struct StopsCollection {
  stops @0 :List(Stop);
}

struct Stop {
  id @0 :Int64;
  code @1 :Text;
  name @2 :Text;
  latitude @3 :Float32;
  longitude @4 :Float32;
  stationId @5 :Int64;
}

