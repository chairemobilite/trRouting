
#ifndef TR_TRIPS_AND_CONNECTIONS_CACHE_FETCHER
#define TR_TRIPS_AND_CONNECTIONS_CACHE_FETCHER

#include <string>
#include <vector>
#include <errno.h>
#include <kj/exception.h>

#include "cache_fetcher.hpp"
#include "trip.hpp"
#include "line.hpp"
#include "block.hpp"
#include "capnp/line.capnp.h"
#include "calculation_time.hpp"
//#include "toolbox.hpp"

namespace TrRouting
{
  
  int CacheFetcher::getSchedules(
    std::vector<std::unique_ptr<Trip>>& trips,
    std::vector<std::unique_ptr<Line>>& lines,
    std::vector<std::unique_ptr<Path>>& paths,
    std::map<boost::uuids::uuid, int>& tripIndexesByUuid,
    std::map<boost::uuids::uuid, int>& serviceIndexesByUuid,
    std::map<boost::uuids::uuid, int>& lineIndexesByUuid,
    std::map<boost::uuids::uuid, int>& pathIndexesByUuid,
    std::map<boost::uuids::uuid, int>& agencyIndexesByUuid,
    std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
    std::map<std::string, int>& modeIndexesByShortname,
    std::vector<std::vector<std::unique_ptr<int>>>&   tripConnectionDepartureTimes,
    std::vector<std::vector<std::unique_ptr<float>>>& tripConnectionDemands,
    std::vector<std::shared_ptr<std::tuple<int,int,int,int,int,short,short,int,int,int,short,short>>>& connections, 
    Parameters& params,
    std::string customPath
  )
  {
    // FIXME ConnectionTuple is defined in calculator.hpp. Should it be elsewhere? Cache fetcher should work for any algorithm, is this tuple csa-specific or should it go somewhere common.
    using ConnectionTuple = std::tuple<int,int,int,int,int,short,short,int,int,int,short,short>;

    trips.clear();
    trips.shrink_to_fit();
    tripIndexesByUuid.clear();
    tripConnectionDepartureTimes.clear();
    tripConnectionDepartureTimes.shrink_to_fit();
    tripConnectionDemands.clear();
    tripConnectionDemands.shrink_to_fit();
    connections.clear();
    connections.shrink_to_fit();

    int transferableModeIdx {modeIndexesByShortname.find("transferable") != modeIndexesByShortname.end() ? modeIndexesByShortname["transferable"] : -1};

    //std::vector<Block> blocks;
    //std::map<boost::uuids::uuid, int> blockIndexesByUuid;
    boost::uuids::string_generator uuidGenerator;
    boost::uuids::uuid tripUuid, pathUuid, serviceUuid; //blockUuid;
    Path * path;
    std::vector<int> pathNodesIdx;
    std::string tripUuidStr, pathUuidStr, serviceUuidStr, cacheFileName; // blockUuidStr
    int serviceIdx, lineIdx, tripIdx;
    unsigned long nodeTimesCount;
    unsigned long linesCount {lines.size()};
    int lineI {0};
    
    std::cout << "Fetching trips and connections from cache..." << std::endl;

    std::cout << std::fixed;
    std::cout << std::setprecision(2);

    for (auto & line : lines)
    {
      cacheFileName = "lines/line_" + boost::uuids::to_string(line->uuid);
      std::string cacheFilePath = CacheFetcher::getFilePath(cacheFileName, params, customPath) + ".capnpbin";
      int fd = open(cacheFilePath.c_str(), O_RDWR);
      if (fd < 0)
      {
        int err = errno;
        std::cerr << "no schedules found for line " << boost::uuids::to_string(line->uuid) << " (" << line->shortname << " " << line->longname << ")" << std::endl;
        continue;
      }

      try
      {
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
              tripUuidStr  = capnpTrip.getUuid();
              pathUuidStr  = capnpTrip.getPathUuid();
              tripUuid     = uuidGenerator(tripUuidStr);
              pathUuid     = uuidGenerator(pathUuidStr);
              path         = paths[pathIndexesByUuid[pathUuid]].get();
              pathNodesIdx = path->nodesIdx;
              
              std::unique_ptr<Trip> trip = std::make_unique<Trip>();

              trip->uuid                   = tripUuid;
              trip->agencyIdx              = line->agencyIdx;
              trip->lineIdx                = lineIndexesByUuid[line->uuid];
              trip->pathIdx                = pathIndexesByUuid[pathUuid];
              trip->modeIdx                = line->modeIdx;
              trip->serviceIdx             = serviceIdx;
              trip->totalCapacity          = capnpTrip.getTotalCapacity();
              trip->seatedCapacity         = capnpTrip.getSeatedCapacity();
              trip->allowSameLineTransfers = line->allowSameLineTransfers;

              /*blockUuidStr = capnpTrip.getBlockUuid();
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
              }*/

              trip->blockIdx = -1;

              /**/

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
                    trip->lineIdx,
                    trip->blockIdx,
                    trip->allowSameLineTransfers,
                    transferableModeIdx == line->modeIdx ? 0 : -1
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
        std::cout << ((((double) lineI) / linesCount) * 100) << "%      \r" << std::flush; // \r is used to stay on the same line
      }
      catch (const kj::Exception& e)
      {
        // TODO Do something about faulty cache files?
        std::cerr << "-- Error reading line cache file -- " <<  cacheFilePath << ": " << e.getDescription().cStr() << std::endl;
      }

      close(fd);
    }
    std::cout << "100%" << std::endl;

    return 0;

  }

}

#endif // TR_TRIPS_AND_CONNECTIONS_CACHE_FETCHER
