@0x88340494115f60b3;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("odTrip");

struct OdTrip {

  uuid                        @0  :Text;
  personUuid                  @1  :Text;
  householdUuid               @2  :Text;
  dataSourceUuid              @3  :Text;
  id                          @4  :UInt32; # unique per data source
  expansionFactor             @5  :Float32;
  departureTimeSeconds        @6  :Int32;
  arrivalTimeSeconds          @7  :Int32;
  mode                        @8  :Mode;
  originActivity              @9  :Activity;
  destinationActivity         @10 :Activity;
  originLatitude              @11 :Int32; # divide by 1000000 to get float
  originLongitude             @12 :Int32; # divide by 1000000 to get float
  destinationLatitude         @13 :Int32; # divide by 1000000 to get float
  destinationLongitude        @14 :Int32; # divide by 1000000 to get float
  originNodesUuids            @15 :List(Text); # unique in the whole network, changed to indexes in collection
  originNodesTravelTimes      @16 :List(Int32); # seconds
  originNodesDistances        @17 :List(Int32); # meters
  destinationNodesUuids       @18 :List(Text); # unique in the whole network, changed to indexes in collection
  destinationNodesTravelTimes @19 :List(Int32); # seconds
  destinationNodesDistances   @20 :List(Int32); # meters
  internalId                  @21 :Text;
  data                        @22 :Text; # json

  enum Mode {
    none            @0 ;
    walking         @1 ;
    cycling         @2 ;
    carDriver       @3 ;
    carPassenger    @4 ;
    motorcycle      @5 ;
    transit         @6 ;
    paratransit     @7 ;
    taxi            @8 ;
    schoolBus       @9 ;
    otherBus        @10;
    intercityBus    @11;
    intercityTrain  @12;
    plane           @13;
    ferry           @14;
    parkAndRide     @15;
    kissAndRide     @16;
    bikeAndRide     @17;
    multimodalOther @18;
    other           @19;
    unknown         @20;
  }

  enum Activity {
    none            @0;
    home            @1;
    workUsual       @2;
    workNonUsual    @3;
    schoolUsual     @4;
    schoolNonUsual  @5;
    shopping        @6;
    leisure         @7;
    service         @8;
    secondaryHome   @9;
    visitingFriends @10;
    dropSomeone     @11;
    fetchSomeone    @12;
    restaurant      @13;
    medical         @14;
    worship         @15;
    onTheRoad       @16;
    other           @17;
    unknown         @18;
  }

}