#ifndef TR_PLACE
#define TR_PLACE

#include <vector>
#include <string>
#include <boost/uuid/uuid.hpp>
#include "point.hpp"


namespace TrRouting
{
  
  struct Place {
  
  public:
   
    boost::uuids::uuid uuid;
    unsigned long long id;
    std::string shortname;
    std::string name;
    std::string osmFeatureKey;
    std::string osmFeatureValue;
    std::string internalId;
    std::unique_ptr<Point> point;
    std::vector<int> nodesIdx;
    std::vector<int> nodesTravelTimesSeconds;
    std::vector<int> nodesDistancesMeters;

    const std::string toString() {
      return "Place " + boost::uuids::to_string(uuid);
    }
    
  };

}

#endif // TR_PLACE
