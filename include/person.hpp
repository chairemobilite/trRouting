#ifndef TR_PERSON
#define TR_PERSON

#include <vector>
#include <string>
#include <boost/uuid/uuid.hpp>
#include "point.hpp"


namespace TrRouting
{
  
  class Person {
  
  public:
    Person( boost::uuids::uuid auuid,
            unsigned long long aid,
            const DataSource &adataSource,
            float aexpansionFactor,
            int aage,
            short adrivingLicenseOwner,
            short atransitPassOwner,
            std::string aageGroup,
            std::string agender,
            std::string aoccupation,
            std::string ainternalId) : uuid(auuid),
                                       id(aid),
                                       dataSource(adataSource),
                                       expansionFactor(aexpansionFactor),
                                       age(aage),
                                       drivingLicenseOwner(adrivingLicenseOwner),
                                       transitPassOwner(atransitPassOwner),
                                       ageGroup(aageGroup),
                                       gender(agender),
                                       occupation(aoccupation),
                                       internalId(ainternalId) {}
   
    boost::uuids::uuid uuid;
    //int householdIdx; //TODO #167
    unsigned long long id;
    const DataSource &dataSource;
    float expansionFactor;
    int age;
    short drivingLicenseOwner;
    short transitPassOwner;
    std::string ageGroup;
    std::string gender;
    std::string occupation;
    std::string internalId;
    /*TODO #167 Restore those fields when necessary
    std::unique_ptr<Point> usualWorkPlace;
    std::unique_ptr<Point> usualSchoolPlace;
    std::vector<int> usualWorkPlaceNodesIdx;
    std::vector<int> usualWorkPlaceNodesTravelTimesSeconds;
    std::vector<int> usualWorkPlaceNodesDistancesMeters;
    std::vector<int> usualSchoolPlaceNodesIdx;
    std::vector<int> usualSchoolPlaceNodesTravelTimesSeconds;
    std::vector<int> usualSchoolPlaceNodesDistancesMeters;
    int usualWorkPlaceWalkingTravelTimeSeconds;
    int usualWorkPlaceCyclingTravelTimeSeconds;
    int usualWorkPlaceDrivingTravelTimeSeconds;
    int usualSchoolPlaceWalkingTravelTimeSeconds;
    int usualSchoolPlaceCyclingTravelTimeSeconds;
    int usualSchoolPlaceDrivingTravelTimeSeconds;
    */
    const std::string toString() {
      return "Person " + boost::uuids::to_string(uuid) + " age " + std::to_string(age);
    }

  };

}

#endif // TR_PERSON
