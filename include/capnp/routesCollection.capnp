@0xd06a46d775e4e446;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("routesCollection");

struct RoutesCollection {
  routes @0 :List(Route);
}

struct Route {
  id @0 :Int64;
  agencyId @1 :Int64;
  routeTypeId @2 :Int64;
  agencyAcronym @3 :Text;
  agencyName @4 :Text;
  shortname @5 :Text;
  longname @6 :Text;
  routeTypeName @7 :Text;
}
