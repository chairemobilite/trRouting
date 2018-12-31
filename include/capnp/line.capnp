@0xd06a46d775e4e446;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("line");

struct Trip {
  pathUuid @0 :Text;
  departureTimeSeconds @1 :Int32;
  arrivalTimeSeconds @2 :Int32;
  nodeArrivalTimesSeconds @3 :List(Int32);
  nodeDepartureTimesSeconds @4 :List(Int32);
  nodesCanBoard @5 :List(Int8);
  nodesCanUnboard @6 :List(Int8);
  blockUuid @7 :Text;
  totalCapacity @8 :Int16; # total vehicle capacity for the trip
  seatedCapacity @9 :Int16; # total seated capacity for this trip
}

struct Schedule {
  uuid @0 :Text;
  serviceUuid @1 :Text;
  trips @2 :List(Trip);
}

struct Line {
  uuid @0 :Text;
  internalId @1 :Text;
  mode @2 :Text;
  category @3 :Text;
  agencyUuid @4 :Text;
  shortname @5 :Text;
  longname @6 :Text;
  color @7 :Text;
  isEnabled @8 :Int8;
  description @9 :Text;
  data @10 :Text;
  isAutonomous @11 :Int8;
  allowSameLineTransfers @12 :Int8;
  schedules @13 :List(Schedule);
}
