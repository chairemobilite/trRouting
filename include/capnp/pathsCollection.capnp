@0xdd621ca05dc0a0e5;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("pathsCollection");

struct pathsCollection {
  paths @0 :List(Path);
}

struct Path {
  id @0 :Int32;
  uuid @1 :Text;
  direction @2 :Text;
  name @3 :Text;
}

