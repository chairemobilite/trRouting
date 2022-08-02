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
      allowSameLineTransfers(aallowSameLineTransfers) {}

    boost::uuids::uuid uuid;
    const Agency &agency;
    const Mode &mode;
    std::string shortname;
    std::string longname;
    std::string internalId;
    short allowSameLineTransfers;

    const std::string toString() {
      return "Line " + boost::uuids::to_string(uuid) + "\n  shortname " + shortname + "\n  longname " + longname;
    }

    // Equal operator. We only compare the uuid, since they should be unique.
    inline bool operator==(const Line& other ) const { return uuid == other.uuid; }
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
    return lhs.get().uuid < rhs.get().uuid;
  }
}

#endif // TR_LINE
