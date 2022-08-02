#ifndef TR_SCENARIO
#define TR_SCENARIO

#include <vector>
#include <string>
#include <boost/uuid/uuid.hpp>

namespace TrRouting
{
  class Mode;
  class Agency;
  class Service;
  
  struct Scenario {
  
  public:
   
    boost::uuids::uuid uuid;
    std::string name;
    boost::uuids::uuid simulationUuid;
    std::vector<std::reference_wrapper<const Service>> servicesList;
    std::vector<std::reference_wrapper<const Mode>> onlyModes;
    std::vector<std::reference_wrapper<const Line>> onlyLines;
    std::vector<std::reference_wrapper<const Agency>> onlyAgencies;
    std::vector<int> onlyNodesIdx;
    std::vector<std::reference_wrapper<const Mode>> exceptModes;
    std::vector<std::reference_wrapper<const Line>> exceptLines;
    std::vector<std::reference_wrapper<const Agency>> exceptAgencies;
    std::vector<int> exceptNodesIdx;

    const std::string toString() {
      return "Scenario " + boost::uuids::to_string(uuid) + "\n  name " + name;
    }

  };

}

#endif // TR_SCENARIO
