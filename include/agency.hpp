#ifndef TR_AGENCY
#define TR_AGENCY

#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace TrRouting
{
  
  struct Agency {
  
  public:
   
    boost::uuids::uuid uuid;
    std::string acronym;
    std::string name;

    const std::string toString() {
      return "Agency " + boost::uuids::to_string(uuid) + "\n  acronym " + acronym + "\n  name " + name;
    }

  };

}

#endif // TR_AGENCY
