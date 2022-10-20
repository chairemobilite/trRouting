
#ifndef TR_OD_TRIPS_CACHE_FETCHER
#define TR_OD_TRIPS_CACHE_FETCHER

#include <string>
#include <vector>
#include <fcntl.h>
#include <boost/uuid/string_generator.hpp>
#include <capnp/serialize-packed.h>
#include "cache_fetcher.hpp"
#include "od_trip.hpp"
#include "point.hpp"
#include "capnp/odTripCollection.capnp.h"
#include "capnp/odTrip.capnp.h"
#include "spdlog/spdlog.h"

namespace TrRouting
{

  std::string getOdTripModeStr(const odTrip::OdTrip::Mode &mode) {
    std::string str;

    switch (mode) {
    case odTrip::OdTrip::Mode::NONE             : str = "none";            break;
    case odTrip::OdTrip::Mode::WALKING          : str = "walking";         break;
    case odTrip::OdTrip::Mode::CYCLING          : str = "cycling";         break;
    case odTrip::OdTrip::Mode::CAR_DRIVER       : str = "carDriver";       break;
    case odTrip::OdTrip::Mode::CAR_PASSENGER    : str = "carPassenger";    break;
    case odTrip::OdTrip::Mode::MOTORCYCLE       : str = "motorcycle";      break;
    case odTrip::OdTrip::Mode::TRANSIT          : str = "transit";         break;
    case odTrip::OdTrip::Mode::PARATRANSIT      : str = "paratransit";     break;
    case odTrip::OdTrip::Mode::TAXI             : str = "taxi";            break;
    case odTrip::OdTrip::Mode::SCHOOL_BUS       : str = "schoolBus";       break;
    case odTrip::OdTrip::Mode::OTHER_BUS        : str = "otherBus";        break;
    case odTrip::OdTrip::Mode::INTERCITY_BUS    : str = "intercityBus";    break;
    case odTrip::OdTrip::Mode::INTERCITY_TRAIN  : str = "intercityTrain";  break;
    case odTrip::OdTrip::Mode::PLANE            : str = "plane";           break;
    case odTrip::OdTrip::Mode::FERRY            : str = "ferry";           break;
    case odTrip::OdTrip::Mode::PARK_AND_RIDE    : str = "parkAndRide";     break;
    case odTrip::OdTrip::Mode::KISS_AND_RIDE    : str = "kissAndRide";     break;
    case odTrip::OdTrip::Mode::BIKE_AND_RIDE    : str = "bikeAndRide";     break;
    case odTrip::OdTrip::Mode::MULTIMODAL_OTHER : str = "multimodalOther"; break;
    case odTrip::OdTrip::Mode::OTHER            : str = "other";           break;
    case odTrip::OdTrip::Mode::UNKNOWN          : str = "unknown";         break;
    }

    return str;
  }

  std::string getOdTripActivityStr(const odTrip::OdTrip::Activity &activity) {
    std::string str;
    switch (activity) {
    case odTrip::OdTrip::Activity::NONE             : str = "none";            break;
    case odTrip::OdTrip::Activity::HOME             : str = "home";            break;
    case odTrip::OdTrip::Activity::WORK_USUAL       : str = "workUsual";       break;
    case odTrip::OdTrip::Activity::WORK_NON_USUAL   : str = "workNonUsual";    break;
    case odTrip::OdTrip::Activity::SCHOOL_USUAL     : str = "schoolUsual";     break;
    case odTrip::OdTrip::Activity::SCHOOL_NON_USUAL : str = "schoolNonUsual";  break;
    case odTrip::OdTrip::Activity::SHOPPING         : str = "shopping";        break;
    case odTrip::OdTrip::Activity::LEISURE          : str = "leisure";         break;
    case odTrip::OdTrip::Activity::SERVICE          : str = "service";         break;
    case odTrip::OdTrip::Activity::SECONDARY_HOME   : str = "secondaryHome";   break;
    case odTrip::OdTrip::Activity::VISITING_FRIENDS : str = "visitingFriends"; break;
    case odTrip::OdTrip::Activity::DROP_SOMEONE     : str = "dropSomeone";     break;
    case odTrip::OdTrip::Activity::FETCH_SOMEONE    : str = "fetchSomeone";    break;
    case odTrip::OdTrip::Activity::RESTAURANT       : str = "restaurant";      break;
    case odTrip::OdTrip::Activity::MEDICAL          : str = "medical";         break;
    case odTrip::OdTrip::Activity::WORSHIP          : str = "worship";         break;
    case odTrip::OdTrip::Activity::ON_THE_ROAD      : str = "onTheRoad";       break;
    case odTrip::OdTrip::Activity::OTHER            : str = "other";           break;
    case odTrip::OdTrip::Activity::UNKNOWN          : str = "unknown";         break;
    }
    return str;
  }

