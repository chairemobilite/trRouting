@0xab0e6c0587bae070;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("modesCollection");

struct modesCollection {
  modes @0 :List(Service);
}

struct Mode {
  id @0 :Int32;
  shortname @1 :Text;
  name @2 :Text;
}

