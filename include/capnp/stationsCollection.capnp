@0x850138bb0f2a785f;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("stationsCollection");

struct StationsCollection {
  stations @0 :List(Station);
}

struct Station {
  uuid @0 :Text;
  code @1 :Text;
  name @2 :Text;
}