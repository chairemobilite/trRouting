@0x9b44b39f61ff2481;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("scenarioCollection");

struct ScenarioCollection {
  scenarios @0 :List(Scenario);
}

struct Scenario {
  uuid @0 :Text;
  name @1 :Text;
  color @2 :Text;
  isEnabled @3 :Int8;
  description @4 :Text;
  data @5 :Text;
  servicesUuids @6 :List(Text);
}

