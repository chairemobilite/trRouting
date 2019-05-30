#ifndef TR_PERSON
#define TR_PERSON

#include <vector>
#include <string>
#include <boost/uuid/uuid.hpp>
#include "point.hpp"


namespace TrRouting
{
  
  struct Person {
  
  public:
   
    boost::uuids::uuid uuid;
    int dataSourceIdx;
    int householdIdx;
    unsigned long long id;
    float expansionFactor;
    int age;
    short drivingLicenseOwner;
    short transitPassOwner;
    std::string ageGroup;
    std::string gender;
    std::string occupation;
    std::string internalId;
    std::unique_ptr<Point> usualWorkPlace;
    std::unique_ptr<Point> usualSchoolPlace;
    std::vector<int> usualWorkPlaceNodesIdx;
    std::vector<int> usualWorkPlaceNodesTravelTimesSeconds;
    std::vector<int> usualWorkPlaceNodesDistancesMeters;
    std::vector<int> usualSchoolPlaceNodesIdx;
    std::vector<int> usualSchoolPlaceNodesTravelTimesSeconds;
    std::vector<int> usualSchoolPlaceNodesDistancesMeters;

    const std::string toString() {
      return "Person " + boost::uuids::to_string(uuid) + " age " + std::to_string(age);
    }

  };

}

#endif // TR_PERSON
