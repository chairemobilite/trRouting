#ifndef TR_TRIP
#define TR_TRIP

#include <boost/uuid/uuid.hpp>
#include <vector>

namespace TrRouting
{
  class Mode;
  class Agency;
  class Service;
  
  struct Trip {
  
  public:
    Trip( boost::uuids::uuid auuid,
          const Agency &aagency,
          int alineIdx,
          int apathIdx,
          const Mode &amode,
          const Service &aservice,
          short aallowSameLineTransfers,
          int atotalCapacity = -1,
          int aseatedCapacity = -1): uuid(auuid),
                                         agency(aagency),
                                         lineIdx(alineIdx),
                                         pathIdx(apathIdx),
                                         mode(amode),
                                         service(aservice),
                                         totalCapacity(atotalCapacity),
                                         seatedCapacity(aseatedCapacity),
                                         allowSameLineTransfers(aallowSameLineTransfers) {}
   
    boost::uuids::uuid uuid;
    const Agency &agency;
    int lineIdx;
    int pathIdx;
    const Mode &mode;
    const Service &service;
    short allowSameLineTransfers;
    int totalCapacity; //Unused
    int seatedCapacity; //Unused
    std::vector<int> forwardConnectionsIdx;
    std::vector<int> reverseConnectionsIdx;

  };

}

#endif // TR_TRIP
