@0x88340494115f60b3;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("odTrip");

struct OdTrip {

  uuid                        @0  :Text;
  personUuid                  @1  :Text;
  id                          @2  :UInt32;
  expansionFactor             @3  :Float32;
  departureTimeSeconds        @4  :Int32;
  arrivalTimeSeconds          @5  :Int32;
  mode                        @6  :Mode;
  originActivity              @7  :Activity;
  destinationActivity         @8  :Activity;
  originLatitude              @9  :Float32;
  originLongitude             @10 :Float32;
  destinationLatitude         @11 :Float32;
  destinationLongitude        @12 :Float32;
  originNodesUuids            @13 :List(Text); # unique in the whole network, changed to indexes in collection
  originNodesTravelTimes      @14 :List(Int32); # seconds
  originNodesDistances        @15 :List(Int32); # meters
  destinationNodesUuids       @16 :List(Text); # unique in the whole network, changed to indexes in collection
  destinationNodesTravelTimes @17 :List(Int32); # seconds
  destinationNodesDistances   @18 :List(Int32); # meters

  enum Mode {
    walking         @0;
    cycling         @1;
    carDriver       @2;
    carPassenger    @3;
    motorcycle      @4;
    transit         @5;
    paratransit     @6;
    taxi            @7;
    schoolBus       @8;
    otherBus        @9;
    intercityBus    @10;
    intercityTrain  @11;
    plane           @12;
    ferry           @13;
    parkAndRide     @14;
    kissAndRide     @15;
    bikeAndRide     @16;
    multimodalOther @17;
    other           @18;
    unknown         @19;
  }

  enum Activity {
    home            @0;
    workUsual       @1;
    workNonUsual    @2;
    schoolUsual     @3;
    schoolNonUsual  @4;
    shopping        @5;
    leisure         @6;
    service         @7;
    secondaryHome   @8;
    visitingFriends @9;
    dropSomeone     @10;
    fetchSomeone    @11;
    restaurant      @12;
    medical         @13;
    worship         @14;
    onTheRoad       @15;
    other           @16;
    unknown         @17;
  }

}