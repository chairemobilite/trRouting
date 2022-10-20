#ifndef TR_OD_TRIP
#define TR_OD_TRIP

#include <vector>
#include <boost/uuid/uuid.hpp>
#include "point.hpp"

namespace TrRouting
{
  class DataSource;
  
  class OdTrip {
  
  public:
    OdTrip(boost::uuids::uuid auuid,
           unsigned long long aid,
           const std::string &ainternalId,
           const DataSource &adataSource,
           int apersonIdx,
           int adepartureTimeSeconds,
           int aarrivalTimeSeconds,
           int awalkingTravelTimeSeconds,
           int acyclingTravelTimeSeconds,
           int adrivingTravelTimeSeconds,
           float aexpansionFactor,
           const std::string &amode,
           const std::string &aoriginActivity,
           const std::string &adestinationActivity,
           const std::vector<NodeTimeDistance> &aoriginNodes,
           const std::vector<NodeTimeDistance> &adestinationNodes,
           std::unique_ptr<Point> aorigin,
           std::unique_ptr<Point> adestination)
    : uuid(auuid),
      id(aid),
      internalId(ainternalId),
      dataSource(adataSource),
      personIdx(apersonIdx),
      departureTimeSeconds(adepartureTimeSeconds),
      arrivalTimeSeconds(aarrivalTimeSeconds),
      walkingTravelTimeSeconds(awalkingTravelTimeSeconds),
      cyclingTravelTimeSeconds(acyclingTravelTimeSeconds),
      drivingTravelTimeSeconds(adrivingTravelTimeSeconds),
      expansionFactor(aexpansionFactor),
      mode(amode),
      originActivity(aoriginActivity),
      destinationActivity(adestinationActivity),
      originNodes(aoriginNodes),
      destinationNodes(adestinationNodes),
      origin(std::move(aorigin)),
      destination(std::move(adestination)) {}

    boost::uuids::uuid uuid;
    unsigned long long id;
    const DataSource & dataSource;
    //int householdIdx; //TODO #167
    int personIdx;
    int departureTimeSeconds;
    int arrivalTimeSeconds;
    int walkingTravelTimeSeconds;
    int cyclingTravelTimeSeconds;
    int drivingTravelTimeSeconds;
    float expansionFactor;
    //TODO Why is it a string and not an actual mode object?
    std::string mode;
    std::string originActivity;
    std::string destinationActivity;
    std::string internalId;
    std::vector<NodeTimeDistance> originNodes;
    std::vector<NodeTimeDistance> destinationNodes;
    std::unique_ptr<Point> origin;
    std::unique_ptr<Point> destination;

    const std::string toString() {
      return "Od trip " + boost::uuids::to_string(uuid);
    }

  };

  

}

#endif // TR_OD_TRIP
