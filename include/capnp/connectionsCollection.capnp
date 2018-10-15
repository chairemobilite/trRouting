@0x8c317b06a598b048;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("connectionsCollection");

struct ConnectionsCollection {
  forwardConnections @0 :List(Connection);
  reverseConnections @1 :List(Connection);
}

struct Connection {
  stopDepIdx @0 :Int32;
  stopArrIdx @1 :Int32;
  timeDep @2 :Int32;
  timeArr @3 :Int32;
  tripIdx @4 :Int32;
  canBoard @5 :Int8;
  canUnboard @6 :Int8;
  sequence @7 :Int32;
}