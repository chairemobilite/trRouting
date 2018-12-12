@0xf4952682718a13e5;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("pathsCollection");

struct PathsCollection {
  paths @0 :List(Path);
}

struct Path {
  uuid @0 :Text;
  direction @1 :Text;
  name @2 :Text;
  lineIdx @3 :Int32; # unique for the whole network, from LinesCollection index
}