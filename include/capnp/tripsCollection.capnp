@0xe4e03ab1c3b16df5;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("tripsCollection");

struct TripsCollection {
  trips @0 :List(Trip);
  blocks @1 :List(Text);
}

struct Trip {
  uuid @0 :Text;
  agencyIdx @1 :Int32;
  lineIdx @2 :Int32;
  pathIdx @3 :Int32;
  modeIdx @4 :Int32;
  serviceIdx @5 :Int32;
  blockIdx @6 :Int32; 
  totalCapacity @7 :Int16; # total vehicle capacity for the trip
  seatedCapacity @8 :Int16; # total seated capacity for this trip
  allowSameLineTransfers @9 :Int8;
}