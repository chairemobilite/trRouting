@0xb2d8c9a9938edfdf;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("blocksCollection");

struct BlocksCollection {
  blocks @0 :List(Block);
}

struct Block {
  uuid @0 :Text;
  name @1 :Text;
}

