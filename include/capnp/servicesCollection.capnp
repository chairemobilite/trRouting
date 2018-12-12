@0xd86f051f59fb1b95;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("servicesCollection");

struct ServicesCollection {
  services @0 :List(Service);
}

struct Service {
  uuid @0 :Text;
  id @1 :Int64;  # unique for the whole network 
  name @2 :Text;
  monday @3 :Int8;
  tuesday @4 :Int8;
  wednesday @5 :Int8;
  thursday @6 :Int8;
  friday @7 :Int8;
  saturday @8 :Int8;
  sunday @9 :Int8;
  startDate @10 :Text; # YYYY-MM-DD
  endDate @11 :Text; # YYYY-MM-DD
  onlyDates @12 :List(Text); # YYYY-MM-DD
  exceptDates @13 :List(Text); # YYYY-MM-DD
}

