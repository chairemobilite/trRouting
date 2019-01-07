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

  };

}

#endif // TR_SCENARIO
