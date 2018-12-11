@0xd86f051f59fb1b95;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("servicesCollection");

struct servicesCollection {
  services @0 :List(Service);
}

struct Service {
  id @0 :Int32;
  uuid @1 :Text;
  name @2 :Text;
  monday @3 :Int8;
  tuesday @4 :Int8;
  wednesday @5 :Int8;
  thursday @6 :Int8;
  friday @7 :Int8;
  saturday @8 :Int8;
  sunday @9 :Int8;
  start_date @10 :Date;
  end_date @11 :Date;
  only_dates @12 :List(:Date);
  except_dates @13 :List(:Date);
}

