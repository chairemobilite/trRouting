#ifndef TR_POINT
#define TR_POINT

#include <string>

namespace TrRouting
{
  
  class Point {
  
  public:
    
    double latitude;
    double longitude;
    Point() {}
    Point(double _latitude, double _longitude) : latitude(_latitude), longitude(_longitude) {}
    std::string asWKT() { return "POINT(" + std::to_string(longitude) + " " + std::to_string(latitude) + ")"; }
    
  };
  
}

#endif // TR_POINT
