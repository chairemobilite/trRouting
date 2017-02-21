#include "trip_based_algorithm.hpp"


namespace TrRouting
{
  
  // (Algorithm 4)
  json TripBasedAlgorithm::calculate()
  {
    
    //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    //std::cerr << "A" << std::endl;
    
    
    algorithmCalculationTime.start();
    
    
    //std::cerr << "start calculation" << std::endl;
    
    //std::cerr << "A1" << std::endl;
    
    // prepare json content and refresh variables for calculation:
    json jsonResult;
    
    //std::cerr << "A2" << std::endl;
    
    //refresh();
    
    //std::cerr << "A3" << std::endl;
    
    resetAccessEgressModes();
    
    //std::cerr << "A4" << std::endl;
    
    int nMax      = params.maxNumberOfTransfers;
    int infinite  = 999999; // this is more than 11 days
    int srcStopI;
    int startTime = params.departureTimeHour * 3600 + params.departureTimeMinutes * 60;
    
    //std::cerr << "A5" << std::endl;
    
    // return failed status if starting stop not valid:
    if (params.startingStopId < 0 || params.startingStopId > stopsIndexById.size() - 1 || stopsIndexById[params.startingStopId] == -1)
    {
      json dumpJson(stopsIndexById);
      jsonResult["status"] = "failed because starting stop id is not valid or does not exist (id = " + std::to_string(params.startingStopId) + ")";
      return jsonResult;
    }
    else
    {
      srcStopI  = stopsIndexById[params.startingStopId];
    }
    
    //std::cerr << "B" << std::endl;
    
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
      // return failed status if ending stop not valid:
      if (params.endingStopId < 0 || params.endingStopId > stopsIndexById.size() - 1 || stopsIndexById[params.endingStopId] == -1)
      {
        jsonResult["status"] = "failed because ending stop id is not valid or does not exist (id = " + std::to_string(params.endingStopId) + ")";
        return jsonResult;
      }
      tgtStopIs = std::vector<int>(1,stopsIndexById[params.endingStopId]);
    }
    
    //std::cerr << "C" << std::endl;
    
    jsonResult["stops"] = json::array();

    std::vector<int>                       tauMin(nMax + 1, infinite); // set arrival time to infinite (line 1: we replace J by a vector tauMin including min arrival time for each number of transfers n)
    std::vector<ReachableRoutePath*>       reachableRoutePaths = std::vector<ReachableRoutePath*>(); // (line 2)
    std::vector<std::vector<TripSegment*>> tripSegments(nMax + 2, std::vector<TripSegment*>()); // (line 3)
    std::vector<TripSegment*>              shortestJourneyTripSegment(nMax + 2);
    std::vector<int>                       firstReachedStopSeqByTrip(trips[params.weekdayIndex].size(), infinite); // (line 4)
    std::vector<int>                       allStopsfirstReachedStopSeqByTrip(trips[params.weekdayIndex].size(), infinite);
    
    //std::cerr << "a" << std::endl;
    
    
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
      
      if(stopFromSourceI == -1)
      {
        continue;
      }
      
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
                  tripSegments[0].push_back(new TripSegment(routePathIndex, trip.i, stopSeq, std::min((int)(stopsIndexByRoutePath[routePathIndex].size()-1), firstReachedStopSeqByTrip[trip.i]))); // (enqueue line 3)
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

    allStopsfirstReachedStopSeqByTrip = firstReachedStopSeqByTrip;
    
    //std::cerr << "b" << std::endl;

