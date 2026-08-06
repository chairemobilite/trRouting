#ifndef TR_TRIP
#define TR_TRIP

#include <boost/uuid/uuid.hpp>
#include <vector>
#include <optional>
#include "toolbox.hpp" //MAX_INT

namespace TrRouting
{
  class Mode;
  class Agency;
  class Service;
  class Path;
  class Connection;
  
  class Trip {
  
  public:
    typedef int uid_t; //Type for a local temporary ID

    Trip( boost::uuids::uuid auuid,
          const Agency &aagency,
          const Line &aline,
          const Path &apath,
          const Mode &amode,
          const Service &aservice,
          short aallowSameLineTransfers): uuid(auuid),
                                     agency(aagency),
                                     line(aline),
                                     path(apath),
                                     mode(amode),
                                     service(aservice),
                                     allowSameLineTransfers(aallowSameLineTransfers),
                                     uid(++global_uid) {}
   
    boost::uuids::uuid uuid;
    const Agency &agency; //TODO Agency is part of line, we could merge them
    const Line &line;
    const Path &path;
    const Mode &mode; //TODO Mode is part of Line, we could merge them
    const Service &service;
    short allowSameLineTransfers;
    std::vector<std::reference_wrapper<const Connection>> forwardConnections;
    std::vector<std::reference_wrapper<const Connection>> reverseConnections;

    uid_t uid; //Local, temporary unique id, used to speed up lookups

    // Equal operator. We only compare the uid, since they are unique.
    inline bool operator==(const Trip& other ) const { return uid == other.uid; }
    inline bool operator<(const Trip& other ) const { return uid < other.uid; }
    inline bool operator!=(const Trip& other ) const { return uid != other.uid; }

    static uid_t getMaxUid() { return global_uid; }
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
        enterConnection(std::nullopt),
        enterConnectionTransferTravelTime(MAX_INT),
        exitConnection(std::nullopt),
        exitConnectionTransferTravelTime(MAX_INT)
    {

    }
    bool usable; // after forward calculation, keep a list of usable trips in time range for reverse calculation
    std::optional<std::reference_wrapper<const Connection>> enterConnection; // index of the entering connection
    int enterConnectionTransferTravelTime;
    std::optional<std::reference_wrapper<const Connection>> exitConnection; // index of the exiting connection
    int exitConnectionTransferTravelTime;
  };

}
#endif // TR_TRIP
