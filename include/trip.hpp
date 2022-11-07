#ifndef TR_TRIP
#define TR_TRIP

#include <boost/uuid/uuid.hpp>
#include <vector>
#include "toolbox.hpp" //MAX_INT

namespace TrRouting
{
  class Mode;
  class Agency;
  class Service;
  class Path;
  
  class Trip {
  
  public:
    typedef int uid_t; //Type for a local temporary ID

    Trip( boost::uuids::uuid auuid,
          const Agency &aagency,
          const Line &aline,
          const Path &apath,
          const Mode &amode,
          const Service &aservice,
          short aallowSameLineTransfers,
          int atotalCapacity = -1,
          int aseatedCapacity = -1): uuid(auuid),
                                     agency(aagency),
                                     line(aline),
                                     path(apath),
                                     mode(amode),
                                     service(aservice),
                                     allowSameLineTransfers(aallowSameLineTransfers),
                                     totalCapacity(atotalCapacity),
                                     seatedCapacity(aseatedCapacity),
                                     uid(++global_uid) {}
   
    boost::uuids::uuid uuid;
    const Agency &agency; //TODO Agency is part of line, we could merge them
    const Line &line;
    const Path &path;
    const Mode &mode; //TODO Mode is part of Line, we could merge them
    const Service &service;
    short allowSameLineTransfers;
    int totalCapacity; //Unused
    int seatedCapacity; //Unused
    std::vector<int> forwardConnectionsIdx;
    std::vector<int> reverseConnectionsIdx;
    std::vector<int>   connectionDepartureTimes; // tripIndex: [connectionIndex (sequence in trip): departureTimeSeconds]

    uid_t uid; //Local, temporary unique id, used to speed up lookups

    inline bool operator==(const Trip& other ) const { return uuid == other.uuid; }
  private:
    //TODO, this could probably be an unsigned long, but current MAX_INT is good enough for our needs
    inline static uid_t global_uid = 0;    
  };
  inline bool operator==(const std::reference_wrapper<const TrRouting::Trip>& lhs, const std::reference_wrapper<const Trip>& rhs)
  {
    return lhs.get() == rhs.get();
  }
  // Scratch data used by the calculation of each query
  class TripQueryData {
  public:

    TripQueryData()
      : usable(false),
        enterConnection(-1),
        enterConnectionTransferTravelTime(MAX_INT),
        exitConnection(-1),
        exitConnectionTransferTravelTime(MAX_INT)
    {

    }
    bool usable; // after forward calculation, keep a list of usable trips in time range for reverse calculation
    int enterConnection; // index of the entering connection
    int enterConnectionTransferTravelTime;
    int exitConnection; // index of the exiting connection
    int exitConnectionTransferTravelTime;
  };

}
#endif // TR_TRIP
