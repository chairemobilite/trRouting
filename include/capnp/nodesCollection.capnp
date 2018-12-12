@0xd68f205b1747bdab;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("nodesCollection");

struct NodesCollection {
  nodes @0 :List(Node);
}

struct Node {
  uuid @0 :Text;
  code @1 :Text;
  name @2 :Text;
  latitude @3 :Float32;
  longitude @4 :Float32;
  stationIdx @5 :Int32;
}