#include "calculator.hpp"

namespace TrRouting
{
  
  void Calculator::reset(bool resetAccessPaths, bool resetFilters)
  {
    
    int benchmarkingStart = algorithmCalculationTime.getEpoch();

    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
    
    std::fill(nodesTentativeTime.begin(),        nodesTentativeTime.end(),   MAX_INT);
    std::fill(nodesReverseTentativeTime.begin(), nodesReverseTentativeTime.end(), -1);
    //std::fill(nodesD.begin(), nodesD.end(), MAX_INT);
    //std::fill(nodesReverseTentativeTime.begin(), nodesReverseTentativeTime.end(), std::deque<std::pair<int,int>>(1, std::make_pair(MAX_INT, MAX_INT));
    std::fill(nodesAccessTravelTime.begin(),     nodesAccessTravelTime.end(),     -1);
    std::fill(nodesEgressTravelTime.begin(),     nodesEgressTravelTime.end(),     -1);
    if (resetAccessPaths)
    {
      accessFootpaths.clear();
      egressFootpaths.clear();
    }
    std::fill(tripsEnterConnection.begin(),      tripsEnterConnection.end(),      -1);
    std::fill(tripsExitConnection.begin(),       tripsExitConnection.end(),       -1);
    std::fill(tripsEnterConnectionTransferTravelTime.begin(), tripsEnterConnectionTransferTravelTime.end(), MAX_INT);
    std::fill(tripsExitConnectionTransferTravelTime.begin(),  tripsExitConnectionTransferTravelTime.end(),  MAX_INT);
    //std::fill(tripsReverseTime.begin(), tripsReverseTime.end(), MAX_INT);
    std::fill(tripsEnabled.begin(),          tripsEnabled.end(), 1);
    std::fill(tripsUsable.begin(),           tripsUsable.end(),  -1);
    std::fill(forwardJourneys.begin(),       forwardJourneys.end(),       std::make_tuple(-1,-1,-1,-1,-1,-1));
    std::fill(forwardEgressJourneys.begin(), forwardEgressJourneys.end(), std::make_tuple(-1,-1,-1,-1,-1,-1));
    std::fill(reverseJourneys.begin(),       reverseJourneys.end(),       std::make_tuple(-1,-1,-1,-1,-1,-1));
    std::fill(reverseAccessJourneys.begin(), reverseAccessJourneys.end(), std::make_tuple(-1,-1,-1,-1,-1,-1));
    
    benchmarking["reset_1"] += algorithmCalculationTime.getEpoch() - benchmarkingStart;
    benchmarkingStart = algorithmCalculationTime.getEpoch();

    departureTimeSeconds = -1;
    arrivalTimeSeconds   = -1;
    
    if(params.odTrip != NULL && params.forwardCalculation == true)
    {
      departureTimeSeconds = params.odTrip->departureTimeSeconds;
    }
    else if (params.departureTimeSeconds >= 0)
    {
      departureTimeSeconds = params.departureTimeSeconds;
    }
    else if (params.departureTimeHour >= 0 && params.departureTimeMinutes >= 0)
    {
      departureTimeSeconds = params.departureTimeHour * 3600 + params.departureTimeMinutes * 60;
    }
    if (params.arrivalTimeSeconds >= 0)
    {
      arrivalTimeSeconds = params.arrivalTimeSeconds;
    }
    else if(params.arrivalTimeHour >= 0 && params.arrivalTimeMinutes >= 0)
    {
      arrivalTimeSeconds = params.arrivalTimeHour * 3600 + params.arrivalTimeMinutes * 60;
    }


    benchmarking["reset_2"] += algorithmCalculationTime.getEpoch() - benchmarkingStart;
    benchmarkingStart = algorithmCalculationTime.getEpoch();


    if (params.debugDisplay)
      std::cerr << "-- reset and preparations -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();


    int i {0};


    // fetch nodes footpaths accessible from origin using params or osrm fetcher if not provided:
    minAccessTravelTime = MAX_INT;
    maxEgressTravelTime = -1;
    minEgressTravelTime = MAX_INT;
    maxAccessTravelTime = -1;
    
    if (!params.returnAllNodesResult || departureTimeSeconds >= -1)
    {
      if (resetAccessPaths)
      {
        if(params.odTrip != NULL && params.odTrip->originNodesIdx.size() > 0)
        {
          i = 0;
          for (auto & accessNodeIdx : params.odTrip->originNodesIdx)
          {
            accessFootpaths.push_back(std::make_pair(accessNodeIdx, params.odTrip->originNodesTravelTimesSeconds[i]));
            i++;
          }
        }
        else if (params.accessNodesIdx.size() > 0 && params.accessNodeTravelTimesSeconds.size() == params.accessNodesIdx.size())
        {
          i = 0;
          for (auto & accessNodeIdx : params.accessNodesIdx)
          {
            accessFootpaths.push_back(std::make_pair(accessNodeIdx, params.accessNodeTravelTimesSeconds[i]));
            i++;
          }
        }
        else
        {
          accessFootpaths = OsrmFetcher::getAccessibleNodesFootpathsFromPoint(params.origin, nodes, params.accessMode, params);
        }
      }


      int footpathTravelTimeSeconds;
      for (auto & accessFootpath : accessFootpaths)
      {
        footpathTravelTimeSeconds = (int)ceil((float)accessFootpath.second / params.walkingSpeedFactor);
        nodesAccessTravelTime[accessFootpath.first] = footpathTravelTimeSeconds;
        forwardJourneys[accessFootpath.first]       = std::make_tuple(-1, -1, -1, -1, footpathTravelTimeSeconds, -1);
        nodesTentativeTime[accessFootpath.first]    = departureTimeSeconds + footpathTravelTimeSeconds + params.minWaitingTimeSeconds;
        if (footpathTravelTimeSeconds < minAccessTravelTime)
        {
          minAccessTravelTime = footpathTravelTimeSeconds;
        }
        if (footpathTravelTimeSeconds > maxAccessTravelTime)
        {
          maxAccessTravelTime = footpathTravelTimeSeconds;
        }
        //std::cerr << "origin_node: " << nodes[accessFootpath.first].name << " - " << Toolbox::convertSecondsToFormattedTime(nodesTentativeTime[accessFootpath.first]) << std::endl;
        //std::cerr << std::to_string(nodes[accessFootpath.first].id) + ",";
      }
    }
  
    benchmarking["reset_3"] += algorithmCalculationTime.getEpoch() - benchmarkingStart;
    benchmarkingStart = algorithmCalculationTime.getEpoch();
  
  
    if (!params.returnAllNodesResult || arrivalTimeSeconds >= -1)
    {
      if (resetAccessPaths)
      {
        // fetch nodes footpaths accessible to destination using params or osrm fetcher if not provided:
        if(params.odTrip != NULL && params.odTrip->destinationNodesIdx.size() > 0)
        {
          i = 0;
          for (auto & egressNodeIdx : params.odTrip->destinationNodesIdx)
          {
            egressFootpaths.push_back(std::make_pair(egressNodeIdx, params.odTrip->destinationNodesTravelTimesSeconds[i]));
            i++;
          }
        }
        else if (params.egressNodesIdx.size() > 0 && params.egressNodeTravelTimesSeconds.size() == params.egressNodesIdx.size())
        {
          egressFootpaths.reserve(params.egressNodesIdx.size());
          i = 0;
          for (auto & egressNodeIdx : params.egressNodesIdx)
          {
            egressFootpaths.push_back(std::make_pair(egressNodeIdx, params.egressNodeTravelTimesSeconds[i]));
            i++;
          }
        }
        else
        {
          egressFootpaths = OsrmFetcher::getAccessibleNodesFootpathsFromPoint(params.destination, nodes, params.accessMode, params);
        }
      }
      
      int footpathTravelTimeSeconds;
      for (auto & egressFootpath : egressFootpaths)
      {
        footpathTravelTimeSeconds                       = (int)ceil((float)egressFootpath.second / params.walkingSpeedFactor);
        nodesEgressTravelTime[egressFootpath.first]     = footpathTravelTimeSeconds;
        reverseJourneys[egressFootpath.first]           = std::make_tuple(-1, -1, -1, -1, footpathTravelTimeSeconds, -1);
        nodesReverseTentativeTime[egressFootpath.first] = arrivalTimeSeconds - footpathTravelTimeSeconds;
        if (footpathTravelTimeSeconds > maxEgressTravelTime)
        {
          maxEgressTravelTime = footpathTravelTimeSeconds;
        }
        if (footpathTravelTimeSeconds < minEgressTravelTime)
        {
          minEgressTravelTime = footpathTravelTimeSeconds;
        }
        //nodesD[egressFootpath.first]                = egressFootpath.second;
        //result.json += "origin_node: " + nodes[accessFootpath.first].name + " - " + Toolbox::convertSecondsToFormattedTime(nodesTentativeTime[accessFootpath.first]) + "\n";
        //result.json += std::to_string((int)(ceil(egressFootpath.second))) + ",";
      }
    }
    
    //std::cerr << "-- maxEgressTravelTime = " << maxEgressTravelTime << std::endl;

    benchmarking["reset_4"] += algorithmCalculationTime.getEpoch() - benchmarkingStart;
    benchmarkingStart = algorithmCalculationTime.getEpoch();


    if (params.debugDisplay)
      std::cerr << "-- access and egress footpaths -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();






    // disable trips according to parameters:
    i = 0;

    if (resetFilters)
    {
      for (auto & trip : trips)
      {
        if (tripsEnabled[i] == 1 && params.onlyServicesIdx.size() > 0)
        {
          if (std::find(params.onlyServicesIdx.begin(), params.onlyServicesIdx.end(), trip.serviceIdx) == params.onlyServicesIdx.end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && params.onlyLinesIdx.size() > 0)
        {
          if (std::find(params.onlyLinesIdx.begin(), params.onlyLinesIdx.end(), trip.lineIdx) == params.onlyLinesIdx.end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && params.onlyModesIdx.size() > 0)
        {
          if (std::find(params.onlyModesIdx.begin(), params.onlyModesIdx.end(), trip.modeIdx) == params.onlyModesIdx.end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && params.onlyAgenciesIdx.size() > 0)
        {
          if (std::find(params.onlyAgenciesIdx.begin(), params.onlyAgenciesIdx.end(), trip.agencyIdx) == params.onlyAgenciesIdx.end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && params.exceptServicesIdx.size() > 0)
        {
          if (std::find(params.exceptServicesIdx.begin(), params.exceptServicesIdx.end(), trip.serviceIdx) != params.exceptServicesIdx.end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && params.exceptLinesIdx.size() > 0)
        {
          if (std::find(params.exceptLinesIdx.begin(), params.exceptLinesIdx.end(), trip.lineIdx) != params.exceptLinesIdx.end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && params.exceptModesIdx.size() > 0)
        {
          if (std::find(params.exceptModesIdx.begin(), params.exceptModesIdx.end(), trip.modeIdx) != params.exceptModesIdx.end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && params.exceptAgenciesIdx.size() > 0)
        {
          if (std::find(params.exceptAgenciesIdx.begin(), params.exceptAgenciesIdx.end(), trip.agencyIdx) != params.exceptAgenciesIdx.end())
          {
            tripsEnabled[i] = -1;
          }
        }

        i++;
      }
    }

    
    
    benchmarking["reset_5"] += algorithmCalculationTime.getEpoch() - benchmarkingStart;
    benchmarkingStart = algorithmCalculationTime.getEpoch();




    if (params.debugDisplay)
      std::cerr << "-- filter trips -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

    //benchmarking["reset"] += algorithmCalculationTime.getEpoch() - benchmarkingStart;



  }



}
