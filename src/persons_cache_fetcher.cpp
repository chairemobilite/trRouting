
#ifndef TR_PERSONS_CACHE_FETCHER
#define TR_PERSONS_CACHE_FETCHER

#include <string>
#include <vector>
#include <fcntl.h>
#include <boost/uuid/string_generator.hpp>
#include <capnp/serialize-packed.h>
#include "cache_fetcher.hpp"
#include "person.hpp"
#include "point.hpp"
#include "capnp/personCollection.capnp.h"
#include "capnp/person.capnp.h"

namespace TrRouting
{

  int CacheFetcher::getPersons(
    std::vector<std::unique_ptr<Person>>& ts,
    std::map<boost::uuids::uuid, int>& tIndexesByUuid,
    std::map<boost::uuids::uuid, int>& dataSourceIndexesByUuid,
    std::map<boost::uuids::uuid, int>& householdIndexesByUuid,
    std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
    Parameters& params,
    std::string customPath
  )
  { 

    using T           = Person;
    using TCollection = personCollection::PersonCollection;
    using cT          = person::Person;

    ts.clear();
    tIndexesByUuid.clear();

    std::string tStr  = "persons";
    std::string TStr  = "Persons";

    std::string cacheFileName{tStr};
    boost::uuids::string_generator uuidGenerator;

    std::cout << "Fetching " << tStr << " from cache..." << std::endl;

    for(std::map<boost::uuids::uuid, int>::iterator iter = dataSourceIndexesByUuid.begin(); iter != dataSourceIndexesByUuid.end(); ++iter)
    {
      boost::uuids::uuid dataSourceUuid = iter->first;

      std::string cacheFilePath {"dataSources/" + boost::uuids::to_string(dataSourceUuid) + "/" + cacheFileName};

      int filesCount {CacheFetcher::getCacheFilesCount(CacheFetcher::getFilePath(cacheFilePath + ".capnpbin.count", params, customPath))};

      std::cout << "files count persons: " << filesCount << " path: " << cacheFilePath << std::endl;

      for (int i = 0; i < filesCount; i++)
      {
        std::string filePath {cacheFilePath + ".capnpbin" + (filesCount > 1 ? "." + std::to_string(i) : "")};
        std::string cacheFilePath = CacheFetcher::getFilePath(filePath, params, customPath);

        int fd = open(cacheFilePath.c_str(), O_RDWR);
        if (fd < 0)
        {
          int err = errno;
          if (err == ENOENT)
          {
            std::cerr << "missing " << filePath << " cache file!" << std::endl;
          }
          else
          {
            std::cerr << "Error opening cache file " << filePath << ": " << err << std::endl;
          }
          continue;
        }

        try
        {   
          ::capnp::PackedFdMessageReader capnpTCollectionMessage(fd, {64 * 1024 * 1024});
          TCollection::Reader capnpTCollection = capnpTCollectionMessage.getRoot<TCollection>();
          for (cT::Reader capnpT : capnpTCollection.getPersons())
          {
            std::string uuid           {capnpT.getUuid()};
            std::string dataSourceUuid {capnpT.getDataSourceUuid()};
            std::string householdUuid  {capnpT.getHouseholdUuid()};

            std::unique_ptr<Point> usualWorkPlace   = std::make_unique<Point>();
            std::unique_ptr<Point> usualSchoolPlace = std::make_unique<Point>();
            std::unique_ptr<T> t                    = std::make_unique<T>();
            
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



            usualWorkPlace->latitude  = ((double)capnpT.getUsualWorkPlaceLatitude())  / 1000000.0;
            usualWorkPlace->longitude = ((double)capnpT.getUsualWorkPlaceLongitude()) / 1000000.0;
            t->usualWorkPlace         = std::move(usualWorkPlace);

            const unsigned int usualWorkPlaceNodesCount {capnpT.getUsualWorkPlaceNodesUuids().size()};
            std::vector<int> usualWorkPlaceNodesIdx(usualWorkPlaceNodesCount);
            std::vector<int> usualWorkPlaceNodesTravelTimesSeconds(usualWorkPlaceNodesCount);
            std::vector<int> usualWorkPlaceNodesDistancesMeters(usualWorkPlaceNodesCount);
            for (int i = 0; i < usualWorkPlaceNodesCount; i++)
            {
              std::string nodeUuid {capnpT.getUsualWorkPlaceNodesUuids()[i]};
              usualWorkPlaceNodesIdx               [i] = nodeIndexesByUuid[uuidGenerator(nodeUuid)];
              usualWorkPlaceNodesTravelTimesSeconds[i] = capnpT.getUsualWorkPlaceNodesTravelTimes()[i];
              usualWorkPlaceNodesDistancesMeters   [i] = capnpT.getUsualWorkPlaceNodesDistances()[i];
            }
            t->usualWorkPlaceNodesIdx                = usualWorkPlaceNodesIdx;
            t->usualWorkPlaceNodesTravelTimesSeconds = usualWorkPlaceNodesTravelTimesSeconds;
            t->usualWorkPlaceNodesDistancesMeters    = usualWorkPlaceNodesDistancesMeters;



            usualSchoolPlace->latitude  = ((double)capnpT.getUsualSchoolPlaceLatitude())  / 1000000.0;
            usualSchoolPlace->longitude = ((double)capnpT.getUsualSchoolPlaceLongitude()) / 1000000.0;
            t->usualSchoolPlace         = std::move(usualSchoolPlace);

            const unsigned int usualSchoolPlaceNodesCount {capnpT.getUsualSchoolPlaceNodesUuids().size()};
            std::vector<int> usualSchoolPlaceNodesIdx(usualSchoolPlaceNodesCount);
            std::vector<int> usualSchoolPlaceNodesTravelTimesSeconds(usualSchoolPlaceNodesCount);
            std::vector<int> usualSchoolPlaceNodesDistancesMeters(usualSchoolPlaceNodesCount);
            for (int i = 0; i < usualSchoolPlaceNodesCount; i++)
            {
              std::string nodeUuid {capnpT.getUsualSchoolPlaceNodesUuids()[i]};
              usualSchoolPlaceNodesIdx               [i] = nodeIndexesByUuid[uuidGenerator(nodeUuid)];
              usualSchoolPlaceNodesTravelTimesSeconds[i] = capnpT.getUsualSchoolPlaceNodesTravelTimes()[i];
              usualSchoolPlaceNodesDistancesMeters   [i] = capnpT.getUsualSchoolPlaceNodesDistances()[i];
            }
            t->usualSchoolPlaceNodesIdx                = usualSchoolPlaceNodesIdx;
            t->usualSchoolPlaceNodesTravelTimesSeconds = usualSchoolPlaceNodesTravelTimesSeconds;
            t->usualSchoolPlaceNodesDistancesMeters    = usualSchoolPlaceNodesDistancesMeters;

            t->internalId                               = capnpT.getInternalId();
            t->usualWorkPlaceWalkingTravelTimeSeconds   = capnpT.getUsualWorkPlaceWalkingTravelTimeSeconds();
            t->usualWorkPlaceCyclingTravelTimeSeconds   = capnpT.getUsualWorkPlaceCyclingTravelTimeSeconds();
            t->usualWorkPlaceDrivingTravelTimeSeconds   = capnpT.getUsualWorkPlaceDrivingTravelTimeSeconds();
            t->usualSchoolPlaceWalkingTravelTimeSeconds = capnpT.getUsualSchoolPlaceWalkingTravelTimeSeconds();
            t->usualSchoolPlaceCyclingTravelTimeSeconds = capnpT.getUsualSchoolPlaceCyclingTravelTimeSeconds();
            t->usualSchoolPlaceDrivingTravelTimeSeconds = capnpT.getUsualSchoolPlaceDrivingTravelTimeSeconds();

            tIndexesByUuid[t->uuid] = ts.size();
            ts.push_back(std::move(t));
          }
        }
        catch (const kj::Exception& e)
        {
          std::cerr << "Error reading cache file " << filePath << ": " << e.getDescription().cStr() << std::endl;
        }
        catch (...)
        {
          std::cerr << "Unknown error occurred " << filePath << std::endl;
        }

        close(fd);

      }
    }
    return 0;
    
  }

}

#endif // TR_PERSONS_CACHE_FETCHER
