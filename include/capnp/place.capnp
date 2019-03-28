@0xd46f5ecd107c0374;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("place");

struct Place {
  
  uuid             @0  :Text;
  dataSourceUuid   @1  :Text;
  id               @2  :UInt32;
  shortname        @3  :Text;
  name             @4  :Text;
  osmFeatureKey    @5  :Text; # see https://wiki.openstreetmap.org/wiki/Map_Features
  osmFeatureValue  @6  :Text; # see https://wiki.openstreetmap.org/wiki/Map_Features
  description      @7  :Text;
  latitude         @8  :Int32; # divide by 1000000 to get float
  longitude        @9  :Int32; # divide by 1000000 to get float
  nodesIdx         @10 :List(Int32);
  nodesTravelTimes @11 :List(Int16); # seconds
  nodesDistances   @12 :List(Int16); # meters
  internalId       @13 :Text;
  data             @14 :Text; # json
  isFrozen         @15 :Int8;

}