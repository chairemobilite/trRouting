#ifndef TR_ROUTE
#define TR_ROUTE

#include <vector>
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
  
  private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive&ar, const unsigned int version)
    {
        ar & id;
        ar & agencyId;
        ar & routeTypeId;
        ar & agencyAcronym;
        ar & agencyName;
        ar & shortname;
        ar & longname;
        ar & routeTypeName;
    }
  };

}

#endif // TR_ROUTE
