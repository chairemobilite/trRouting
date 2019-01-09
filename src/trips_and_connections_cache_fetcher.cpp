
#ifndef TR_TRIPS_AND_CONNECTIONS_CACHE_FETCHER
#define TR_TRIPS_AND_CONNECTIONS_CACHE_FETCHER

#include <string>
#include <vector>

#include "cache_fetcher.hpp"
#include "trip.hpp"
#include "line.hpp"
#include "block.hpp"
#include "capnp/line.capnp.h"
//#include "toolbox.hpp"

namespace TrRouting
{

  const std::tuple<std::vector<Trip>, std::map<boost::uuids::uuid, int>, std::vector<Block>, std::map<boost::uuids::uuid, int>, std::vector<std::tuple<int,int,int,int,int,short,short,int>>, std::vector<std::tuple<int,int,int,int,int,short,short,int>>> CacheFetcher::getTripsAndConnections(std::map<boost::uuids::uuid, int> agencyIndexesByUuid, std::vector<Line> lines, std::map<boost::uuids::uuid, int> lineIndexesByUuid, std::map<boost::uuids::uuid, int> pathIndexesByUuid, std::map<boost::uuids::uuid, int> nodeIndexesByUuid, std::map<boost::uuids::uuid, int> serviceIndexesByUuid, Parameters& params)
  { 

    std::vector<Trip> trips;
    std::map<boost::uuids::uuid, int> tripIndexesByUuid;
    std::vector<Block> blocks;
    std::map<boost::uuids::uuid, int> blockIndexesByUuid;
    std::vector<std::tuple<int,int,int,int,int,short,short,int>> forwardConnections;
    std::vector<std::tuple<int,int,int,int,int,short,short,int>> reverseConnections; 

    boost::uuids::string_generator uuidGenerator;
    std::string serviceUuidStr;
    boost::uuids::uuid serviceUuid;
    std::string blockUuidStr;
    boost::uuids::uuid blockUuid;
    std::string cacheFileName;
    int serviceIdx;
    int lineIdx;

    std::cout << "Fetching trips and connections from cache..." << std::endl;

    for (auto & line : lines)
    {
      cacheFileName = "lines/line_" + boost::uuids::to_string(line.uuid);
      if (CacheFetcher::capnpCacheFileExists(cacheFileName, params))
      {
        int fd = open((params.cacheDirectoryPath + params.projectShortname + "/" + cacheFileName + ".capnpbin").c_str(), O_RDWR);
        ::capnp::PackedFdMessageReader capnpLineMessage(fd, {32 * 1024 * 1024});
        line::Line::Reader capnpLine = capnpLineMessage.getRoot<line::Line>();
        
        const auto schedules {capnpLine.getSchedules()};
        for (const auto & schedule : schedules)
        {
          serviceUuidStr = schedule.getServiceUuid();
          serviceUuid    = uuidGenerator(serviceUuidStr);
          serviceIdx     = serviceIndexesByUuid[serviceUuid];

          const auto periods {schedule.getPeriods()};
          for (const auto & period : periods)
          {
            const auto capnpTrips {period.getTrips()};
            for (const auto & capnpTrip : capnpTrips)
            {
              std::string uuid     {capnpTrip.getUuid()};
              std::string pathUuid {capnpTrip.getPathUuid()};
              Trip * trip                  = new Trip();
              trip->uuid                   = uuidGenerator(uuid);
              trip->agencyIdx              = line.agencyIdx;
              trip->lineIdx                = lineIndexesByUuid[line.uuid];
              trip->pathIdx                = pathIndexesByUuid[uuidGenerator(pathUuid)];
              trip->modeIdx                = line.modeIdx;
              trip->serviceIdx             = serviceIdx;
              trip->totalCapacity          = capnpTrip.getTotalCapacity();
              trip->seatedCapacity         = capnpTrip.getSeatedCapacity();
              trip->allowSameLineTransfers = line.allowSameLineTransfers;

              blockUuidStr = capnpTrip.getBlockUuid();
              if (blockUuidStr.length() > 0) // if block does not exist yet
              {
                blockUuid = uuidGenerator(blockUuidStr);
                if (blockIndexesByUuid.count(blockUuid) != 1)
                {
                  // create new block:
                  Block * block = new Block();
                  block->uuid = blockUuid;
                  blocks.push_back(*block);
                  blockIndexesByUuid[block->uuid] = blocks.size() - 1;
                }
                trip->blockIdx = blockIndexesByUuid[blockUuid];
              }
              else
              {
                trip->blockIdx = -1;
              }

              trips.push_back(*trip);
              tripIndexesByUuid[trip->uuid] = trips.size() - 1;
            }
          }
        }
        close(fd);
      }
      else
      {
        std::cerr << "missing schedules cache file for line " << boost::uuids::to_string(line.uuid) << " !" << std::endl;
      }
    }

    return std::make_tuple(trips, tripIndexesByUuid, blocks, blockIndexesByUuid, forwardConnections, reverseConnections);
  }

}

#endif // TR_TRIPS_AND_CONNECTIONS_CACHE_FETCHER
