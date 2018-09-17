#ifndef TR_POINT
#define TR_POINT

#include <string>
#include <boost/serialization/access.hpp>

namespace TrRouting
{
  
  struct Point {
  
  public:
    
    double latitude;
    double longitude;
    Point() {}
    Point(double _latitude, double _longitude) : latitude(_latitude), longitude(_longitude) {}
    std::string asWKT() { return "POINT(" + std::to_string(longitude) + " " + std::to_string(latitude) + ")"; }
    
  private:
    
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive&ar, const unsigned int version)
    {
        ar & latitude;
        ar & longitude;
    }
    
  };
  
}

#endif // TR_POINT
