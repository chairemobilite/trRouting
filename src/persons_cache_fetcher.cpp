
#ifndef TR_PERSONS_CACHE_FETCHER
#define TR_PERSONS_CACHE_FETCHER

#include <string>
#include <vector>

#include "cache_fetcher.hpp"
#include "person.hpp"
#include "point.hpp"
#include "capnp/personCollection.capnp.h"
#include "capnp/person.capnp.h"
//#include "toolbox.hpp"

namespace TrRouting
{

  const std::pair<std::vector<Person>, std::map<boost::uuids::uuid, int>> CacheFetcher::getPersons(std::map<boost::uuids::uuid, int> dataSourceIndexesByUuid, std::map<boost::uuids::uuid, int> householdIndexesByUuid, Parameters& params)
  { 

    using T           = Person;
    using TCollection = personCollection::PersonCollection;
    using cT          = person::Person;

    std::string tStr  = "persons";
    std::string TStr  = "Persons";

    std::vector<T> ts;
    std::string cacheFileName{tStr};
    std::map<boost::uuids::uuid, int> tIndexesByUuid;
    boost::uuids::string_generator uuidGenerator;

    std::cout << "Fetching " << tStr << " from cache..." << std::endl;

    for(std::map<boost::uuids::uuid, int>::iterator iter = dataSourceIndexesByUuid.begin(); iter != dataSourceIndexesByUuid.end(); ++iter)
    {
      boost::uuids::uuid dataSourceUuid = iter->first;

      std::string cacheFilePath {"dataSources/" + boost::uuids::to_string(dataSourceUuid) + "/persons/" + cacheFileName};

      int filesCount {CacheFetcher::getCacheFilesCount(cacheFilePath + ".capnpbin.count", params)};

      std::cout << "files count persons: " << filesCount << " path: " << cacheFilePath << std::endl;

      for (int i = 0; i < filesCount; i++)
      {
        std::string filePath {cacheFilePath + ".capnpbin" + (filesCount > 1 ? "." + std::to_string(i) : "")};

        if (CacheFetcher::capnpCacheFileExists(filePath, params))
        {
          int fd = open((CacheFetcher::getFilePath(filePath, params)).c_str(), O_RDWR);
          ::capnp::PackedFdMessageReader capnpTCollectionMessage(fd, {64 * 1024 * 1024});
          TCollection::Reader capnpTCollection = capnpTCollectionMessage.getRoot<TCollection>();
          for (cT::Reader capnpT : capnpTCollection.getPersons())
          {
            std::string uuid           {capnpT.getUuid()};
            std::string dataSourceUuid {capnpT.getDataSourceUuid()};
            std::string householdUuid  {capnpT.getHouseholdUuid()};
            Point * usualWorkPlace   = new Point();
            Point * usualSchoolPlace = new Point();
            T     * t                = new T();
            t->uuid                  = uuidGenerator(uuid);
            t->id                    = capnpT.getId();
            t->expansionFactor       = capnpT.getExpansionFactor();
            t->age                   = capnpT.getAge();
            t->internalId            = capnpT.getInternalId();
            t->drivingLicenseOwner   = capnpT.getDrivingLicenseOwner();
            t->transitPassOwner      = capnpT.getTransitPassOwner();

            t->dataSourceIdx   = dataSourceUuid.length() > 0 ? dataSourceIndexesByUuid[uuidGenerator(dataSourceUuid)] : -1;
            t->householdIdx    = householdUuid.length()  > 0 ? householdIndexesByUuid[uuidGenerator(householdUuid)]   : -1;

            switch (capnpT.getAgeGroup()) {
              case person::Person::AgeGroup::NONE     : t->ageGroup = "none";     break;
              case person::Person::AgeGroup::AG0004   : t->ageGroup = "ag0004";   break;
              case person::Person::AgeGroup::AG0509   : t->ageGroup = "ag0509";   break;
              case person::Person::AgeGroup::AG1014   : t->ageGroup = "ag1014";   break;
              case person::Person::AgeGroup::AG1519   : t->ageGroup = "ag1519";   break;
              case person::Person::AgeGroup::AG2024   : t->ageGroup = "ag2024";   break;
              case person::Person::AgeGroup::AG2529   : t->ageGroup = "ag2529";   break;
              case person::Person::AgeGroup::AG3034   : t->ageGroup = "ag3034";   break;
              case person::Person::AgeGroup::AG3539   : t->ageGroup = "ag3539";   break;
              case person::Person::AgeGroup::AG4044   : t->ageGroup = "ag4044";   break;
              case person::Person::AgeGroup::AG4549   : t->ageGroup = "ag4549";   break;
              case person::Person::AgeGroup::AG5054   : t->ageGroup = "ag5054";   break;
              case person::Person::AgeGroup::AG5559   : t->ageGroup = "ag5559";   break;
              case person::Person::AgeGroup::AG6064   : t->ageGroup = "ag6064";   break;
              case person::Person::AgeGroup::AG6569   : t->ageGroup = "ag6569";   break;
              case person::Person::AgeGroup::AG7074   : t->ageGroup = "ag7074";   break;
              case person::Person::AgeGroup::AG7579   : t->ageGroup = "ag7579";   break;
              case person::Person::AgeGroup::AG8084   : t->ageGroup = "ag8084";   break;
              case person::Person::AgeGroup::AG8589   : t->ageGroup = "ag8589";   break;
              case person::Person::AgeGroup::AG9094   : t->ageGroup = "ag9094";   break;
              case person::Person::AgeGroup::AG95PLUS : t->ageGroup = "ag95plus"; break;
              case person::Person::AgeGroup::UNKNOWN  : t->ageGroup = "unknown";  break;
            }

            switch (capnpT.getGender()) {
              case person::Person::Gender::NONE    : t->gender = "none";    break;
              case person::Person::Gender::FEMALE  : t->gender = "female";  break;
              case person::Person::Gender::MALE    : t->gender = "male";    break;
              case person::Person::Gender::CUSTOM  : t->gender = "custom";  break;
              case person::Person::Gender::UNKNOWN : t->gender = "unknown"; break;
            }

            switch (capnpT.getOccupation()) {
              case person::Person::Occupation::NONE               : t->occupation = "none";             break;
              case person::Person::Occupation::FULL_TIME_WORKER   : t->occupation = "fullTimeWorker";   break;
              case person::Person::Occupation::PART_TIME_WORKER   : t->occupation = "partTimeWorker";   break;
              case person::Person::Occupation::FULL_TIME_STUDENT  : t->occupation = "fullTimeStudent";  break;
              case person::Person::Occupation::PART_TIME_STUDENT  : t->occupation = "partTimeStudent";  break;
              case person::Person::Occupation::WORKER_AND_STUDENT : t->occupation = "workerAndStudent"; break;
              case person::Person::Occupation::RETIRED            : t->occupation = "retired";          break;
              case person::Person::Occupation::AT_HOME            : t->occupation = "atHome";           break;
              case person::Person::Occupation::OTHER              : t->occupation = "other";            break;
              case person::Person::Occupation::NON_APPLICABLE     : t->occupation = "nonApplicable";    break;
              case person::Person::Occupation::UNKNOWN            : t->occupation = "unknown";          break;
            }



            t->usualWorkPlace           = *usualWorkPlace;
            t->usualWorkPlace.latitude  = ((double)capnpT.getUsualWorkPlaceLatitude())  / 1000000.0;
            t->usualWorkPlace.longitude = ((double)capnpT.getUsualWorkPlaceLongitude()) / 1000000.0;

            const unsigned int usualWorkPlaceNodesCount {capnpT.getUsualWorkPlaceNodesIdx().size()};
            std::vector<int> usualWorkPlaceNodesIdx(usualWorkPlaceNodesCount);
            std::vector<int> usualWorkPlaceNodesTravelTimesSeconds(usualWorkPlaceNodesCount);
            std::vector<int> usualWorkPlaceNodesDistancesMeters(usualWorkPlaceNodesCount);
            for (int i = 0; i < usualWorkPlaceNodesCount; i++)
            {
              usualWorkPlaceNodesIdx               [i] = capnpT.getUsualWorkPlaceNodesIdx()[i];
              usualWorkPlaceNodesTravelTimesSeconds[i] = capnpT.getUsualWorkPlaceNodesTravelTimes()[i];
              usualWorkPlaceNodesDistancesMeters   [i] = capnpT.getUsualWorkPlaceNodesDistances()[i];
            }
            t->usualWorkPlaceNodesIdx                = usualWorkPlaceNodesIdx;
            t->usualWorkPlaceNodesTravelTimesSeconds = usualWorkPlaceNodesTravelTimesSeconds;
            t->usualWorkPlaceNodesDistancesMeters    = usualWorkPlaceNodesDistancesMeters;



            t->usualSchoolPlace           = *usualSchoolPlace;
            t->usualSchoolPlace.latitude  = ((double)capnpT.getUsualSchoolPlaceLatitude())  / 1000000.0;
            t->usualSchoolPlace.longitude = ((double)capnpT.getUsualSchoolPlaceLongitude()) / 1000000.0;

            const unsigned int usualSchoolPlaceNodesCount {capnpT.getUsualSchoolPlaceNodesIdx().size()};
            std::vector<int> usualSchoolPlaceNodesIdx(usualSchoolPlaceNodesCount);
            std::vector<int> usualSchoolPlaceNodesTravelTimesSeconds(usualSchoolPlaceNodesCount);
            std::vector<int> usualSchoolPlaceNodesDistancesMeters(usualSchoolPlaceNodesCount);
            for (int i = 0; i < usualSchoolPlaceNodesCount; i++)
            {
              usualSchoolPlaceNodesIdx               [i] = capnpT.getUsualSchoolPlaceNodesIdx()[i];
              usualSchoolPlaceNodesTravelTimesSeconds[i] = capnpT.getUsualSchoolPlaceNodesTravelTimes()[i];
              usualSchoolPlaceNodesDistancesMeters   [i] = capnpT.getUsualSchoolPlaceNodesDistances()[i];
            }
            t->usualSchoolPlaceNodesIdx                = usualSchoolPlaceNodesIdx;
            t->usualSchoolPlaceNodesTravelTimesSeconds = usualSchoolPlaceNodesTravelTimesSeconds;
            t->usualSchoolPlaceNodesDistancesMeters    = usualSchoolPlaceNodesDistancesMeters;

            ts.push_back(*t);
            tIndexesByUuid[t->uuid] = ts.size() - 1;
          }
          //std::cout << TStr << ":\n" << Toolbox::prettyPrintStructVector(ts) << std::endl;
          close(fd);
        }
        else
        {
          std::cerr << "missing " << filePath << " cache file!" << std::endl;
        }

      }
    }
    
    return std::make_pair(ts, tIndexesByUuid);
  }

}

#endif // TR_PERSONS_CACHE_FETCHER