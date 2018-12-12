@0xda2c404d77413fd7;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("agenciesCollection");

struct AgenciesCollection {
  agencies @0 :List(Agency);
}

struct Agency {
  uuid @0 :Text;
  acronym @1 :Text;
  name @2 :Text;
}

