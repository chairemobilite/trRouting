@0x8aac323e4ce89a50;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("simulationCollection");

using Simulation = import "simulation.capnp".Simulation;

struct SimulationCollection {
  simulations @0 :List(Simulation);
}
