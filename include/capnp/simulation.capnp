@0xa020a53688508035;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("simulation");

struct Simulation {
  uuid            @0  :Text;
  shortname       @1  :Text;
  name            @2  :Text;
  internalId      @3  :Text;
  color           @4  :Text;
  isEnabled       @5  :Int8;
  isStarted       @6  :Int8;
  isCompleted     @7  :Int8;
  description     @8  :Text;
  data            @9  :Text;
  isFrozen        @10 :Int8;
  simulationClass @11 :Text;
}