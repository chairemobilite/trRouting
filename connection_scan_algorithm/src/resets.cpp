#include "calculator.hpp"

namespace TrRouting
{
  
  void Calculator::reset(bool resetAccessPaths)
  {
    
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
    
    std::fill(stopsTentativeTime.begin(),        stopsTentativeTime.end(),   MAX_INT);
    std::fill(stopsReverseTentativeTime.begin(), stopsReverseTentativeTime.end(), -1);
    //std::fill(stopsD.begin(), stopsD.end(), MAX_INT);
    //std::fill(stopsReverseTentativeTime.begin(), stopsReverseTentativeTime.end(), std::deque<std::pair<int,int>>(1, std::make_pair(MAX_INT, MAX_INT));
    std::fill(stopsAccessTravelTime.begin(),     stopsAccessTravelTime.end(),     -1);
    std::fill(stopsEgressTravelTime.begin(),     stopsEgressTravelTime.end(),     -1);
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
    
    departureTimeSeconds = -1;
    arrivalTimeSeconds   = -1;
    
    if(params.odTrip != NULL && params.forwardCalculation == true)
    {
      departureTimeSeconds = params.odTrip->departureTimeSeconds;
    }
    else if (params.departureTimeSeconds != -1)
    {
      departureTimeSeconds = params.departureTimeSeconds;
    }
    else if (params.departureTimeHour != -1 && params.departureTimeMinutes != -1)
    {
      departureTimeSeconds = params.departureTimeHour * 3600 + params.departureTimeMinutes * 60;
    }
    if (params.arrivalTimeSeconds != -1)
    {
      arrivalTimeSeconds = params.arrivalTimeSeconds;
    }
    else if(params.arrivalTimeHour != -1 && params.arrivalTimeMinutes != -1)
    {
      arrivalTimeSeconds = params.arrivalTimeHour * 3600 + params.arrivalTimeMinutes * 60;
    }





    if (params.debugDisplay)
      std::cerr << "-- reset and preparations -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();


    int i {0};


    // fetch stops footpaths accessible from origin using params or osrm fetcher if not provided:
    minAccessTravelTime = MAX_INT;
    maxEgressTravelTime = -1;
    minEgressTravelTime = MAX_INT;
    maxAccessTravelTime = -1;
    
    if (!params.returnAllStopsResult || departureTimeSeconds > -1)
    {
      if (resetAccessPaths)
      {
        if(params.odTrip != NULL && params.odTrip->accessFootpathsStartIndex >= 0 && params.odTrip->accessFootpathsEndIndex >= 0 && params.odTrip->accessFootpathsEndIndex >= params.odTrip->accessFootpathsStartIndex)
        {
          accessFootpaths.assign(odTripFootpaths.begin() + params.odTrip->accessFootpathsStartIndex, odTripFootpaths.begin() + params.odTrip->accessFootpathsEndIndex);
        }
        else if (params.accessStopIds.size() > 0 && params.accessStopTravelTimesSeconds.size() == params.accessStopIds.size())
        {
          i = 0;
          for (auto & accessStopId : params.accessStopIds)
          {
            accessFootpaths.push_back(std::make_pair(stopIndexesById[accessStopId], params.accessStopTravelTimesSeconds[i]));
            i++;
          }
        }
        else
        {
          accessFootpaths = OsrmFetcher::getAccessibleStopsFootpathsFromPoint(params.origin, stops, params.accessMode, params);
        }
      }
    
      for (auto & accessFootpath : accessFootpaths)
      {
        if (accessFootpath.second <= params.maxAccessWalkingTravelTimeSeconds)
        {
          stopsAccessTravelTime[accessFootpath.first] = accessFootpath.second;
          forwardJourneys[accessFootpath.first]       = std::make_tuple(-1, -1, -1, -1, accessFootpath.second, -1);
          stopsTentativeTime[accessFootpath.first]    = departureTimeSeconds + accessFootpath.second + params.minWaitingTimeSeconds;
          if (accessFootpath.second < minAccessTravelTime)
          {
            minAccessTravelTime = accessFootpath.second;
          }
          if (accessFootpath.second > maxAccessTravelTime)
          {
            maxAccessTravelTime = accessFootpath.second;
          }
        }
        
        //std::cerr << "origin_stop: " << stops[accessFootpath.first].name << " - " << Toolbox::convertSecondsToFormattedTime(stopsTentativeTime[accessFootpath.first]) << std::endl;
        //std::cerr << std::to_string(stops[accessFootpath.first].id) + ",";
      }
    }
  
  
  
  
    if (!params.returnAllStopsResult || arrivalTimeSeconds > -1)
    {
      if (resetAccessPaths)
      {
        // fetch stops footpaths accessible to destination using params or osrm fetcher if not provided:
        if(params.odTrip != NULL && params.odTrip->egressFootpathsStartIndex >= 0 && params.odTrip->egressFootpathsEndIndex >= 0 && params.odTrip->egressFootpathsEndIndex >= params.odTrip->egressFootpathsStartIndex)
        {
          egressFootpaths.assign(odTripFootpaths.begin() + params.odTrip->egressFootpathsStartIndex, odTripFootpaths.begin() + params.odTrip->egressFootpathsEndIndex);
        }
        else if (params.egressStopIds.size() > 0 && params.egressStopTravelTimesSeconds.size() == params.egressStopIds.size())
        {
          egressFootpaths.reserve(params.egressStopIds.size());
          i = 0;
          for (auto & egressStopId : params.egressStopIds)
          {
            egressFootpaths.push_back(std::make_pair(stopIndexesById[egressStopId], params.egressStopTravelTimesSeconds[i]));
            i++;
          }
        }
        else
        {
          egressFootpaths = OsrmFetcher::getAccessibleStopsFootpathsFromPoint(params.destination, stops, params.accessMode, params);
        }
      }
      
      for (auto & egressFootpath : egressFootpaths)
      {
        if (egressFootpath.second <= params.maxEgressWalkingTravelTimeSeconds)
        {
          stopsEgressTravelTime[egressFootpath.first]     = egressFootpath.second;
          reverseJourneys[egressFootpath.first]           = std::make_tuple(-1, -1, -1, -1, egressFootpath.second, -1);
          stopsReverseTentativeTime[egressFootpath.first] = arrivalTimeSeconds - egressFootpath.second;
          if (egressFootpath.second > maxEgressTravelTime)
          {
            maxEgressTravelTime = egressFootpath.second;
          }
          if (egressFootpath.second < minEgressTravelTime)
          {
            minEgressTravelTime = egressFootpath.second;
          }
        }
        //stopsD[egressFootpath.first]                = egressFootpath.second;
        //result.json += "origin_stop: " + stops[accessFootpath.first].name + " - " + Toolbox::convertSecondsToFormattedTime(stopsTentativeTime[accessFootpath.first]) + "\n";
        //result.json += std::to_string((int)(ceil(egressFootpath.second))) + ",";
      }
    }
    
    //std::cerr << "-- maxEgressTravelTime = " << maxEgressTravelTime << std::endl;





    if (params.debugDisplay)
      std::cerr << "-- access and egress footpaths -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();






    // disable trips according to parameters:
    i = 0;
    for (auto & trip : trips)
    {
      if (params.onlyServiceIds.size() > 0)
      {
        if (std::find(params.onlyServiceIds.begin(), params.onlyServiceIds.end(), trip.serviceId) == params.onlyServiceIds.end())
        {
          tripsEnabled[i] = -1;
        }
      }
      
      if (params.onlyRouteIds.size() > 0)
      {
        if (std::find(params.onlyRouteIds.begin(), params.onlyRouteIds.end(), trip.routeId) == params.onlyRouteIds.end())
        {
          tripsEnabled[i] = -1;
        }
      }
      
      if (params.onlyRouteTypeIds.size() > 0)
      {
        if (std::find(params.onlyRouteTypeIds.begin(), params.onlyRouteTypeIds.end(), trip.routeTypeId) == params.onlyRouteTypeIds.end())
        {
          tripsEnabled[i] = -1;
        }
      }
      
      if (params.onlyAgencyIds.size() > 0)
      {
        if (std::find(params.onlyAgencyIds.begin(), params.onlyAgencyIds.end(), trip.agencyId) == params.onlyAgencyIds.end())
        {
          tripsEnabled[i] = -1;
        }
      }
      
      
      
      if (params.exceptServiceIds.size() > 0)
      {
        if (std::find(params.exceptServiceIds.begin(), params.exceptServiceIds.end(), trip.serviceId) != params.exceptServiceIds.end())
        {
          tripsEnabled[i] = -1;
        }
      }
      
      if (params.exceptRouteIds.size() > 0)
      {
        if (std::find(params.exceptRouteIds.begin(), params.exceptRouteIds.end(), trip.routeId) != params.exceptRouteIds.end())
        {
          tripsEnabled[i] = -1;
        }
      }
      
      if (params.exceptRouteTypeIds.size() > 0)
      {
        if (std::find(params.exceptRouteTypeIds.begin(), params.exceptRouteTypeIds.end(), trip.routeTypeId) != params.exceptRouteTypeIds.end())
        {
          tripsEnabled[i] = -1;
        }
      }
      
      if (params.exceptAgencyIds.size() > 0)
      {
        if (std::find(params.exceptAgencyIds.begin(), params.exceptAgencyIds.end(), trip.agencyId) != params.exceptAgencyIds.end())
        {
          tripsEnabled[i] = -1;
        }
      }
      
      i++;
    }
    




    if (params.debugDisplay)
      std::cerr << "-- filter trips -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();





  }



}
