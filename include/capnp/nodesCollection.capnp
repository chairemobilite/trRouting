0xd68f205b1747bdab;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("nodesCollection");

struct NodesCollection {
  nodes @0 :List(Node);
}

struct Node {
  id @0 :Int32;
  uuid @1 :Text;
  code @2 :Text;
  name @3 :Text;
  latitude @4 :Float32;
  longitude @5 :Float32;
  stationId @6 :Int32;
}