@0x9b44b39f61ff2481;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("scenariosCollection");

struct scenariosCollection {
  scenarios @0 :List(Scenario);
}

struct Scenario {
  id @0 :Int32;
  uuid @1 :Text;
  name @2 :Text;
}

