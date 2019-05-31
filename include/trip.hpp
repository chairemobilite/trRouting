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
    short allowSameLineTransfers;

  };

}

#endif // TR_TRIP