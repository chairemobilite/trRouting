#ifndef TR_STOP
#define TR_STOP

#include <boost/serialization/access.hpp>
#include "point.hpp"

namespace TrRouting
{
  
  struct Stop {
  
  public:
   
    unsigned long long id;
    std::string code;
    std::string name;
    long long stationId;
    Point point;
  
  private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive&ar, const unsigned int version)
    {
      ar & id;
      ar & code;
      ar & name;
      ar & stationId;
      ar & point;
    }
  };

}

#endif // TR_STOP
