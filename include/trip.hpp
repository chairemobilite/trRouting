#ifndef TR_TRIP
#define TR_TRIP

#include <boost/uuid/uuid.hpp>
#include <vector>

namespace TrRouting
{
  
  struct Trip {
  
  public:
   
    boost::uuids::uuid uuid;
    int agencyIdx;
    int lineIdx;
    int pathIdx;
    int modeIdx;
    int serviceIdx;
    int blockIdx;
    int totalCapacity;
    int seatedCapacity;
    std::vector<int> loadBySegment; // must be reset before each batch routing

  };

}

#endif // TR_TRIP