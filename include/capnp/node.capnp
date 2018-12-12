@0xd68f205b1747bdab;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("nodesCollection");

struct NodesCollection {
  nodes @0 :List(Node);
}

struct Node {
  uuid @0 :Text;
  id @1 :Int64; # unique for the whole network
  code @2 :Text;
  name @3 :Text;
  latitude @4 :Float32;
  longitude @5 :Float32;
  stationUuid @6 :Text;
  transferableNodesIdx @7 :List(Int32);  # unique in the whole network
  transferableNodesTravelTimes @8 :List(Int32); # seconds
}