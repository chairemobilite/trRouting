@0xf7637bd96012f3a2;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("networkCollection");

struct NetworkCollection {
  networks @0 :List(Network);
}

struct Network {
  uuid           @0  :Text;
  acronym        @1  :Text;
  shortname      @2  :Text;
  name           @3  :Text;
  internalId     @4  :Text;
  color          @5  :Text;
  isEnabled      @6  :Int8;
  description    @7  :Text;
  data           @8  :Text;
  agenciesUuids  @9  :List(Text);
  servicesUuids  @10 :List(Text);
  scenariosUuids @11 :List(Text);
  isFrozen       @12 :Int8;
  simulationUuid @13 :Text;
}