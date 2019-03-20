
#ifndef TR_OD_TRIPS_CACHE_FETCHER
#define TR_OD_TRIPS_CACHE_FETCHER

#include <string>
#include <vector>

#include "cache_fetcher.hpp"
#include "od_trip.hpp"
#include "point.hpp"
#include "capnp/odTripCollection.capnp.h"
#include "capnp/odTrip.capnp.h"
//#include "toolbox.hpp"

namespace TrRouting
{

  const std::pair<std::vector<OdTrip>, std::map<boost::uuids::uuid, int>> CacheFetcher::getOdTrips(std::map<boost::uuids::uuid, int> dataSourceIndexesByUuid, std::map<boost::uuids::uuid, int> householdIndexesByUuid, std::map<boost::uuids::uuid, int> personIndexesByUuid, Parameters& params)
  { 

    using T           = OdTrip;
    using TCollection = odTripCollection::OdTripCollection;
    using cT          = odTrip::OdTrip;

    std::string tStr  = "odTrips";
    std::string TStr  = "OdTrips";

    std::vector<T> ts;
    std::string cacheFileName{tStr};
    std::map<boost::uuids::uuid, int> tIndexesByUuid;
    boost::uuids::string_generator uuidGenerator;

    std::cout << "Fetching " << tStr << " from cache..." << std::endl;

    for(std::map<boost::uuids::uuid, int>::iterator iter = dataSourceIndexesByUuid.begin(); iter != dataSourceIndexesByUuid.end(); ++iter)
    {
      boost::uuids::uuid dataSourceUuid = iter->first;

      std::string cacheFilePath {params.cacheDirectoryPath + params.projectShortname + "/dataSources/" + boost::uuids::to_string(dataSourceUuid) + "/odTrips/" + cacheFileName};

      int filesCount {CacheFetcher::getCacheFilesCount(cacheFilePath, params)};

      for (int i = 0; i < filesCount; i++)
      {
        std::string filePath {cacheFilePath + ".capnpbin" + (filesCount > 1 ? std::to_string(i) : "")};

        if (CacheFetcher::capnpCacheFileExists(cacheFileName, params))
        {
          int fd = open((filePath).c_str(), O_RDWR);
          ::capnp::PackedFdMessageReader capnpTCollectionMessage(fd, {64 * 1024 * 1024});
          TCollection::Reader capnpTCollection = capnpTCollectionMessage.getRoot<TCollection>();
          for (cT::Reader capnpT : capnpTCollection.getOdTrips())
          {
            std::string uuid           {capnpT.getUuid()};
            std::string dataSourceUuid {capnpT.getDataSourceUuid()};
            std::string householdUuid  {capnpT.getHouseholdUuid()};
            std::string personUuid     {capnpT.getPersonUuid()};
            Point * origin              = new Point();
            Point * destination         = new Point();
            T     * t                   = new T();
            t->uuid                     = uuidGenerator(uuid);
            t->id                       = capnpT.getId();
            t->expansionFactor          = capnpT.getExpansionFactor();
            t->internalId               = capnpT.getInternalId();
            t->departureTimeSeconds     = capnpT.getDepartureTimeSeconds();
            t->arrivalTimeSeconds       = capnpT.getArrivalTimeSeconds();
            t->walkingTravelTimeSeconds = capnpT.getWalkingTravelTimeSeconds();
            t->cyclingTravelTimeSeconds = capnpT.getCyclingTravelTimeSeconds();
            t->drivingTravelTimeSeconds = capnpT.getDrivingTravelTimeSeconds();

            t->dataSourceIdx   = dataSourceUuid.length() > 0 ? dataSourceIndexesByUuid[uuidGenerator(dataSourceUuid)] : -1;
            t->householdIdx    = householdUuid.length()  > 0 ? householdIndexesByUuid[uuidGenerator(householdUuid)]   : -1;
            t->personIdx       = personUuid.length()     > 0 ? personIndexesByUuid[uuidGenerator(personUuid)]         : -1;

            switch (capnpT.getMode()) {
              case odTrip::OdTrip::Mode::NONE             : t->mode = "none";            break;
              case odTrip::OdTrip::Mode::WALKING          : t->mode = "walking";         break;
              case odTrip::OdTrip::Mode::CYCLING          : t->mode = "cycling";         break;
              case odTrip::OdTrip::Mode::CAR_DRIVER       : t->mode = "carDriver";       break;
              case odTrip::OdTrip::Mode::CAR_PASSENGER    : t->mode = "carPassenger";    break;
              case odTrip::OdTrip::Mode::MOTORCYCLE       : t->mode = "motorcycle";      break;
              case odTrip::OdTrip::Mode::TRANSIT          : t->mode = "transit";         break;
              case odTrip::OdTrip::Mode::PARATRANSIT      : t->mode = "paratransit";     break;
              case odTrip::OdTrip::Mode::TAXI             : t->mode = "taxi";            break;
              case odTrip::OdTrip::Mode::SCHOOL_BUS       : t->mode = "schoolBus";       break;
              case odTrip::OdTrip::Mode::OTHER_BUS        : t->mode = "otherBus";        break;
              case odTrip::OdTrip::Mode::INTERCITY_BUS    : t->mode = "intercityBus";    break;
              case odTrip::OdTrip::Mode::INTERCITY_TRAIN  : t->mode = "intercityTrain";  break;
              case odTrip::OdTrip::Mode::PLANE            : t->mode = "plane";           break;
              case odTrip::OdTrip::Mode::FERRY            : t->mode = "ferry";           break;
              case odTrip::OdTrip::Mode::PARK_AND_RIDE    : t->mode = "parkAndRide";     break;
              case odTrip::OdTrip::Mode::KISS_AND_RIDE    : t->mode = "kissAndRide";     break;
              case odTrip::OdTrip::Mode::BIKE_AND_RIDE    : t->mode = "bikeAndRide";     break;
              case odTrip::OdTrip::Mode::MULTIMODAL_OTHER : t->mode = "multimodalOther"; break;
              case odTrip::OdTrip::Mode::OTHER            : t->mode = "other";           break;
              case odTrip::OdTrip::Mode::UNKNOWN          : t->mode = "unknown";         break;
            }

            switch (capnpT.getOriginActivity()) {
              case odTrip::OdTrip::Activity::NONE             : t->originActivity = "none";            break;
              case odTrip::OdTrip::Activity::HOME             : t->originActivity = "home";            break;
              case odTrip::OdTrip::Activity::WORK_USUAL       : t->originActivity = "workUsual";       break;
              case odTrip::OdTrip::Activity::WORK_NON_USUAL   : t->originActivity = "workNonUsual";    break;
              case odTrip::OdTrip::Activity::SCHOOL_USUAL     : t->originActivity = "schoolUsual";     break;
              case odTrip::OdTrip::Activity::SCHOOL_NON_USUAL : t->originActivity = "schoolNonUsual";  break;
              case odTrip::OdTrip::Activity::SHOPPING         : t->originActivity = "shopping";        break;
              case odTrip::OdTrip::Activity::LEISURE          : t->originActivity = "leisure";         break;
              case odTrip::OdTrip::Activity::SERVICE          : t->originActivity = "service";         break;
              case odTrip::OdTrip::Activity::SECONDARY_HOME   : t->originActivity = "secondaryHome";   break;
              case odTrip::OdTrip::Activity::VISITING_FRIENDS : t->originActivity = "visitingFriends"; break;
              case odTrip::OdTrip::Activity::DROP_SOMEONE     : t->originActivity = "dropSomeone";     break;
              case odTrip::OdTrip::Activity::FETCH_SOMEONE    : t->originActivity = "fetchSomeone";    break;
              case odTrip::OdTrip::Activity::RESTAURANT       : t->originActivity = "restaurant";      break;
              case odTrip::OdTrip::Activity::MEDICAL          : t->originActivity = "medical";         break;
              case odTrip::OdTrip::Activity::WORSHIP          : t->originActivity = "worship";         break;
              case odTrip::OdTrip::Activity::ON_THE_ROAD      : t->originActivity = "onTheRoad";       break;
              case odTrip::OdTrip::Activity::OTHER            : t->originActivity = "other";           break;
              case odTrip::OdTrip::Activity::UNKNOWN          : t->originActivity = "unknown";         break;
            }

            switch (capnpT.getDestinationActivity()) {
              case odTrip::OdTrip::Activity::NONE             : t->destinationActivity = "none";            break;
              case odTrip::OdTrip::Activity::HOME             : t->destinationActivity = "home";            break;
              case odTrip::OdTrip::Activity::WORK_USUAL       : t->destinationActivity = "workUsual";       break;
              case odTrip::OdTrip::Activity::WORK_NON_USUAL   : t->destinationActivity = "workNonUsual";    break;
              case odTrip::OdTrip::Activity::SCHOOL_USUAL     : t->destinationActivity = "schoolUsual";     break;
              case odTrip::OdTrip::Activity::SCHOOL_NON_USUAL : t->destinationActivity = "schoolNonUsual";  break;
              case odTrip::OdTrip::Activity::SHOPPING         : t->destinationActivity = "shopping";        break;
              case odTrip::OdTrip::Activity::LEISURE          : t->destinationActivity = "leisure";         break;
              case odTrip::OdTrip::Activity::SERVICE          : t->destinationActivity = "service";         break;
              case odTrip::OdTrip::Activity::SECONDARY_HOME   : t->destinationActivity = "secondaryHome";   break;
              case odTrip::OdTrip::Activity::VISITING_FRIENDS : t->destinationActivity = "visitingFriends"; break;
              case odTrip::OdTrip::Activity::DROP_SOMEONE     : t->destinationActivity = "dropSomeone";     break;
              case odTrip::OdTrip::Activity::FETCH_SOMEONE    : t->destinationActivity = "fetchSomeone";    break;
              case odTrip::OdTrip::Activity::RESTAURANT       : t->destinationActivity = "restaurant";      break;
              case odTrip::OdTrip::Activity::MEDICAL          : t->destinationActivity = "medical";         break;
              case odTrip::OdTrip::Activity::WORSHIP          : t->destinationActivity = "worship";         break;
              case odTrip::OdTrip::Activity::ON_THE_ROAD      : t->destinationActivity = "onTheRoad";       break;
              case odTrip::OdTrip::Activity::OTHER            : t->destinationActivity = "other";           break;
              case odTrip::OdTrip::Activity::UNKNOWN          : t->destinationActivity = "unknown";         break;
            }



            t->origin           = *origin;
            t->origin.latitude  = ((double)capnpT.getOriginLatitude())  / 1000000.0;
            t->origin.longitude = ((double)capnpT.getOriginLongitude()) / 1000000.0;

            const unsigned int originNodesCount {capnpT.getOriginNodesIdx().size()};
            std::vector<int> originNodesIdx(originNodesCount);
            std::vector<int> originNodesTravelTimesSeconds(originNodesCount);
            std::vector<int> originNodesDistancesMeters(originNodesCount);
            for (int i = 0; i < originNodesCount; i++)
            {
              originNodesIdx               [i] = capnpT.getOriginNodesIdx()[i];
              originNodesTravelTimesSeconds[i] = capnpT.getOriginNodesTravelTimes()[i];
              originNodesDistancesMeters   [i] = capnpT.getOriginNodesDistances()[i];
            }
            t->originNodesIdx                = originNodesIdx;
            t->originNodesTravelTimesSeconds = originNodesTravelTimesSeconds;
            t->originNodesDistancesMeters    = originNodesDistancesMeters;



            t->destination           = *destination;
            t->destination.latitude  = ((double)capnpT.getDestinationLatitude())  / 1000000.0;
            t->destination.longitude = ((double)capnpT.getDestinationLongitude()) / 1000000.0;

            const unsigned int destinationNodesCount {capnpT.getDestinationNodesIdx().size()};
            std::vector<int> destinationNodesIdx(destinationNodesCount);
            std::vector<int> destinationNodesTravelTimesSeconds(destinationNodesCount);
            std::vector<int> destinationNodesDistancesMeters(destinationNodesCount);
            for (int i = 0; i < destinationNodesCount; i++)
            {
              destinationNodesIdx               [i] = capnpT.getDestinationNodesIdx()[i];
              destinationNodesTravelTimesSeconds[i] = capnpT.getDestinationNodesTravelTimes()[i];
              destinationNodesDistancesMeters   [i] = capnpT.getDestinationNodesDistances()[i];
            }
            t->destinationNodesIdx                = destinationNodesIdx;
            t->destinationNodesTravelTimesSeconds = destinationNodesTravelTimesSeconds;
            t->destinationNodesDistancesMeters    = destinationNodesDistancesMeters;



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

#endif // TR_OD_TRIPS_CACHE_FETCHER