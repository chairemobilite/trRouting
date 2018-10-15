@0x8ffad6ff4ac3568a;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("odTripsCollection");

struct OdTripsCollection {
  odTrips @0 :List(OdTrip);
}

struct OdTrip {
  id @0 :Int64;
  personId @1 :Int64;
  householdId @2 :Int64;
  age @3 :Int64;
  departureTimeSeconds @4 :Int64;
  arrivalTimeSeconds @5 :Int64;
  walkingTravelTimeSeconds @6 :Int64;
  cyclingTravelTimeSeconds @7 :Int64;
  drivingTravelTimeSeconds @8 :Int64;
  expansionFactor @9 :Float32;
  
  ageGroup @10 :Text;
  gender @11 :Text;
  mode @12 :Text;
  occupation @13 :Text;
  originActivity @14 :Text;
  destinationActivity @15 :Text;

  accessFootpathsStartIdx @16 :Int64;
  accessFootpathsEndIdx @17 :Int64;
  egressFootpathsStartIdx @18 :Int64;
  egressFootpathsEndIdx @19 :Int64;

  originLatitude @20 :Float32;
  originLongitude @21 :Float32;
  destinationLatitude @22 :Float32;
  destinationLongitude @23 :Float32;
  homeLatitude @24 :Float32;
  homeLongitude @25 :Float32;
}