@0xe4e03ab1c3b16df5;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("tripsCollection");

struct TripsCollection {
  trips @0 :List(Trip);
}

struct Trip {
  id @0 :Int64; # unique for the whole network
  lineIdx @1 :Int32; # unique for the whole network, from LinesCollection index
  pathIdx @2 :Int32; # unique for the whole network, from PathsCollection index
  serviceIdx @3 :Int32; # unique for the whole network, from ServicesCollection index
  scenarioIdx @4 :Int32; # unique for the whole network, from ScenariosCollection index
  blockIdx @5 :Int64; # unique for whole network, from BlocksCollection index
  totalCapacity @6 :Int16; # total vehicle capacity for the trip
  seatedCapacity @7 :Int16; # total seated capacity for this trip
  allowSameLineTransfers @8 :Int8;
}