    // calculate for each stop index in the target stop indexes vector:
    for(const auto & tgtStopI : tgtStopIs)
    {
      
      //std::cerr << "c" << std::endl;
      
      //std::vector<std::vector<TripSegment>> tripSegments(nMax + 1, std::vector<TripSegment>()); // (line 3)
      reachableRoutePaths.clear();
      firstReachedStopSeqByTrip = allStopsfirstReachedStopSeqByTrip; // reset for each stop (needed!)
      
      //std::cerr << "c1" << std::endl;

      //std::vector<int>                      tauMin(nMax + 1, infinite); // set arrival time to infinite (line 1: we replace J by a vector tauMin including min arrival time for each number of transfers n)
      //std::vector<ReachableRoutePath>       reachableRoutePaths = std::vector<ReachableRoutePath>(); // (line 2)
      //std::vector<std::vector<TripSegment>> tripSegments(nMax + 1, std::vector<TripSegment>()); // (line 3)
      //std::vector<TripSegment> shortestJourneyTripSegment(nMax + 1, TripSegment());
      //std::vector<int> firstReachedStopSeqByTrip(trips[params.weekdayIndex].size(), infinite); // (line 4)
      
      Footpath footpathToTarget;
      int stopToTargetI; // q
      int travelTimeToTarget;
      
      //std::cerr << "c2" << std::endl;
      
      // fetch footpaths to target stop:
      for (auto it = footpathsByTarget.begin() + footpathsIndex[tgtStopI][0]; it != footpathsByTarget.begin() + footpathsIndex[tgtStopI][1] + 1; ++it) // (line 5)
      {
        
        //std::cerr << "c3" << std::endl;
        
        footpathToTarget   = *it;
        stopToTargetI      = footpathToTarget.srcI;
        travelTimeToTarget = footpathToTarget.tt; // (line 6)
        
        //std::cerr << "srcI:" << stopToTargetI << " tt:" << travelTimeToTarget << std::endl;
        
        //std::cerr << "c4" << std::endl;
        
        if(stopToTargetI == -1)
        {
          continue;
        }
        
        for (const auto & routePathIndexAndStopSeq : routePathsIndexByStop[stopToTargetI]) // (line 7)
        {
          
          //std::cerr << "routePathIndexAndStopSeq:" << routePathIndexAndStopSeq[0] << " | " << routePathIndexAndStopSeq[1] << std::endl;
          
          //std::cerr << "c5" << std::endl;
          
          reachableRoutePaths.push_back(new ReachableRoutePath(routePathIndexAndStopSeq[0], routePathIndexAndStopSeq[1], travelTimeToTarget)); // (line 8)
          
          //std::cerr << "c6" << std::endl;
        }
      }
      
      //std::cerr << "d" << std::endl;
      
      for(int n = 0; n <= nMax; n++) // n : number of transfers (line 16)
      {
        //std::cout << "stopId=" << stops[tgtStopI].id << " n=" << n << " tsegCount=" << tripSegments[n].size() << std::endl;
        for(const auto & tripSegment : tripSegments[n]) // (line 17)
        {
          int routeIndex   = tripSegment->rpI;
          int firstStopSeq = tripSegment->firstStopSeq;
          int lastStopSeq  = std::min((int)tripSegment->lastStopSeq, (int)(stopsIndexByRoutePath[routeIndex].size() - 1));
          int tripI        = tripSegment->tripI;
          int newArrivalTime;
          int nextStopSeqArrivalTimeIndex;
          int arrivalTimeIndex;
          int transfersFirstIndex;
          Trip transferTrip;
          Transfer transfer;

          for(auto & reachableRoutePath : reachableRoutePaths) // (line 18)
          {
            if(reachableRoutePath->rpI == routeIndex && firstStopSeq < reachableRoutePath->stopSeq)
            {
              newArrivalTime = arrivalTimes[params.weekdayIndex][arrivalTimesIndex[params.weekdayIndex][tripI][0] + reachableRoutePath->stopSeq] + reachableRoutePath->tt;
              if(newArrivalTime < tauMin[n])
              {
                tauMin[n]                     = newArrivalTime; // (line 19)
                shortestJourneyTripSegment[n] = tripSegment; // (line 20)
              }
            }
          }
          
          //std::cerr << "e" << std::endl;
          
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
                      if (n + 1 <= nMax)
                      {
                        tripSegments[n + 1].push_back(new TripSegment(transferTrip.rpI, transfer.tgtTripI, transfer.tgtStopSeq, std::min((int)(stopsIndexByRoutePath[transferTrip.rpI].size()-1), firstReachedStopSeqByTrip[transfer.tgtTripI]), tripSegment->prevTripSegments, tripSegment)); // (enqueue line 3)
                      }
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
      
      //std::cerr << "f" << std::endl;
      
      // add target stop results to json:
      int minArrivalTimeSecondForStop = infinite;
      int minArrivalTimeSecondForStopWithTransferPenalty = infinite;
      int transferPenaltySeconds = 0;
      int numberOfTransfers = 0;
      for(int n = 0; n <= nMax; n++)
      {
        if (n > 0 && params.transferPenaltyMinutes != -1)
        {
          transferPenaltySeconds = params.transferPenaltyMinutes * 60 * n;
        }
        if (tauMin[n] + transferPenaltySeconds < minArrivalTimeSecondForStopWithTransferPenalty)
        {
          minArrivalTimeSecondForStopWithTransferPenalty = tauMin[n] + transferPenaltySeconds;
          minArrivalTimeSecondForStop = tauMin[n];
          numberOfTransfers = n;
        }
      }
      
      //std::cerr << "g" << std::endl;
      
      if (params.detailedResults && (minArrivalTimeSecondForStop < infinite || (tgtStopIs.size() == 1 && !params.returnAllStopsResult)))
      {
        
        //std::cerr << "g1" << std::endl;
        
        json jsonJourneySteps;
        jsonJourneySteps["steps"]            = json::array();
        jsonJourneySteps["legs"]             = json::array();
        jsonJourneySteps["routeIds"]         = json::array();
        jsonJourneySteps["tripIds"]          = json::array();
        jsonJourneySteps["transferRouteIds"] = json::array();
        
        //std::cerr << "g2" << std::endl;
        
        RoutePath    routePath;
        Stop         boardStop;
        Stop         unboardStop;
        TripSegment  lastStep;
        long long    lastRouteId{-1};
        
        //std::cerr << "g3" << std::endl;
        
        if(shortestJourneyTripSegment[numberOfTransfers]->prevTripSegments.size() > 0)
        {
          
          //std::cerr << "g4" << std::endl;
          
          // first steps:
          for (const auto & step : shortestJourneyTripSegment[numberOfTransfers]->prevTripSegments)
          {
            
            //std::cerr << "g5" << std::endl;
            
            routePath   = routePaths[step->rpI];
            //std::cout << step->firstStopSeq << ":" << step->lastStopSeq << "|" << stopsIndexByRoutePath[step->rpI].size() << std::endl;
            //boardStop   = stops[stopsIndexByRoutePath[step->rpI][step->firstStopSeq]];
            //unboardStop = stops[stopsIndexByRoutePath[step->rpI][step->lastStopSeq]];
            //jsonJourneySteps["steps"].push_back({
            //  {"routeFullname", routePath.rSn + " " + routePath.rLn },
            //  {"routeId", routePath.rId},
            //  {"tripId", trips[params.weekdayIndex][step->tripI].id},
            //  {"fromStopId", boardStop.id},
            //  {"fromStop", "[" + boardStop.code + "] " + boardStop.name },
            //  {"toStopId", unboardStop.id},
            //  {"toStop", "[" + unboardStop.code + "] " + unboardStop.name }
            //}); 
            
            //std::cerr << "g6" << std::endl;
            
            jsonJourneySteps["routeIds"].push_back(routePath.rId);
            if (lastRouteId != -1)
            {
              json transferJson = json::array();
              transferJson.push_back(lastRouteId);
              transferJson.push_back(routePath.rId);
              jsonJourneySteps["transferRouteIds"].push_back(transferJson);
            }
            
            //std::cerr << "g7" << std::endl;
            
            lastRouteId = routePath.rId;
            for (int stopSeq = step->firstStopSeq; stopSeq < step->lastStopSeq; stopSeq++)
            {
              json stopsPair = json::array();
              stopsPair.push_back(stops[stopsIndexByRoutePath[step->rpI][stopSeq]].id);
              stopsPair.push_back(stops[stopsIndexByRoutePath[step->rpI][stopSeq + 1]].id);
              jsonJourneySteps["legs"].push_back({{"routeId", routePath.rId}, {"stopsPair", stopsPair}});
            }
          }
        }
        
        //std::cerr << "g8" << std::endl;
        
        // last step:
        lastStep   = *(shortestJourneyTripSegment[numberOfTransfers]);
        routePath   = routePaths[lastStep.rpI];
        
        //std::cerr << "g9" << std::endl;
        
        //boardStop   = stops[stopsIndexByRoutePath[lastStep.rpI][lastStep.firstStopSeq]];
        //unboardStop = stops[stopsIndexByRoutePath[lastStep.rpI][lastStep.lastStopSeq]];
        //jsonJourneySteps["steps"].push_back({
        //  {"routeFullname", routePath.rSn + " " + routePath.rLn },
        //  {"routeId", routePath.rId},
        //  {"tripId", trips[params.weekdayIndex][lastStep.tripI].id},
        //  {"fromStopId", boardStop.id},
        //  {"fromStop", "[" + boardStop.code + "] " + boardStop.name },
        //  {"toStopId", unboardStop.id},
        //  {"toStop", "[" + unboardStop.code + "] " + unboardStop.name }
        //});
        jsonJourneySteps["routeIds"].push_back(routePath.rId);
        
        //std::cerr << "g10" << std::endl;
        
        if (lastRouteId != -1)
        {
          json transferJson = json::array();
          transferJson.push_back(lastRouteId);
          transferJson.push_back(routePath.rId);
          jsonJourneySteps["transferRouteIds"].push_back(transferJson);
        }
        
        //std::cerr << "g11" << std::endl;
        
        //std::cerr << "firstStopSeq=" << lastStep.firstStopSeq << " lastStopSeq=" << lastStep.lastStopSeq << " rpCount=" << stopsIndexByRoutePath[lastStep.rpI].size() << std::endl;
        
        for (int stopSeq = lastStep.firstStopSeq; stopSeq < lastStep.lastStopSeq; stopSeq++)
        {
          json stopsPair = json::array();
          stopsPair.push_back(stops[stopsIndexByRoutePath[lastStep.rpI][stopSeq]].id);
          stopsPair.push_back(stops[stopsIndexByRoutePath[lastStep.rpI][stopSeq + 1]].id);
          jsonJourneySteps["legs"].push_back({{"routeId", routePath.rId}, {"stopsPair", stopsPair}});
        }
        
        
        //std::cerr << "g12" << std::endl;
        
        jsonResult["stops"].push_back({
          {"id", stops[tgtStopI].id},
          {"arrivalTimeSeconds", minArrivalTimeSecondForStop},
          {"numberOfTransfers", numberOfTransfers},
          {"arrivalTimeSecondsWithTransferPenalty", minArrivalTimeSecondForStopWithTransferPenalty},
          {"totalTravelTimeSecondsWithTransferPenalty", (minArrivalTimeSecondForStopWithTransferPenalty == infinite ? infinite : minArrivalTimeSecondForStopWithTransferPenalty - startTime) },
          {"totalTravelTimeSeconds", (minArrivalTimeSecondForStop == infinite ? infinite : minArrivalTimeSecondForStop - startTime) },
          {"routeIds", jsonJourneySteps["routeIds"]},
          {"transferRouteIds", jsonJourneySteps["transferRouteIds"]},
          {"legs", jsonJourneySteps["legs"]}
          //{"steps", jsonJourneySteps["steps"]}
        });
        
        //std::cerr << "g13" << std::endl;
         
      }
      else if(minArrivalTimeSecondForStop < infinite || (tgtStopIs.size() == 1 && !params.returnAllStopsResult)) // no detailed results
      {
        //std::cerr << "g14" << std::endl;
        
        jsonResult["stops"].push_back({
          {"id", stops[tgtStopI].id},
          {"arrivalTimeSeconds", minArrivalTimeSecondForStop},
          {"numberOfTransfers", numberOfTransfers},
          {"arrivalTimeSecondsWithTransferPenalty", minArrivalTimeSecondForStopWithTransferPenalty},
          {"totalTravelTimeSecondsWithTransferPenalty", (minArrivalTimeSecondForStopWithTransferPenalty == infinite ? infinite : minArrivalTimeSecondForStopWithTransferPenalty - startTime) },
          {"totalTravelTimeSeconds", (minArrivalTimeSecondForStop == infinite ? infinite : minArrivalTimeSecondForStop - startTime) }
        });
        
        //std::cerr << "g15" << std::endl;
        
      }
      
      //std::cerr << "h" << std::endl;
      
      for(int n = 0; n <= nMax; n++)
      {
        tauMin[n]                     = infinite;
        //delete shortestJourneyTripSegment[n];
        shortestJourneyTripSegment[n] = new TripSegment();
        if (n > 0)
        {
          for(auto & tripSegment : tripSegments[n])
          {
            delete tripSegment;
          }
          tripSegments[n].clear();
          //std::vector<TripSegment>().swap(tripSegments[n]);
        }
      }
      for(auto & reachableRoutePath : reachableRoutePaths)
      {
        delete reachableRoutePath;
      }
      reachableRoutePaths.clear();
      tauMin.clear();
      shortestJourneyTripSegment.clear();
      firstReachedStopSeqByTrip.clear();
    }

    //for(auto & reachableRoutePath : reachableRoutePaths)
    //{
    //  delete reachableRoutePath;
    //}
    
    //std::cerr << "i" << std::endl;
    
    for(auto & tripSegment : tripSegments[0])
    {
      delete tripSegment;
    }
    tripSegments[0].clear();

    //reachableRoutePaths.clear();
    //tauMin.clear();
    //shortestJourneyTripSegment.clear();
    tripSegments.clear();

    firstReachedStopSeqByTrip.clear();
    allStopsfirstReachedStopSeqByTrip.clear();

    //std::vector<int>().swap(tauMin);
    //std::vector<std::vector<TripSegment*>>().swap(tripSegments);
    //std::vector<TripSegment*>().swap(shortestJourneyTripSegment);
    
    //std::cerr << "j" << std::endl;
    
    algorithmCalculationTime.stop();
    jsonResult["calculatedInMilliseconds"] = algorithmCalculationTime.getDurationMilliseconds();

    return jsonResult;
  }




}
