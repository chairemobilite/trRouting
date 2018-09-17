#ifndef TR_ROUTE_PATH
#define TR_ROUTE_PATH

namespace TrRouting
{
  
  struct RoutePath {
    
    int i;           // route path index
    long long id;    // route path id
    long long rId;   // route path route id
    std::string rSn;   // route shortname
    std::string rLn;   // route longname
    long long aId;   // agency id
    std::string aName; // agency name
  };

}

#endif // TR_ROUTE_PATH
