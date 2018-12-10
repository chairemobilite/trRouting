@0xdcbcd5808d363f36;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("footpathsCollection");

struct FootpathsCollection {
  footpaths @0 :List(Footpath);
  footpathRanges @1 :List(FootpathRange);
}

struct Footpath {
  stop1Idx @0 :Int32;
  stop2Idx @1 :Int32;
  travelTime @2 :Int32;
}

struct FootpathRange {
  footpathsStartIdx @0 :Int32;
  footpathsEndIdx @1 :Int32;
}

