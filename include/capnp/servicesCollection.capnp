@0xd86f051f59fb1b95;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("servicesCollection");

struct ServicesCollection {
  services @0 :List(Service);
}

struct Service {
  uuid @0 :Text;
  name @1 :Text;
  monday @2 :Int8;
  tuesday @3 :Int8;
  wednesday @4 :Int8;
  thursday @5 :Int8;
  friday @6 :Int8;
  saturday @7 :Int8;
  sunday @8 :Int8;
  startDate @9 :Text; # YYYY-MM-DD
  endDate @10 :Text; # YYYY-MM-DD
  onlyDates @11 :List(Text); # YYYY-MM-DD
  exceptDates @12 :List(Text); # YYYY-MM-DD
}

