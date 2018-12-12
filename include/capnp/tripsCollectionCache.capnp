@0xcfdfb077dabb958c;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("tripsCollectionCache");

struct ServicesCollectionCache {
  services @0 :List(Trip);
}

struct Trip {
  id @0 :Int64; # unique for the whole network
  lineIdx @1 :Int64; # unique for the whole network
  pathIdx @2 :Int32; # unique for the line
  serviceId @3 :Int64; # unique for the whole network
  scenarioId @4 :Int64; # unique for the whole network
  blockId @5 :Int64; # unique for whole network
  totalCapacity @6 :Int16; # total vehicle capacity for the trip
  seatedCapacity @7 :Int16; # total seated capacity for this trip
}