@0x8c317b06a598b048;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("connectionsCollection");

struct ConnectionsCollection {
  forwardConnections @0 :List(Connection);
  reverseConnections @1 :List(Connection);
}

struct Connection {
  nodeDepIdx @0 :Int32; # unique in the whole network, NodesCollection index
  nodeArrIdx @1 :Int32; # unique in the whole network, NodesCollection index
  timeDep @2 :Int32; # seconds after midnight
  timeArr @3 :Int32; # seconds after midnight
  tripIdx @4 :Int32; # unique in the whole network, TripsCollection index
  canBoard @5 :Int8;
  canUnboard @6 :Int8;
  sequence @7 :Int16;
}