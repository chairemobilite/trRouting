@0x8ffad6ff4ac3568a;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("odTripsCollection");

using OdTrip = import "odTrip.capnp".OdTrip;

struct OdTripsCollection {
  odTrips @0 :List(OdTrip);
}

