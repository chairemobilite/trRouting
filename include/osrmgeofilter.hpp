#ifndef TR_OSRM_GEO_FILTER
#define TR_OSRM_GEO_FILTER

#include "geofilter.hpp"
#include <string>

namespace TrRouting
{
  /* Filter nodes using OSRM */
  class OsrmGeoFilter : public GeoFilter
  {
  public:
    OsrmGeoFilter(const std::string &mode, const std::string &host, const std::string & port);
    
    virtual std::vector<NodeTimeDistance> getAccessibleNodesFootpathsFromPoint(const Point &point,
                                                                       const std::map<boost::uuids::uuid, Node> &nodes,
                                                                       int maxWalkingTravelTime,
                                                                       float walkingSpeedMetersPerSecond,
                                                                       bool reversed = false);
  protected:
    /**
     * Format the coordinates of a point as sent to OSRM: "longitude,latitude",
     * with exactly 6 decimals (about 0.1 m)
     *
     * @param point The point to format
     * @return std::string The formatted coordinates
     */
    static std::string formatOsrmCoordinates(const Point &point);


    std::string mode;
    std::string host;
    std::string port; //Could be an int, but it's used as a string every where. Keep a string remove conversions

  };
}

#endif // TR_OSRM_GEO_FILTER
