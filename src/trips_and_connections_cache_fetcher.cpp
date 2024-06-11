
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
#include "path.hpp"
#include "capnp/line.capnp.h"
#include "calculation_time.hpp"

namespace TrRouting
{
  int CacheFetcher::getSchedules(
    std::map<boost::uuids::uuid, Trip>& trips,
    const std::map<boost::uuids::uuid, Line>& lines,
    std::map<boost::uuids::uuid, Path>& paths,
    const std::map<boost::uuids::uuid, Service>& services,
    std::vector<Connection>& connections,
    std::string customPath
  )
  {
    trips.clear();
    connections.clear();
    connections.shrink_to_fit();

    boost::uuids::string_generator uuidGenerator;
    boost::uuids::uuid tripUuid, pathUuid;
    std::string tripUuidStr, pathUuidStr, cacheFileName;
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
              Path &path = paths.at(pathUuid);
              
              trips.emplace(tripUuid, Trip(tripUuid,
                                           line.agency,
                                           line,
                                           path,
                                           line.mode,
                                           service,
                                           line.allowSameLineTransfers,
                                           capnpTrip.getTotalCapacity(),
                                           capnpTrip.getSeatedCapacity()));
              //Current trip
              Trip & trip = trips.at(tripUuid);
              
              // TODO This should probably be done in the Trip constructor (setting the back reference)
              path.tripsRef.push_back(trip);

              nodeTimesCount             = capnpTrip.getNodeArrivalTimesSeconds().size();
              auto arrivalTimesSeconds   = capnpTrip.getNodeArrivalTimesSeconds();
              auto departureTimesSeconds = capnpTrip.getNodeDepartureTimesSeconds();
              auto canBoards             = capnpTrip.getNodesCanBoard();
              auto canUnboards           = capnpTrip.getNodesCanUnboard();
              trip.connectionDepartureTimes.resize(nodeTimesCount);
              // nodeTimesCount - 1, since we process node pairs, we have to stop and the second from last
              for (unsigned long nodeTimeI = 0; nodeTimeI < nodeTimesCount - 1; nodeTimeI++)
              {

                try {
                  connections.push_back(Connection(
                    path.nodesRef.at(nodeTimeI).get(),
                    path.nodesRef.at(nodeTimeI + 1).get(),
                    departureTimesSeconds[nodeTimeI],
                    arrivalTimesSeconds[nodeTimeI + 1],
                    trip,
                    canBoards[nodeTimeI] == 1,
                    canUnboards[nodeTimeI + 1] == 1,
                    nodeTimeI + 1,
                    trip.allowSameLineTransfers,
                    line.mode.isTransferable() ? 0 : -1
                  ));

                  trip.connectionDepartureTimes[nodeTimeI] = departureTimesSeconds[nodeTimeI];
                } catch (std::out_of_range const& exc) {
                  spdlog::error("Index out of range while parsing connection for trip on line ({})", path.line.longname);
                  return -1;
                }
              }
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
