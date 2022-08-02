
#ifndef TR_TRIPS_AND_CONNECTIONS_CACHE_FETCHER
#define TR_TRIPS_AND_CONNECTIONS_CACHE_FETCHER

#include <string>
#include <vector>
#include <errno.h>
#include <fcntl.h>
#include <kj/exception.h>
#include <boost/uuid/string_generator.hpp>
#include <capnp/serialize-packed.h>
#include "spdlog/spdlog.h"

#include "cache_fetcher.hpp"
#include "trip.hpp"
#include "line.hpp"
#include "capnp/line.capnp.h"
#include "calculation_time.hpp"

namespace TrRouting
{
  // FIXME ConnectionTuple is defined in calculator.hpp. Should it be elsewhere? Cache fetcher should work for any algorithm, is this tuple csa-specific or should it go somewhere common.
  using ConnectionTuple = std::tuple<int,int,int,int,int,short,short,int,short,short>;
  
  int CacheFetcher::getSchedules(
    std::vector<std::unique_ptr<Trip>>& trips,
    const std::map<boost::uuids::uuid, Line>& lines,
    std::vector<std::unique_ptr<Path>>& paths,
    std::map<boost::uuids::uuid, int>& tripIndexesByUuid,
    const std::map<boost::uuids::uuid, Service>& services,
    const std::map<boost::uuids::uuid, int>& pathIndexesByUuid,
    const std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
    std::vector<std::vector<std::unique_ptr<int>>>&   tripConnectionDepartureTimes,
    std::vector<std::vector<std::unique_ptr<float>>>& tripConnectionDemands,
    std::vector<std::shared_ptr<ConnectionTuple>>& connections,
    std::string customPath
  )
  {
    trips.clear();
    trips.shrink_to_fit();
    tripIndexesByUuid.clear();
    tripConnectionDepartureTimes.clear();
    tripConnectionDepartureTimes.shrink_to_fit();
    tripConnectionDemands.clear();
    tripConnectionDemands.shrink_to_fit();
    connections.clear();
    connections.shrink_to_fit();

    boost::uuids::string_generator uuidGenerator;
    boost::uuids::uuid tripUuid, pathUuid;
    Path * path;
    std::vector<int> pathNodesIdx;
    std::string tripUuidStr, pathUuidStr, cacheFileName;
    int tripIdx;
    unsigned long nodeTimesCount;
    unsigned long linesCount {lines.size()};
    int lineI {0};
    
    spdlog::info("Fetching trips and connections from cache... ({} lines)", linesCount);

    for (auto & lineIter : lines)
    {
      const Line &line = lineIter.second;
      cacheFileName = "lines/line_" + boost::uuids::to_string(line.uuid);
      std::string cacheFilePath = getFilePath(cacheFileName, customPath) + ".capnpbin";
      int fd = open(cacheFilePath.c_str(), O_RDWR);
      if (fd < 0)
      {
        int err = errno;
        spdlog::error("no schedules found for line {} ({} {})", boost::uuids::to_string(line.uuid), line.shortname, line.longname);
        continue;
      }

      try
      {
        ::capnp::PackedFdMessageReader capnpLineMessage(fd, {32 * 1024 * 1024});
        line::Line::Reader capnpLine = capnpLineMessage.getRoot<line::Line>();
        
        const auto schedules {capnpLine.getSchedules()};
        for (const auto & schedule : schedules)
        {
          std::string serviceUuidStr = schedule.getServiceUuid();
          auto & service  = services.at(uuidGenerator(serviceUuidStr));

          const auto periods {schedule.getPeriods()};
          for (const auto & period : periods)
          {
            const auto capnpTrips {period.getTrips()};
            for (const auto & capnpTrip : capnpTrips)
            {
              tripUuidStr  = capnpTrip.getUuid();
              pathUuidStr  = capnpTrip.getPathUuid();
              tripUuid     = uuidGenerator(tripUuidStr);
              pathUuid     = uuidGenerator(pathUuidStr);
              path         = paths[pathIndexesByUuid.at(pathUuid)].get();
              pathNodesIdx = path->nodesIdx;
              
              std::unique_ptr<Trip> trip = std::make_unique<Trip>(tripUuid,
                                                                  line.agency,
                                                                  line,
                                                                  pathIndexesByUuid.at(pathUuid),
                                                                  line.mode,
                                                                  service,
                                                                  line.allowSameLineTransfers,
                                                                  capnpTrip.getTotalCapacity(),
                                                                  capnpTrip.getSeatedCapacity());

              
              tripIdx = trips.size();
              paths[trip->pathIdx].get()->tripsIdx.push_back(tripIdx);
              tripIndexesByUuid[trip->uuid] = tripIdx;

              nodeTimesCount             = capnpTrip.getNodeArrivalTimesSeconds().size();
              auto arrivalTimesSeconds   = capnpTrip.getNodeArrivalTimesSeconds();
              auto departureTimesSeconds = capnpTrip.getNodeDepartureTimesSeconds();
              auto canBoards             = capnpTrip.getNodesCanBoard();
              auto canUnboards           = capnpTrip.getNodesCanUnboard();
              
              std::vector<std::unique_ptr<int>>   connectionDepartureTimes = std::vector<std::unique_ptr<int>>(nodeTimesCount);
              std::vector<std::unique_ptr<float>> connectionDemands        = std::vector<std::unique_ptr<float>>(nodeTimesCount);

              for (int nodeTimeI = 0; nodeTimeI < nodeTimesCount; nodeTimeI++)
              {
                if (nodeTimeI < nodeTimesCount - 1)
                {

                  std::shared_ptr<ConnectionTuple> forwardConnection(std::make_shared<ConnectionTuple>(ConnectionTuple(
                    pathNodesIdx[nodeTimeI],
                    pathNodesIdx[nodeTimeI + 1],
                    departureTimesSeconds[nodeTimeI],
                    arrivalTimesSeconds[nodeTimeI + 1],
                    tripIdx,
                    canBoards[nodeTimeI],
                    canUnboards[nodeTimeI + 1],
                    nodeTimeI + 1,
                    trip->allowSameLineTransfers,
                    line.mode.shortname == Mode::TRANSFERABLE ? 0 : -1
                  )));

                  connections.push_back(std::move(forwardConnection));

                  connectionDepartureTimes[nodeTimeI] = std::make_unique<int>(departureTimesSeconds[nodeTimeI]);
                  connectionDemands[nodeTimeI]        = std::make_unique<float>(0.0);

                }
              }
              trips.push_back(std::move(trip));

              tripConnectionDepartureTimes.push_back(std::move(connectionDepartureTimes));
              tripConnectionDemands.push_back(std::move(connectionDemands));

            }
          }
        }
        lineI++;
        if (lineI % 100 == 0) {
          spdlog::info("Fetching trips and connections from cache...({:.2}%)", ((((double) lineI) / linesCount) * 100));
        }
      }
      catch (const kj::Exception& e)
      {
        // TODO Do something about faulty cache files?
        spdlog::error("-- Error reading line cache file -- {}: {}", cacheFilePath, e.getDescription().cStr());
      }

      close(fd);
    }
    spdlog::info("Fetching trips and connections from cache DONE");

    return 0;

  }

}

#endif // TR_TRIPS_AND_CONNECTIONS_CACHE_FETCHER
