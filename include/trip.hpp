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
    std::vector<std::shared_ptr<std::tuple<int,int,int,int,int,short,short,int,int,int,short,short>>> forwardConnections;
    std::vector<std::shared_ptr<std::tuple<int,int,int,int,int,short,short,int,int,int,short,short>>> reverseConnections;

  };

}

#endif // TR_TRIP