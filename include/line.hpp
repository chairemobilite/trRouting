#ifndef TR_LINE
#define TR_LINE

#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace TrRouting
{

  class Mode;
  
  struct Line {
  
  public:
    Line(const boost::uuids::uuid &auuid,
         int aagencyIdx,
         const Mode &amode,
         const std::string &ashortname,
         const std::string &alongname,
         const std::string &ainternalId,
         short aallowSameLineTransfers):
      uuid(auuid),
      agencyIdx(aagencyIdx),
      mode(amode),
      shortname(ashortname),
      longname(alongname),
      internalId(ainternalId),
      allowSameLineTransfers(aallowSameLineTransfers) {}

    boost::uuids::uuid uuid;
    int agencyIdx;
    const Mode &mode;
    std::string shortname;
    std::string longname;
    std::string internalId;
    short allowSameLineTransfers;

    const std::string toString() {
      return "Line " + boost::uuids::to_string(uuid) + "\n  shortname " + shortname + "\n  longname " + longname;
    }

  };

}

#endif // TR_LINE
