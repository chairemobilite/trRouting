
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
#include "spdlog/spdlog.h"

namespace TrRouting
{
  class DataSource;

  std::string getPersonAgeGroupStr(const person::Person::AgeGroup& group) {
    std::string str;

    switch (group) {
      case person::Person::AgeGroup::NONE     : str = "none";     break;
      case person::Person::AgeGroup::AG0004   : str = "ag0004";   break;
      case person::Person::AgeGroup::AG0509   : str = "ag0509";   break;
      case person::Person::AgeGroup::AG1014   : str = "ag1014";   break;
      case person::Person::AgeGroup::AG1519   : str = "ag1519";   break;
      case person::Person::AgeGroup::AG2024   : str = "ag2024";   break;
      case person::Person::AgeGroup::AG2529   : str = "ag2529";   break;
      case person::Person::AgeGroup::AG3034   : str = "ag3034";   break;
      case person::Person::AgeGroup::AG3539   : str = "ag3539";   break;
      case person::Person::AgeGroup::AG4044   : str = "ag4044";   break;
      case person::Person::AgeGroup::AG4549   : str = "ag4549";   break;
      case person::Person::AgeGroup::AG5054   : str = "ag5054";   break;
      case person::Person::AgeGroup::AG5559   : str = "ag5559";   break;
      case person::Person::AgeGroup::AG6064   : str = "ag6064";   break;
      case person::Person::AgeGroup::AG6569   : str = "ag6569";   break;
      case person::Person::AgeGroup::AG7074   : str = "ag7074";   break;
      case person::Person::AgeGroup::AG7579   : str = "ag7579";   break;
      case person::Person::AgeGroup::AG8084   : str = "ag8084";   break;
      case person::Person::AgeGroup::AG8589   : str = "ag8589";   break;
      case person::Person::AgeGroup::AG9094   : str = "ag9094";   break;
      case person::Person::AgeGroup::AG95PLUS : str = "ag95plus"; break;
      case person::Person::AgeGroup::UNKNOWN  : str = "unknown";  break;
    }
    return str;
  }

  std::string getPersonGenderStr(const person::Person::Gender &gender) {
    std::string str;

    switch (gender) {
      case person::Person::Gender::NONE    : str = "none";    break;
      case person::Person::Gender::FEMALE  : str = "female";  break;
      case person::Person::Gender::MALE    : str = "male";    break;
      case person::Person::Gender::CUSTOM  : str = "custom";  break;
      case person::Person::Gender::UNKNOWN : str = "unknown"; break;
    }
    return str;
  }

  std::string getPersonOccupationStr(const person::Person::Occupation &occupation) {
    std::string str;
    switch (occupation) {
      case person::Person::Occupation::NONE               : str = "none";             break;
      case person::Person::Occupation::FULL_TIME_WORKER   : str = "fullTimeWorker";   break;
      case person::Person::Occupation::PART_TIME_WORKER   : str = "partTimeWorker";   break;
      case person::Person::Occupation::FULL_TIME_STUDENT  : str = "fullTimeStudent";  break;
      case person::Person::Occupation::PART_TIME_STUDENT  : str = "partTimeStudent";  break;
      case person::Person::Occupation::WORKER_AND_STUDENT : str = "workerAndStudent"; break;
      case person::Person::Occupation::RETIRED            : str = "retired";          break;
      case person::Person::Occupation::AT_HOME            : str = "atHome";           break;
      case person::Person::Occupation::OTHER              : str = "other";            break;
      case person::Person::Occupation::NON_APPLICABLE     : str = "nonApplicable";    break;
      case person::Person::Occupation::UNKNOWN            : str = "unknown";          break;
    }
    return str;
  }

