#ifndef TR_TRIP
#define TR_TRIP

#include <boost/uuid/uuid.hpp>
#include <vector>

namespace TrRouting
{
  class Mode;
  
  struct Trip {
  
  public:
    Trip( boost::uuids::uuid auuid,
          int aagencyIdx,
          int alineIdx,
          int apathIdx,
          const Mode &amode,
          int aserviceIdx,
          int ablockIdx,
          short aallowSameLineTransfers,
          int atotalCapacity = -1,
          int aseatedCapacity = -1): uuid(auuid),
                                         agencyIdx(aagencyIdx),
                                         lineIdx(alineIdx),
                                         pathIdx(apathIdx),
                                         mode(amode),
                                         serviceIdx(aserviceIdx),
                                         blockIdx(ablockIdx),
                                         totalCapacity(atotalCapacity),
                                         seatedCapacity(aseatedCapacity),
                                         allowSameLineTransfers(aallowSameLineTransfers) {}
   
    boost::uuids::uuid uuid;
    int agencyIdx;
    int lineIdx;
    int pathIdx;
    const Mode &mode;
    int serviceIdx;
    int blockIdx;
    short allowSameLineTransfers;
    int totalCapacity; //Unused
    int seatedCapacity; //Unused
    std::vector<int> forwardConnectionsIdx;
    std::vector<int> reverseConnectionsIdx;

  };

}

#endif // TR_TRIP
