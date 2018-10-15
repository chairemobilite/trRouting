@0x9ae8c15ade8bb26f;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("tripsCollection");

struct TripsCollection {
  trips @0 :List(Trip);
}

struct Trip {
  id @0 :Int64;
  routeId @1 :Int64;
  routePathId @2 :Int64;
  routeTypeId @3 :Int64;
  agencyId @4 :Int64;
  serviceId @5 :Int64;
}