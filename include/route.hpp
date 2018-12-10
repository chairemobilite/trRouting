#ifndef TR_ROUTE
#define TR_ROUTE

#include <boost/serialization/access.hpp>

namespace TrRouting
{
  
  struct Route {
  
  public:
   
    unsigned long long id;
    unsigned long long agencyId;
    unsigned long long routeTypeId;
    std::string agencyAcronym;
    std::string agencyName;
    std::string shortname;
    std::string longname;
    std::string routeTypeName;
  
  };

}

#endif // TR_ROUTE
