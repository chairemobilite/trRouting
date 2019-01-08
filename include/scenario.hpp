#ifndef TR_SCENARIO
#define TR_SCENARIO

#include <vector>
#include <string>
#include <boost/uuid/uuid.hpp>

namespace TrRouting
{
  
  struct Scenario {
  
  public:
   
    boost::uuids::uuid uuid;
    std::string name;
    std::vector<int> servicesIdx;

    const std::string toString() {
      return "Scenario " + boost::uuids::to_string(uuid) + "\n  name " + name;
    }

  };

}

#endif // TR_SCENARIO
