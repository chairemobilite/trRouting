#include "trip_based_algorithm.hpp"


namespace TrRouting
{
  
  // (Algorithm 4)
  json TripBasedAlgorithm::calculate()
  {

    std::cout << "start calculation" << std::endl;

    // prepare json content and refresh variables for calculation:
    json jsonResult;
    jsonResult["stops"] = json::array();
    refresh();
    resetAccessEgressModes();
    int nMax      = params.maxNumberOfTransfers;
    int infinite  = 99999;
    int srcStopI  = stopsIndexById[params.startingStopId];
    int startTime = params.departureTimeHour * 3600 + params.departureTimeMinutes * 60;
    

    // prepare target stop indexes to calculate according to returnAllStopsResult param value:
    std::vector<int> tgtStopIs;
    if (params.returnAllStopsResult)
    {
      tgtStopIs = std::vector<int>(stops.size());
      // fill the target stop indexes vector with increasing values from 0 to the number of stop indexes - 1:
      std::iota(std::begin(tgtStopIs), std::end(tgtStopIs), 0);
    }
    else
    {
      tgtStopIs = std::vector<int>(1,stopsIndexById[params.endingStopId]);
    }


    // calculate for each stop index in the target stop indexes vector:
    for(const auto & tgtStopI : tgtStopIs)
    {
      std::vector<int>                      tauMin(nMax + 1, infinite); // set arrival time to infinite (line 1: we replace J by a vector tauMin including min arrival time for each number of transfers n)
      std::vector<ReachableRoutePath>       reachableRoutePaths = std::vector<ReachableRoutePath>(); // (line 2)
      std::vector<std::vector<TripSegment>> tripSegments = std::vector<std::vector<TripSegment>>(7); // (line 3)
      std::vector<TripSegment> shortestJourneyTripSegment = std::vector<TripSegment>(7);
      std::vector<int> firstReachedStopSeqByTrip(trips[params.weekdayIndex].size() - 1, infinite); // (line 4)

      Footpath footpathToTarget;
      int stopToTargetI; // q
      int travelTimeToTarget;


      // fetch footpaths to target stop:
      for (auto it = footpathsByTarget.begin() + footpathsIndex[tgtStopI][0]; it != footpathsByTarget.begin() + footpathsIndex[tgtStopI][1] + 1; ++it) // (line 5)
      {
        footpathToTarget   = *it;
        stopToTargetI      = footpathToTarget.srcI;
        travelTimeToTarget = footpathToTarget.tt; // (line 6)
        for (const auto & routePathIndexAndStopSeq : routePathsIndexByStop[stopToTargetI]) // (line 7)
        {
          reachableRoutePaths.push_back(*(new ReachableRoutePath(routePathIndexAndStopSeq[0], routePathIndexAndStopSeq[1], travelTimeToTarget))); // (line 8)
        }
      }

      Footpath footpathFromSource;
      int  stopFromSourceI; // q
      int  readyTimeFromSource;
      int  routePathIndex;
      bool foundEarliestTrip;
      int  tripDepartureTimeAtStop;
      Trip trip;
      int  stopSeq;
      int  arrivalTimeAtNextStop;


      // fetch footpaths from source stop:
      for (auto it = footpathsBySource.begin() + footpathsIndex[srcStopI][0]; it != footpathsBySource.begin() + footpathsIndex[srcStopI][1] + 1; ++it) // (line 9)
      {
        footpathFromSource  = *it;
        stopFromSourceI     = footpathFromSource.srcI;
        readyTimeFromSource = footpathFromSource.tt + params.minWaitingTimeMinutes * 60; // (line 10)


        for (const auto & routePathIndexAndStopSeq : routePathsIndexByStop[stopFromSourceI]) // (line 11)
        {
          routePathIndex    = routePathIndexAndStopSeq[0];
          stopSeq           = routePathIndexAndStopSeq[1];


          if (tripsIndex[params.weekdayIndex][routePathIndex][0] != -1)
          {
            foundEarliestTrip = false;


            for (auto it = trips[params.weekdayIndex].begin() + tripsIndex[params.weekdayIndex][routePathIndex][0]; it != trips[params.weekdayIndex].begin() + tripsIndex[params.weekdayIndex][routePathIndex][1] + 1; ++it)
            {
              trip = *it;
              tripDepartureTimeAtStop = departureTimes[params.weekdayIndex][departureTimesIndex[params.weekdayIndex][trip.i][0] + stopSeq];
              if (!foundEarliestTrip)
              {
                if (startTime + readyTimeFromSource <= tripDepartureTimeAtStop) // (line 12)
                {
                  foundEarliestTrip = true;
                  if(stopSeq < firstReachedStopSeqByTrip[trip.i]) // (enqueue line 2)
                  {
                    tripSegments[0].push_back(*(new TripSegment(routePathIndex, trip.i, stopSeq, firstReachedStopSeqByTrip[trip.i]))); // (enqueue line 3)
                  }
                  else
                  {
                    break;
                  }
                }
              }
              else // (enqueue line 4)
              {
                firstReachedStopSeqByTrip[trip.i] = std::min(stopSeq, firstReachedStopSeqByTrip[trip.i]); // (enqueue line 5)
              }
            }
          }
        }
      }


      for(int n = 0; n <= nMax + 1; n++) // n : number of transfers (line 16)
      {
        for(auto & tripSegment : tripSegments[n]) // (line 17)
        {
          int routeIndex   = tripSegment.rpI;
          int firstStopSeq = tripSegment.firstStopSeq;
          int lastStopSeq  = std::min((unsigned long long)tripSegment.lastStopSeq, (unsigned long long)(stopsIndexByRoutePath[routeIndex].size() - 1));
          int tripI        = tripSegment.tripI;
          int newArrivalTime;
          int nextStopSeqArrivalTimeIndex;
          int arrivalTimeIndex;
          int transfersFirstIndex;
          Trip transferTrip;
          Transfer transfer;


          for(auto & reachableRoutePath : reachableRoutePaths) // (line 18)
          {
            if(reachableRoutePath.rpI == routeIndex && firstStopSeq < reachableRoutePath.stopSeq)
            {
              newArrivalTime = arrivalTimes[params.weekdayIndex][arrivalTimesIndex[params.weekdayIndex][tripI][0] + reachableRoutePath.stopSeq] + reachableRoutePath.tt;
              if(newArrivalTime < tauMin[n])
              {
                tauMin[n]                     = newArrivalTime; // (line 19)
                shortestJourneyTripSegment[n] = tripSegment; // (line 20)
              }
            }
          }


          nextStopSeqArrivalTimeIndex = arrivalTimesIndex[params.weekdayIndex][tripI][0] + firstStopSeq + 1;
          if (nextStopSeqArrivalTimeIndex <= arrivalTimesIndex[params.weekdayIndex][tripI][1])
          {
            arrivalTimeAtNextStop = arrivalTimes[params.weekdayIndex][nextStopSeqArrivalTimeIndex];
            if(arrivalTimeAtNextStop < tauMin[n]) // (line 21)
            {

              for (int stopSeq = firstStopSeq; stopSeq <= lastStopSeq; stopSeq++)
              {
                arrivalTimeIndex    = arrivalTimesIndex[params.weekdayIndex][tripI][0] + stopSeq;
                transfersFirstIndex = transfersIndex[params.weekdayIndex][arrivalTimeIndex][0];
                if(transfersFirstIndex != -1)
                {
                  for (auto it = transfers[params.weekdayIndex].begin() + transfersFirstIndex; it != transfers[params.weekdayIndex].begin() + transfersIndex[params.weekdayIndex][arrivalTimeIndex][1] + 1; ++it)
                  {
                    transfer = *it;
                    if (transfer.tgtStopSeq < firstReachedStopSeqByTrip[transfer.tgtTripI]) // (enqueue line 2)
                    {
                      transferTrip = trips[params.weekdayIndex][transfer.tgtTripI];
                      tripSegments[n + 1].push_back(*(new TripSegment(transferTrip.rpI, transfer.tgtTripI, transfer.tgtStopSeq, firstReachedStopSeqByTrip[transfer.tgtTripI], tripSegment.prevTripSegments, tripSegment))); // (enqueue line 3)
                      for(int routePathTripI = tripsIndex[params.weekdayIndex][transferTrip.rpI][0] + transferTrip.seq; routePathTripI <= tripsIndex[params.weekdayIndex][transferTrip.rpI][1]; routePathTripI++) // (enqueue line 4)
                      {
                        firstReachedStopSeqByTrip[routePathTripI] = std::min(transfer.tgtStopSeq, firstReachedStopSeqByTrip[routePathTripI]); // (enqueue line 5)
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }

      int minArrivalTimeSecondForStop = infinite;
      for(int n = 0; n <= nMax; n++)
      {
        if(tauMin[n] < minArrivalTimeSecondForStop)
        {
          minArrivalTimeSecondForStop = tauMin[n];
        }
      }

      json stopResult = {{"stopId", stops[tgtStopI].id}, {"arrivalTimeSeconds", minArrivalTimeSecondForStop}, {"travelTimeSeconds", (minArrivalTimeSecondForStop == infinite ? infinite : minArrivalTimeSecondForStop - startTime) }};
      jsonResult["stops"].push_back(stopResult);

    }

    return jsonResult;
  }




}