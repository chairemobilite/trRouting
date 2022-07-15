#ifndef TR_SCENARIO
#define TR_SCENARIO

#include <vector>
#include <string>
#include <boost/uuid/uuid.hpp>

namespace TrRouting
{
  class Mode;
  
  struct Scenario {
  
  public:
   
    boost::uuids::uuid uuid;
    std::string name;
    boost::uuids::uuid simulationUuid;
    std::vector<int> servicesIdx;
    std::vector<std::reference_wrapper<const Mode>> onlyModes;
    std::vector<int> onlyLinesIdx;
    std::vector<int> onlyAgenciesIdx;
    std::vector<int> onlyNodesIdx;
    std::vector<std::reference_wrapper<const Mode>> exceptModes;
    std::vector<int> exceptLinesIdx;
    std::vector<int> exceptAgenciesIdx;
    std::vector<int> exceptNodesIdx;

    const std::string toString() {
      return "Scenario " + boost::uuids::to_string(uuid) + "\n  name " + name;
    }

  };

}

#endif // TR_SCENARIO