  int CacheFetcher::getPersons(
    std::map<boost::uuids::uuid, Person>& ts,
    const std::map<boost::uuids::uuid, DataSource>& dataSources,
    std::string customPath
  )
  { 

    using T           = Person;
    using TCollection = personCollection::PersonCollection;
    using cT          = person::Person;

    ts.clear();

    std::string tStr  = "persons";
    std::string TStr  = "Persons";

    std::string cacheFileName{tStr};
    boost::uuids::string_generator uuidGenerator;

    spdlog::info("Fetching {} from cache... {}", tStr, customPath);

    for(std::map<boost::uuids::uuid, DataSource>::const_iterator iter = dataSources.begin(); iter != dataSources.end(); ++iter)
    {
      boost::uuids::uuid dataSourceUuid = iter->first;

      std::string cacheFilePath {"dataSources/" + boost::uuids::to_string(dataSourceUuid) + "/" + cacheFileName};

      int filesCount {CacheFetcher::getCacheFilesCount(getFilePath(cacheFilePath + ".capnpbin.count", customPath))};

      spdlog::info("files count persons: {} path: {}", filesCount, cacheFilePath);

      for (int i = 0; i < filesCount; i++)
      {
        std::string filePath {cacheFilePath + ".capnpbin" + (filesCount > 1 ? "." + std::to_string(i) : "")};
        std::string cacheFilePath = getFilePath(filePath, customPath);

        int fd = open(cacheFilePath.c_str(), O_RDWR);
        if (fd < 0)
        {
          int err = errno;
          if (err == ENOENT)
          {
            spdlog::error("missing {} cache files!", filePath);
          }
          else
          {
            spdlog::error("Error opening cache file {} : {} ", filePath, err);
          }
          continue;
        }

        try
        {   
          ::capnp::PackedFdMessageReader capnpTCollectionMessage(fd, {64 * 1024 * 1024});
          TCollection::Reader capnpTCollection = capnpTCollectionMessage.getRoot<TCollection>();
          for (cT::Reader capnpT : capnpTCollection.getPersons())
          {
            std::string uuidStr           {capnpT.getUuid()};
            std::string dataSourceUuid {capnpT.getDataSourceUuid()};
            //TODO #167 Household are ignored for the moment
            std::string householdUuid  {capnpT.getHouseholdUuid()};

            std::unique_ptr<Point> usualWorkPlace   = std::make_unique<Point>();
            std::unique_ptr<Point> usualSchoolPlace = std::make_unique<Point>();

            /* TODO #167
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
              usualWorkPlaceNodesIdx               [i] = nodeIndexesByUuid.at(uuidGenerator(nodeUuid));
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
              usualSchoolPlaceNodesIdx               [i] = nodeIndexesByUuid.at(uuidGenerator(nodeUuid));
              usualSchoolPlaceNodesTravelTimesSeconds[i] = capnpT.getUsualSchoolPlaceNodesTravelTimes()[i];
              usualSchoolPlaceNodesDistancesMeters   [i] = capnpT.getUsualSchoolPlaceNodesDistances()[i];
            }
            t->usualSchoolPlaceNodesIdx                = usualSchoolPlaceNodesIdx;
            t->usualSchoolPlaceNodesTravelTimesSeconds = usualSchoolPlaceNodesTravelTimesSeconds;
            t->usualSchoolPlaceNodesDistancesMeters    = usualSchoolPlaceNodesDistancesMeters;

            t->usualWorkPlaceWalkingTravelTimeSeconds   = capnpT.getUsualWorkPlaceWalkingTravelTimeSeconds();
            t->usualWorkPlaceCyclingTravelTimeSeconds   = capnpT.getUsualWorkPlaceCyclingTravelTimeSeconds();
            t->usualWorkPlaceDrivingTravelTimeSeconds   = capnpT.getUsualWorkPlaceDrivingTravelTimeSeconds();
            t->usualSchoolPlaceWalkingTravelTimeSeconds = capnpT.getUsualSchoolPlaceWalkingTravelTimeSeconds();
            t->usualSchoolPlaceCyclingTravelTimeSeconds = capnpT.getUsualSchoolPlaceCyclingTravelTimeSeconds();
            t->usualSchoolPlaceDrivingTravelTimeSeconds = capnpT.getUsualSchoolPlaceDrivingTravelTimeSeconds();

            */
            auto uuid = uuidGenerator(uuidStr);
            ts.emplace(uuid, T(uuid,
                               capnpT.getId(),
                               dataSources.at(uuidGenerator(dataSourceUuid)),
                               capnpT.getExpansionFactor(),
                               capnpT.getAge(),
                               capnpT.getDrivingLicenseOwner(),
                               capnpT.getTransitPassOwner(),
                               getPersonAgeGroupStr(capnpT.getAgeGroup()),
                               getPersonGenderStr(capnpT.getGender()),
                               getPersonOccupationStr(capnpT.getOccupation()),
                               capnpT.getInternalId()
                               )
                       );
          }
        }
        catch (const kj::Exception& e)
        {
          spdlog::error("Error opening cache file {}: {}", filePath, e.getDescription().cStr());
        }
        catch (...)
        {
          spdlog::error("Unknown error occurred {} ", filePath);
        }

        close(fd);

      }
    }
    return 0;
    
  }

}

#endif // TR_PERSONS_CACHE_FETCHER
