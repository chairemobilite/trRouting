#ifndef TR_AGENCY
#define TR_AGENCY

#include <string>
#include <boost/uuid/uuid.hpp>

namespace TrRouting
{
  
  struct Agency {
  
  public:
   
    boost::uuids::uuid uuid;
    std::string acronym;
    std::string name;
 
  };

}

#endif // TR_AGENCY
