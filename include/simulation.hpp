#ifndef TR_SIMULATION
#define TR_SIMULATION

#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace TrRouting
{
  
  struct Simulation {
    
  public:

    boost::uuids::uuid uuid;
    std::string shortname;
    std::string name;
    std::string internalId;

    const std::string toString() {
      return "Simulation " + boost::uuids::to_string(uuid) + "\n  shortname " + shortname + "\n  name " + name;
    }

  };

}

#endif // TR_SIMULATION
