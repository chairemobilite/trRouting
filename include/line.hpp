#ifndef TR_LINE
#define TR_LINE

#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace TrRouting
{

  class Mode;
  class Agency;
  
  class Line {
  
  public:

    typedef int uid_t; //Type for a local temporary ID

    Line(const boost::uuids::uuid &auuid,
         const Agency &aagency,
         const Mode &amode,
         const std::string &ashortname,
         const std::string &alongname,
         const std::string &ainternalId,
         short aallowSameLineTransfers):
      uuid(auuid),
      agency(aagency),
      mode(amode),
      shortname(ashortname),
      longname(alongname),
      internalId(ainternalId),
      allowSameLineTransfers(aallowSameLineTransfers),
      uid(++global_uid) {}

    boost::uuids::uuid uuid;
    const Agency &agency;
    const Mode &mode;
    std::string shortname;
    std::string longname;
    std::string internalId;
    short allowSameLineTransfers;
    uid_t uid; //Local, temporary unique id, used to speed up lookups

    const std::string toString() {
      return "Line " + boost::uuids::to_string(uuid) + "\n  shortname " + shortname + "\n  longname " + longname;
    }

    // Equal operator. We only compare the uid, since they should be unique.
    inline bool operator==(const Line& other ) const { return uid == other.uid; }
    inline bool operator<(const Line& other ) const { return uid < other.uid; }
    inline bool operator!=(const Line& other ) const { return uid != other.uid; }

    static uid_t getMaxUid() { return global_uid; }

  private:
    //TODO, this could probably be an unsigned long, but current MAX_INT is good enough for our needs
    inline static uid_t global_uid = 0;
  };

  // To use std::find with a vector<reference_wrapper<const Line>>
  inline bool operator==(const std::reference_wrapper<const TrRouting::Line>& lhs, const Line& rhs)
  {
    return lhs.get() == rhs;
  }

  inline bool operator==(const std::reference_wrapper<const TrRouting::Line>& lhs, const std::reference_wrapper<const Line>& rhs)
  {
    return lhs.get() == rhs.get();
  }
  // For sorting and std::map usage
  inline bool operator<(const std::reference_wrapper<const TrRouting::Line>& lhs, const std::reference_wrapper<const Line>& rhs)
  {
    return lhs.get().uid < rhs.get().uid;
  }
}

#endif // TR_LINE
