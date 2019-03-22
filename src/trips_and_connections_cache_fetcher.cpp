
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

  const std::tuple<std::vector<Trip>, std::map<boost::uuids::uuid, int>, std::vector<std::vector<int>>, std::vector<std::vector<float>>, std::vector<Block>, std::map<boost::uuids::uuid, int>, std::vector<std::tuple<int,int,int,int,int,short,short,int,int,int,short>>, std::vector<std::tuple<int,int,int,int,int,short,short,int,int,int,short>>> CacheFetcher::getTripsAndConnections(std::map<boost::uuids::uuid, int> agencyIndexesByUuid, std::vector<Line> lines, std::map<boost::uuids::uuid, int> lineIndexesByUuid, std::vector<Path> paths, std::map<boost::uuids::uuid, int> pathIndexesByUuid, std::map<boost::uuids::uuid, int> nodeIndexesByUuid, std::map<boost::uuids::uuid, int> serviceIndexesByUuid, Parameters& params)
  { 

    std::vector<Trip> trips;
    std::vector<Block> blocks;
    std::vector<std::vector<int>> tripConnectionDepartureTimes;
    std::vector<std::vector<float>> tripConnectionDemands;
    std::map<boost::uuids::uuid, int> tripIndexesByUuid, blockIndexesByUuid;
    std::vector<std::tuple<int,int,int,int,int,short,short,int,int,int,short>> forwardConnections, reverseConnections;
    boost::uuids::string_generator uuidGenerator;
    boost::uuids::uuid tripUuid, pathUuid, blockUuid, serviceUuid;
    Path path;
    std::vector<int> pathNodesIdx;
    std::string tripUuidStr, pathUuidStr, blockUuidStr, serviceUuidStr, cacheFileName;
    int serviceIdx, lineIdx, tripIdx;
    unsigned long nodeTimesCount;
    unsigned long linesCount {lines.size()};
    int lineI {0};

    std::cout << "Fetching trips and connections from cache..." << std::endl;

    std::cout << std::fixed;
    std::cout << std::setprecision(2);

    for (auto & line : lines)
    {
      cacheFileName = "lines/line_" + boost::uuids::to_string(line.uuid);
      
      if (CacheFetcher::capnpCacheFileExists(cacheFileName + ".capnpbin", params))
      {
        int fd = open((CacheFetcher::getFilePath(cacheFileName, params) + ".capnpbin").c_str(), O_RDWR);
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
              path         = paths[pathIndexesByUuid[pathUuid]];
              pathNodesIdx = path.nodesIdx;
              
              Trip * trip                  = new Trip();
              trip->uuid                   = tripUuid;
              trip->agencyIdx              = line.agencyIdx;
              trip->lineIdx                = lineIndexesByUuid[line.uuid];
              trip->pathIdx                = pathIndexesByUuid[pathUuid];
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
              tripIdx = trips.size() - 1;
              paths[trip->pathIdx].tripsIdx.push_back(tripIdx);
              tripIndexesByUuid[trip->uuid] = tripIdx;

              nodeTimesCount = capnpTrip.getNodeArrivalTimesSeconds().size();
              auto arrivalTimesSeconds   = capnpTrip.getNodeArrivalTimesSeconds();
              auto departureTimesSeconds = capnpTrip.getNodeDepartureTimesSeconds();
              auto canBoards             = capnpTrip.getNodesCanBoard();
              auto canUnboards           = capnpTrip.getNodesCanUnboard();
              
              std::vector<int>   connectionDepartureTimes(nodeTimesCount);
              std::vector<float> connectionDemands(nodeTimesCount);

              for (int nodeTimeI = 0; nodeTimeI < nodeTimesCount; nodeTimeI++)
              {
                if (nodeTimeI < nodeTimesCount - 1)
                {

                  forwardConnections.push_back(std::make_tuple(
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
                    trip->allowSameLineTransfers
                  ));

                  connectionDepartureTimes[nodeTimeI] = departureTimesSeconds[nodeTimeI];
                  connectionDemands[nodeTimeI]        = 0.0;

                }
              }

              tripConnectionDepartureTimes.push_back(connectionDepartureTimes);
              tripConnectionDemands.push_back(connectionDemands);

            }
          }
        }
        close(fd);
        lineI++;
        std::cout << ((((double) lineI) / linesCount) * 100) << "%      \r" << std::flush; // \r is used to stay on the same line
      }
      else
      {
        std::cerr << "missing schedules cache file for line " << boost::uuids::to_string(line.uuid) << " !" << std::endl;
      }
    }
    std::cout << "100%         " << std::endl;

    std::cout << "Sorting connections..." << std::endl;
    std::stable_sort(forwardConnections.begin(), forwardConnections.end(), [](std::tuple<int,int,int,int,int,short,short,int,int,int,short> connectionA, std::tuple<int,int,int,int,int,short,short,int,int,int,short> connectionB)
    {
      // { NODE_DEP = 0, NODE_ARR = 1, TIME_DEP = 2, TIME_ARR = 3, TRIP = 4, CAN_BOARD = 5, CAN_UNBOARD = 6, SEQUENCE = 7, LINE = 8, BLOCK = 9, CAN_TRANSFER_SAME_LINE = 10 };
      if (std::get<2>(connectionA) < std::get<2>(connectionB))
      {
        return true;
      }
      else if (std::get<2>(connectionA) > std::get<2>(connectionB))
      {
        return false;
      }
      if (std::get<4>(connectionA) < std::get<4>(connectionB))
      {
        return true;
      }
      else if (std::get<4>(connectionA) > std::get<4>(connectionB))
      {
        return false;
      }
      if (std::get<7>(connectionA) < std::get<7>(connectionB))
      {
        return true;
      }
      else if (std::get<7>(connectionA) > std::get<7>(connectionB))
      {
        return false;
      }
      return false;
    });
    reverseConnections = forwardConnections;
    std::stable_sort(reverseConnections.begin(), reverseConnections.end(), [](std::tuple<int,int,int,int,int,short,short,int,int,int,short> connectionA, std::tuple<int,int,int,int,int,short,short,int,int,int,short> connectionB)
    {
      // { NODE_DEP = 0, NODE_ARR = 1, TIME_DEP = 2, TIME_ARR = 3, TRIP = 4, CAN_BOARD = 5, CAN_UNBOARD = 6, SEQUENCE = 7, LINE = 8, BLOCK = 9, CAN_TRANSFER_SAME_LINE = 10 };
      if (std::get<3>(connectionA) > std::get<3>(connectionB))
      {
        return true;
      }
      else if (std::get<3>(connectionA) < std::get<3>(connectionB))
      {
        return false;
      }
      if (std::get<4>(connectionA) > std::get<4>(connectionB)) // here we need to reverse sequence!
      {
        return true;
      }
      else if (std::get<4>(connectionA) < std::get<4>(connectionB))
      {
        return false;
      }
      if (std::get<7>(connectionA) > std::get<7>(connectionB)) // here we need to reverse sequence!
      {
        return true;
      }
      else if (std::get<7>(connectionA) < std::get<7>(connectionB))
      {
        return false;
      }
      return false;
    });

    //for (auto & connection : forwardConnections)
    //{
    //  std::cout 
    //  << " nD" << std::get<0>(connection) 
    //  << " nA" << std::get<1>(connection) 
    //  << " D" << std::get<2>(connection) 
    //  << " A" << std::get<3>(connection) 
    //  << " T" << std::get<4>(connection) 
    //  << " B" << std::get<5>(connection) 
    //  << " U" << std::get<6>(connection) 
    //  << " S" << std::get<7>(connection) 
    //  << std::endl;
    //}

    return std::make_tuple(trips, tripIndexesByUuid, tripConnectionDepartureTimes, tripConnectionDemands, blocks, blockIndexesByUuid, forwardConnections, reverseConnections);
  }

}

#endif // TR_TRIPS_AND_CONNECTIONS_CACHE_FETCHER
