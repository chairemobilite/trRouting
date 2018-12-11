@0xe72037944c35561f;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("nodeTimesCollection");

struct nodeTimesCollection {
  nodeTimes @0 :List(NodeTime);
}

struct NodeTime {
  nodeIdx @0 :Int32;
  timeArr @1 :Int32;
  timeDep @2 :Int32;
  tripIdx @3 :Int32;
  canUnboard @4 :Int8;
  canBoard @5 :Int8;
  sequence @6 :Int32;
}