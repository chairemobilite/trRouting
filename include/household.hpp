#ifndef TR_HOUSEHOLD
#define TR_HOUSEHOLD

#include <vector>
#include <string>
#include <boost/uuid/uuid.hpp>
#include "point.hpp"


namespace TrRouting
{
  
  class Household {
  
  public:
   
    boost::uuids::uuid uuid;
    unsigned long long id;
    float expansionFactor;
    int size;
    int carNumber;
    int incomeLevel;
    std::string incomeLevelGroup;
    std::string category;
    std::string internalId;
    std::unique_ptr<Point> point;
    std::vector<int> homeNodesIdx;
    std::vector<int> homeNodesTravelTimesSeconds;
    std::vector<int> homeNodesDistancesMeters;

    const std::string toString() {
      return "Household " + boost::uuids::to_string(uuid) + " size " + std::to_string(size);
    }

  };

}

#endif // TR_HOUSEHOLD
