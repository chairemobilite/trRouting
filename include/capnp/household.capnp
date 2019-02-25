@0xa6587c9eb014ab3e;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("household");

struct Household {
  
  uuid                 @0  :Text;
  id                   @1  :UInt32;
  expansionFactor      @2  :Float32;
  size                 @3  :Int8;
  carNumber            @4  :Int8;
  incomeLevel          @5  :Int32;
  incomeLevelGroup     @6  :IncomeLevelGroup;
  category             @7  :Category;
  homeLatitude         @8  :Float32;
  homeLongitude        @9  :Float32;
  homeNodesUuids       @10 :List(Text); # unique in the whole network, changed to indexes in collection
  homeNodesTravelTimes @11 :List(Int32); # seconds
  homeNodesDistances   @12 :List(Int32); # meters

  enum IncomeLevelGroup {
    veryLow  @0;
    low      @1;
    medium   @2;
    high     @3;
    veryHigh @4;
    unknown  @5;
  }

  enum Category {
    singlePerson       @0;
    couple             @1;
    monoparentalFamily @2;
    biparentalFamily   @3;
    other              @4;
    unknown            @5;
  }

}