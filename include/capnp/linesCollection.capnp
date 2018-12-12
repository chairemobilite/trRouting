@0xcdd4740831f7524a;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("linesCollection");

struct LinesCollection {
  nodes @0 :List(Line);
}

struct Line {
  uuid @0 :Text;
  agencyUuid @1 :Text;
  mode @2 :Text;
  shortname @3 :Text;
  longname @4 :Text;
  allowSameLineTransfers @5 :Int8;
}
