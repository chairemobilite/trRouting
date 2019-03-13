@0xda2c404d77413fd7;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("agencyCollection");

struct AgencyCollection {
  agencies @0 :List(Agency);
}

struct Agency {
  uuid        @0 :Text;
  acronym     @1 :Text;
  name        @2 :Text;
  internalId  @3 :Text;
  color       @4 :Text;
  isEnabled   @5 :Int8;
  description @6 :Text;
  data        @7 :Text;
}