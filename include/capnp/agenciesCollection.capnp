@0xda2c404d77413fd7;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("agenciesCollection");

struct agenciesCollection {
  agencies @0 :List(Agency);
}

struct Agency {
  id @0 :Int32;
  uuid @1 :Text;
  acronym @2 :Text;
  name @3 :Text;
}

