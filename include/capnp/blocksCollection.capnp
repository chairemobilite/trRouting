@0xb2d8c9a9938edfdf;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("blocksCollection");

struct BlockCollection {
  blocks @0 :List(Block);
}

struct Block {
  uuid @0 :Text;
  id @1 :Int64;  # unique for the whole network 
  name @2 :Text;
}

