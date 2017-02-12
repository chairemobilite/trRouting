#ifndef TR_PATH_STOP_SEQUENCE
#define TR_PATH_STOP_SEQUENCE

#include <vector>
#include <boost/serialization/access.hpp>

namespace TrRouting
{
  
  struct PathStopSequence {
  public:
    
    long long id;
    long long stopId;
    long long routePathId;
    long long routeId;
    long long routeTypeId;
    long long agencyId;
    long long sequence;
  
  private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive&ar, const unsigned int version)
    {
        ar & id;
        ar & stopId;
        ar & routePathId;
        ar & routeId;
        ar & routeTypeId;
        ar & agencyId;
        ar & sequence;
    }
  };
  
  
}

#endif // TR_PATH_STOP_SEQUENCE
