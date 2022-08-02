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
          const Line &aline,
          int apathIdx,
          const Mode &amode,
          const Service &aservice,
          short aallowSameLineTransfers,
          int atotalCapacity = -1,
          int aseatedCapacity = -1): uuid(auuid),
                                         agency(aagency),
                                         line(aline),
                                         pathIdx(apathIdx),
                                         mode(amode),
                                         service(aservice),
                                         totalCapacity(atotalCapacity),
                                         seatedCapacity(aseatedCapacity),
                                         allowSameLineTransfers(aallowSameLineTransfers) {}
   
    boost::uuids::uuid uuid;
    const Agency &agency; //TODO Agency is part of line, we could merge them
    const Line &line;
    int pathIdx;
    const Mode &mode; //TODO Mode is part of Line, we could merge them
    const Service &service;
    short allowSameLineTransfers;
    int totalCapacity; //Unused
    int seatedCapacity; //Unused
    std::vector<int> forwardConnectionsIdx;
    std::vector<int> reverseConnectionsIdx;

  };

}

#endif // TR_TRIP
