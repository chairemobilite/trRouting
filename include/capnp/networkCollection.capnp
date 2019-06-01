@0xf7637bd96012f3a2;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("networkCollection");

struct NetworkCollection {
  networks @0 :List(Network);
}

struct Network {
  uuid           @0  :Text;
  shortname      @1  :Text;
  name           @2  :Text;
  internalId     @3  :Text;
  color          @4  :Text;
  isEnabled      @5  :Int8;
  description    @6  :Text;
  data           @7  :Text;
  agenciesUuids  @8  :List(Text);
  servicesUuids  @9  :List(Text);
  scenariosUuids @10 :List(Text);
  isFrozen       @11 :Int8;
  simulationUuid @12 :Text;
}