  int CacheFetcher::getOdTrips(
    std::vector<std::unique_ptr<OdTrip>>& ts,
    std::map<boost::uuids::uuid, int>& tIndexesByUuid,
    const std::map<boost::uuids::uuid, DataSource>& dataSources,
    const std::map<boost::uuids::uuid, int>& personIndexesByUuid,
    const std::map<boost::uuids::uuid, Node>& nodes,
    std::string customPath
  )
  {

    using T           = OdTrip;
    using TCollection = odTripCollection::OdTripCollection;
    using cT          = odTrip::OdTrip;

    std::string tStr  = "odTrips";
    std::string TStr  = "OdTrips";

    std::string cacheFileName{tStr};
    boost::uuids::string_generator uuidGenerator;

    spdlog::info("Fetching {} from cache... {}", tStr, customPath);

    for(std::map<boost::uuids::uuid,  DataSource>::const_iterator iter = dataSources.begin(); iter != dataSources.end(); ++iter)
    {
      boost::uuids::uuid dataSourceUuid = iter->first;

      std::string cacheFilePath {"dataSources/" + boost::uuids::to_string(dataSourceUuid) + "/" + cacheFileName};

      int filesCount {CacheFetcher::getCacheFilesCount(getFilePath(cacheFilePath + ".capnpbin.count", customPath))};

      spdlog::info("files count odTrips: {} path: {}", filesCount, cacheFilePath);

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
          ::capnp::PackedFdMessageReader capnpTCollectionMessage(fd, {512 * 1024 * 1024});
          TCollection::Reader capnpTCollection = capnpTCollectionMessage.getRoot<TCollection>();
          for (cT::Reader capnpT : capnpTCollection.getOdTrips())
          {
            std::string uuid           {capnpT.getUuid()};
            std::string dataSourceUuid {capnpT.getDataSourceUuid()};
            //TODO #167 Household are ignored for the moment
            std::string householdUuid  {capnpT.getHouseholdUuid()};
            std::string personUuid     {capnpT.getPersonUuid()};

            std::unique_ptr<Point> origin      = std::make_unique<Point>();
            std::unique_ptr<Point> destination = std::make_unique<Point>();

            origin->latitude          = ((double)capnpT.getOriginLatitude())       / 1000000.0;
            origin->longitude         = ((double)capnpT.getOriginLongitude())      / 1000000.0;
            destination->latitude     = ((double)capnpT.getDestinationLatitude())  / 1000000.0;
            destination->longitude    = ((double)capnpT.getDestinationLongitude()) / 1000000.0;

            const unsigned int originNodesCount {capnpT.getOriginNodesUuids().size()};
            std::vector<NodeTimeDistance> originNodes;
            for (int i = 0; i < originNodesCount; i++)
            {
              std::string nodeUuid {capnpT.getOriginNodesUuids()[i]};
              originNodes.push_back(NodeTimeDistance(nodes.at(uuidGenerator(nodeUuid)),
                                                     capnpT.getOriginNodesTravelTimes()[i],
                                                     capnpT.getOriginNodesDistances()[i]));
            }

            const unsigned int destinationNodesCount {capnpT.getDestinationNodesUuids().size()};
            std::vector<NodeTimeDistance> destinationNodes;
            for (int i = 0; i < destinationNodesCount; i++)
            {
              std::string nodeUuid {capnpT.getDestinationNodesUuids()[i]};
              destinationNodes.push_back(NodeTimeDistance(nodes.at(uuidGenerator(nodeUuid)),
                                                          capnpT.getDestinationNodesTravelTimes()[i],
                                                          capnpT.getDestinationNodesDistances()[i]));
            }

            // Create new odTrip
            std::unique_ptr<T> t = std::make_unique<T>(uuidGenerator(uuid),
                                                       capnpT.getId(),
                                                       capnpT.getInternalId(),
                                                       dataSources.at(uuidGenerator(dataSourceUuid)),
                                                       personUuid.length() > 0 ? personIndexesByUuid.at(uuidGenerator(personUuid)) : -1,
                                                       capnpT.getDepartureTimeSeconds(),
                                                       capnpT.getArrivalTimeSeconds(),
                                                       capnpT.getWalkingTravelTimeSeconds(),
                                                       capnpT.getCyclingTravelTimeSeconds(),
                                                       capnpT.getDrivingTravelTimeSeconds(),
                                                       //TODO comparison with float is fishy, confirm it's ok
                                                       capnpT.getExpansionFactor() == -1.0 ? 1.0 : capnpT.getExpansionFactor(),
                                                       getOdTripModeStr(capnpT.getMode()),
                                                       getOdTripActivityStr(capnpT.getOriginActivity()),
                                                       getOdTripActivityStr(capnpT.getDestinationActivity()),
                                                       originNodes,
                                                       destinationNodes,
                                                       std::move(origin),
                                                       std::move(destination)
                                                       );
            tIndexesByUuid[t->uuid] = ts.size();
            ts.push_back(std::move(t));

          }

          spdlog::info("parsed {} od trips", ts.size());
        }
        catch (const kj::Exception& e)
        {
          spdlog::error("Error opening cache file {}: {}", filePath, e.getDescription().cStr());
        }
        catch (const std::exception& e)
        {
          spdlog::error("Unknown error occurred {} {}", tStr, e.what());
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

#endif // TR_OD_TRIPS_CACHE_FETCHER
