@0xd68f205b1747bdab;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("node");

struct Node {
  uuid @0 :Text;
  id @1 :Int64; # unique for the whole network
  code @2 :Text;
  name @3 :Text;
  latitude @4 :Float32;
  longitude @5 :Float32;
  stationUuid @6 :Text;
  transferableNodesIds @7 :List(Int64); # unique in the whole network, changed to indexes in collection
  transferableNodesTravelTimes @8 :List(Int32); # seconds
}