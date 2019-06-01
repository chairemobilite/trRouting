#ifndef TR_NETWORK
#define TR_NETWORK

#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace TrRouting
{
  
  struct Network {
    
  public:

    boost::uuids::uuid uuid;
    std::string shortname;
    std::string name;
    std::string internalId;
    std::vector<int> agenciesIdx;
    std::vector<int> servicesIdx;
    std::vector<int> scenariosIdx;

    const std::string toString() {
      return "Network " + boost::uuids::to_string(uuid) + "\n  shortname " + shortname + "\n  name " + name;
    }

  };

}

#endif // TR_NETWORK
