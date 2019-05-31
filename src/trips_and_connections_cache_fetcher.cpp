
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

  void CacheFetcher::getSchedules(
    std::vector<std::unique_ptr<Trip>>& trips,
    std::vector<std::unique_ptr<Path>>& paths,
    std::map<boost::uuids::uuid, int>& tripIndexesById,
    std::map<boost::uuids::uuid, int>& serviceIndexesByUuid,
    std::map<boost::uuids::uuid, int>& lineIndexesByUuid,
    std::map<boost::uuids::uuid, int>& agencyIndexesByUuid,
    std::map<boost::uuids::uuid, int>& nodeIndexesByUuid,
    std::map<std::string, int>& modeIndexesByShortname,
    std::vector<std::unique_ptr<std::vector<int>>>&   tripConnectionDepartureTimes,
    std::vector<std::unique_ptr<std::vector<float>>>& tripConnectionDemands,
    std::vector<std::shared_ptr<std::tuple<int,int,int,int,int,short,short,int,int,int,short>>>& forwardConnections, 
    std::vector<std::shared_ptr<std::tuple<int,int,int,int,int,short,short,int,int,int,short>>>& reverseConnections,
    Parameters& params,
    std::string customPath = ""
  )
  {

    trips.clear();
    tripsIndexesByUuid.clear();
    tripConnectionDepartureTimes.clear();
    tripConnectionDemands.clear();
    forwardConnections.clear();
    reverseConnections.clear();

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
              tripIdx = trips.size();
              paths[trip->pathIdx].get()->tripsIdx.push_back(tripIdx);
              tripIndexesByUuid[trip->uuid] = tripIdx;
              trips.push_back(std::move(trip));

              nodeTimesCount             = capnpTrip.getNodeArrivalTimesSeconds().size();
              auto arrivalTimesSeconds   = capnpTrip.getNodeArrivalTimesSeconds();
              auto departureTimesSeconds = capnpTrip.getNodeDepartureTimesSeconds();
              auto canBoards             = capnpTrip.getNodesCanBoard();
              auto canUnboards           = capnpTrip.getNodesCanUnboard();
              
              std::vector<int>   connectionDepartureTimes = std::make_unique(std::vector<int>(nodeTimesCount));
              std::vector<float> connectionDemands        = std::make_unique(std::vector<float>(nodeTimesCount));

              for (int nodeTimeI = 0; nodeTimeI < nodeTimesCount; nodeTimeI++)
              {
                if (nodeTimeI < nodeTimesCount - 1)
                {
                  
                  std::shared_ptr<std::tuple<int,int,int,int,int,short,short,int,int,int,short>> forwardConnection(std::make_shared(std::make_tuple(
                    pathNodesIdx[nodeTimeI],
                    pathNodesIdx[nodeTimeI + 1],
                    departureTimesSeconds[nodeTimeI],
                    arrivalTimesSeconds[nodeTimeI + 1],
                    tripIdx,
                    canBoards[nodeTimeI],
                    canUnboards[nodeTimeI + 1],
                    nodeTimeI + 1,
                    trip->lineIdx,
                    //trip->blockIdx,
                    trip->allowSameLineTransfers
                  )));
                  std::shared_ptr<std::tuple<int,int,int,int,int,short,short,int,int,int,short>> reverseConnection = forwardConnection;

                  forwardConnections.push_back(std::move(forwardConnection));
                  reverseConnections.push_back(std::move(reverseConnection));

                  connectionDepartureTimes[nodeTimeI] = departureTimesSeconds[nodeTimeI];
                  connectionDemands[nodeTimeI]        = 0.0;

                }
              }

              tripConnectionDepartureTimes.push_back(std::move(connectionDepartureTimes));
              tripConnectionDemands.push_back(std::move(connectionDemands));

            }
          }
        }
        close(fd);
        lineI++;
        std::cout << ((((double) lineI) / linesCount) * 100) << "%      \r" << std::flush; // \r is used to stay on the same line
      }
      else
      {
        std::cerr << "no schedules found for line " << boost::uuids::to_string(line->uuid) << " (" << line->shortname << " " << line->longname << ")" << std::endl;
      }
    }
    std::cout << "100%         " << std::endl;

    std::cout << "Sorting connections..." << std::endl;
    std::stable_sort(forwardConnections.begin(), forwardConnections.end(), [](std::tuple<int,int,int,int,int,short,short,int,int,int,short> connectionA *, std::tuple<int,int,int,int,int,short,short,int,int,int,short> connectionB *)
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
    std::stable_sort(reverseConnections.begin(), reverseConnections.end(), [](std::tuple<int,int,int,int,int,short,short,int,int,int,short> connectionA *, std::tuple<int,int,int,int,int,short,short,int,int,int,short> connectionB *)
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

    //return std::make_tuple(trips, tripIndexesByUuid, tripConnectionDepartureTimes, tripConnectionDemands, blocks, blockIndexesByUuid, forwardConnections, reverseConnections);
  }

}

#endif // TR_TRIPS_AND_CONNECTIONS_CACHE_FETCHER
