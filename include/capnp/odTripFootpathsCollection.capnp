@0x964450da4385f299;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("odTripFootpathsCollection");

struct OdTripFootpathsCollection {
  odTripFootpaths @0 :List(OdTripFootpath);
  odTripFootpathRanges @1 :List(OdTripFootpathRange);
}

struct OdTripFootpath {
  nodeIdx @0 :Int32;
  travelTime @1 :Int32;
}

struct OdTripFootpathRange {
  footpathsStartIdx @0 :Int32;
  footpathsEndIdx @1 :Int32;
}

