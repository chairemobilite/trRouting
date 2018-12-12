@0xd06a46d775e4e446;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("linesCollection");

struct LinesCollection {
  lines @0 :List(Line);
}

struct Line {
  uuid @0 :Text;
  agencyIdx @1 :Int32;
  modeIdx @2 :Int32;
  shortname @3 :Text;
  longname @4 :Text;
}
