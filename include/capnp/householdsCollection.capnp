@0xfa6a69e9ccb7fa72;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("householdsCollection");

using Household = import "household.capnp".Household;

struct HouseholdsCollection {
  households @0 :List(Household);
}

