@0xf9b31db38bb1e2ed;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("personsCollection");

using Person = import "person.capnp".Person;

struct PersonsCollection {
  persons @0 :List(Person);
}

