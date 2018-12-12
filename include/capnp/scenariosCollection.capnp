@0x9b44b39f61ff2481;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("scenariosCollection");

struct ScenariosCollection {
  scenarios @0 :List(Scenario);
}

struct Scenario {
  uuid @0 :Text;
  id @1 :Int64;  # unique for the whole network 
  name @2 :Text;
}

