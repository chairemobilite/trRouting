@0xdd621ca05dc0a0e5;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("pathsCollection");

struct PathsCollection {
  paths @0 :List(Path);
}

struct Path {
  uuid @0 :Text;
  lineIdx @1 :Int32;
  direction @2 :Text;
  name @3 :Text;
}

