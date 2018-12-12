@0xab0e6c0587bae070;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("modesCollection");

struct ModesCollection {
  modes @0 :List(Mode);
}

struct Mode {
  shortname @0 :Text;
  name @1 :Text;
}

