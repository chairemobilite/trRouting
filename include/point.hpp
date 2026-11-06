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

    /**
     * Check that the coordinates are within the range of the WGS84 degrees.
     * NaN coordinates are invalid, since any comparison with NaN is false.
     *
     * @return bool True if the latitude and the longitude are within their range
     */
    bool isValid() const {
      return latitude >= -MAX_ABSOLUTE_LATITUDE && latitude <= MAX_ABSOLUTE_LATITUDE
        && longitude >= -MAX_ABSOLUTE_LONGITUDE && longitude <= MAX_ABSOLUTE_LONGITUDE;
    }

  private:
    static constexpr double MAX_ABSOLUTE_LATITUDE = 90.0;
    static constexpr double MAX_ABSOLUTE_LONGITUDE = 180.0;
    
  };
  
}

#endif // TR_POINT
