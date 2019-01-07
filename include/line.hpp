#ifndef TR_LINE
#define TR_LINE

#include <string>
#include <boost/uuid/uuid.hpp>

namespace TrRouting
{
  
  struct Line {
  
  public:
   
    boost::uuids::uuid uuid;
    int agencyIdx;
    int modeIdx;
    std::string shortname;
    std::string longname;
    short allowSameLineTransfers;

  };

}

#endif // TR_LINE
