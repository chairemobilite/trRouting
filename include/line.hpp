#ifndef TR_LINE
#define TR_LINE

#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace TrRouting
{
  
  struct Line {
  
  public:
   
    boost::uuids::uuid uuid;
    int agencyIdx;
    int modeIdx;
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
