@0xff526f818562f0b2;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("person");

struct Person {
  
  uuid                             @0  :Text;
  householdUuid                    @1  :Text;
  id                               @2  :UInt32;
  expansionFactor                  @3  :Float32;
  age                              @4  :Int16;
  drivingLicenseOwner              @5  :Int8;
  transitPassOwner                 @6  :Int8;
  ageGroup                         @7  :AgeGroup;
  gender                           @8  :Gender;
  occupation                       @9  :Occupation;
  usualWorkPlaceLatitude           @10 :Float32;
  usualWorkPlaceLongitude          @11 :Float32;
  usualschoolPlaceLatitude         @12 :Float32;
  usualschoolPlaceLongitude        @13 :Float32;
  usualWorkPlaceNodesUuids         @14 :List(Text); # unique in the whole network, changed to indexes in collection
  usualWorkPlaceNodesTravelTimes   @15 :List(Int32); # seconds
  usualWorkPlaceNodesDistances     @16 :List(Int32); # meters
  usualSchoolPlaceNodesUuids       @17 :List(Text); # unique in the whole network, changed to indexes in collection
  usualSchoolPlaceNodesTravelTimes @18 :List(Int32); # seconds
  usualSchoolPlaceNodesDistances   @19 :List(Int32); # meters

  enum AgeGroup {
    ag0004   @0;
    ag0509   @1;
    ag1014   @2;
    ag1519   @3;
    ag2024   @4;
    ag2529   @5;
    ag3034   @6;
    ag3539   @7;
    ag4044   @8;
    ag4549   @9;
    ag5054   @10;
    ag5559   @11;
    ag6064   @12;
    ag6569   @13;
    ag7074   @14;
    ag7579   @15;
    ag8084   @16;
    ag8589   @17;
    ag9094   @18;
    ag95plus @19;
    unknown  @20;
  }

  enum Gender {
    female  @0;
    male    @1;
    custom  @2;
    unknown @3;
  }

  enum Occupation {
    fullTimeWorker   @0;
    partTimeWorker   @1;
    fullTimeStudent  @2;
    partTimeStudent  @3;
    workerAndStudent @4;
    retired          @5;
    atHome           @6;
    other            @7;
    nonApplicable    @8;
    unknown          @9;
  }

}