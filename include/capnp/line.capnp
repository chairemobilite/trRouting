@0xd06a46d775e4e446;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("line");

struct Line {
  uuid @0 :Text;
  agencyUuid @1 :Text;
  mode @2 :Text;
  shortname @3 :Text;
  longname @4 :Text;
  paths @5 :List(Path);
  allowSameLineTransfers @6 :Int8;
}

struct NodeTime {
  nodeId @0 :Int64; # unique for the whole network
  timeArr @1 :Int32; # seconds after midnight
  timeDep @2 :Int32; # seconds after midnight
  tripIdx @3 :Int32; # index in line trips list
  canUnboard @4 :Int8;
  canBoard @5 :Int8;
  sequence @6 :Int32;
}

struct Path {
  uuid @0 :Text;
  direction @1 :Text;
  name @2 :Text;
  trips @3 :List(Trip);
}

struct Trip {
  timeDep @0 :Int32; # seconds after midnight
  timeArr @1 :Int32; # seconds after midnight
  serviceUuid @2 :Text;
  scenarioUuid @3 :Text;
  blockUuid @4 :Text;
  totalCapacity @5 :Int16; # total vehicle capacity for the trip
  seatedCapacity @6 :Int16; # total seated capacity for this trip
  nodeTimes @7 :List(NodeTime);
}