#include "calculator.hpp"
#include "parameters.hpp"
#include "trip.hpp"
#include "osrm_fetcher.hpp"
#include "toolbox.hpp" //MAX_INT
#include "od_trip.hpp"
#include "routing_result.hpp"

namespace TrRouting
{

  void Calculator::reset(RouteParameters &parameters, bool resetAccessPaths, bool resetFilters)
  {
    
    using JourneyTuple = std::tuple<int,int,int,int,int,short,int>;

    int benchmarkingStart = algorithmCalculationTime.getEpoch();

    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
    
    std::fill(nodesTentativeTime.begin()       , nodesTentativeTime.end()       , MAX_INT);
    std::fill(nodesReverseTentativeTime.begin(), nodesReverseTentativeTime.end(), -1);
    std::fill(nodesAccessTravelTime.begin()    , nodesAccessTravelTime.end()    , -1);
    std::fill(nodesAccessDistance.begin()      , nodesAccessDistance.end()      , -1);
    std::fill(nodesEgressTravelTime.begin()    , nodesEgressTravelTime.end()    , -1);
    std::fill(nodesEgressDistance.begin()      , nodesEgressDistance.end()      , -1);
    if (resetAccessPaths)
    {
      accessFootpaths.clear();
      accessFootpaths.shrink_to_fit();
      egressFootpaths.clear();
      egressFootpaths.shrink_to_fit();
    }
    std::fill(tripsEnterConnection.begin()                  , tripsEnterConnection.end()                  , -1);
    std::fill(tripsExitConnection.begin()                   , tripsExitConnection.end()                   , -1);
    std::fill(tripsEnterConnectionTransferTravelTime.begin(), tripsEnterConnectionTransferTravelTime.end(), MAX_INT);
    std::fill(tripsExitConnectionTransferTravelTime.begin() , tripsExitConnectionTransferTravelTime.end() , MAX_INT);
    if (resetFilters)
    {
      std::fill(tripsEnabled.begin(), tripsEnabled.end(), 1);
    }
    std::fill(tripsUsable.begin()               , tripsUsable.end()               , -1);
    std::fill(forwardJourneysSteps.begin()      , forwardJourneysSteps.end()      , JourneyTuple(-1,-1,-1,-1,-1,-1,-1));
    std::fill(forwardEgressJourneysSteps.begin(), forwardEgressJourneysSteps.end(), JourneyTuple(-1,-1,-1,-1,-1,-1,-1));
    std::fill(reverseJourneysSteps.begin()      , reverseJourneysSteps.end()      , JourneyTuple(-1,-1,-1,-1,-1,-1,-1));
    std::fill(reverseAccessJourneysSteps.begin(), reverseAccessJourneysSteps.end(), JourneyTuple(-1,-1,-1,-1,-1,-1,-1));
    
    departureTimeSeconds = -1;
    arrivalTimeSeconds   = -1;
    
    if(odTrip != nullptr && params.forwardCalculation == true)
    {
      departureTimeSeconds = odTrip->departureTimeSeconds;
    }
    else if (parameters.isForwardCalculation())
    {
      departureTimeSeconds = parameters.getTimeOfTrip();
    }
    if (!parameters.isForwardCalculation())
    {
      arrivalTimeSeconds = parameters.getTimeOfTrip();
    }

    if (params.debugDisplay)
      std::cerr << "-- reset and preparations -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";

    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

    // fetch nodes footpaths accessible from origin using params or osrm fetcher if not provided:
    minAccessTravelTime = MAX_INT;
    maxEgressTravelTime = -1;
    minEgressTravelTime = MAX_INT;
    maxAccessTravelTime = -1;

    int j {0};

    if (!params.returnAllNodesResult || departureTimeSeconds > -1)
    {
      if (resetAccessPaths)
      {

        if (params.debugDisplay)
          std::cerr << "  resetting access paths " << std::endl;

        if(odTrip != nullptr)
        {
          if (params.debugDisplay)
            std::cerr << "  using odTrip with " << odTrip->originNodesIdx.size() << " accessible nodes" << std::endl;

          accessFootpaths.clear();
          j = 0;
          for (auto & accessNodeIdx : odTrip->originNodesIdx)
          {
            accessFootpaths.push_back(std::make_tuple(accessNodeIdx, odTrip->originNodesTravelTimesSeconds[j], odTrip->originNodesDistancesMeters[j]));
            j++;
          }
        }
        else if (params.accessNodesIdx.size() > 0 && params.accessNodeTravelTimesSeconds.size() == params.accessNodesIdx.size())
        {
          accessFootpaths.clear();
          j = 0;
          for (auto & accessNodeIdx : params.accessNodesIdx)
          {
            int distanceMeters{-1};
            if (params.accessNodeDistancesMeters.size() == params.accessNodesIdx.size())
            {
              distanceMeters = params.accessNodeDistancesMeters[j];
            }
            accessFootpaths.push_back(std::make_tuple(accessNodeIdx, params.accessNodeTravelTimesSeconds[j], distanceMeters));
            j++;
          }
        }
        else
        {
          if (params.debugDisplay)
            std::cout << "  fetching nodes with osrm with mode " << params.accessMode << std::endl;

          accessFootpaths = std::move(OsrmFetcher::getAccessibleNodesFootpathsFromPoint(*parameters.getOrigin(), nodes, params.accessMode, params, parameters.getMaxAccessWalkingTravelTimeSeconds(), params.walkingSpeedMetersPerSecond));
          if (accessFootpaths.size() == 0) {
            throw NoRoutingFoundException(NoRoutingReason::NO_ACCESS_AT_ORIGIN); 
          }
        }
      }

      if (params.debugDisplay)
        std::cout << "  parsing access footpaths to find min/max access travel times" << std::endl;

      int footpathTravelTimeSeconds;
      int footpathDistanceMeters;
      for (auto & accessFootpath : accessFootpaths)
      {
        footpathTravelTimeSeconds = (int)ceil((float)(std::get<1>(accessFootpath)) / params.walkingSpeedFactor);
        footpathDistanceMeters    = std::get<2>(accessFootpath);

        nodesAccessTravelTime[std::get<0>(accessFootpath)] = footpathTravelTimeSeconds;
        nodesAccessDistance[std::get<0>(accessFootpath)]   = footpathDistanceMeters;
        forwardJourneysSteps[std::get<0>(accessFootpath)]  = std::make_tuple(-1, -1, -1, -1, footpathTravelTimeSeconds, -1, footpathDistanceMeters);
        nodesTentativeTime[std::get<0>(accessFootpath)]    = departureTimeSeconds + footpathTravelTimeSeconds;
        if (footpathTravelTimeSeconds < minAccessTravelTime)
        {
          minAccessTravelTime = footpathTravelTimeSeconds;
        }
        if (footpathTravelTimeSeconds > maxAccessTravelTime)
        {
          maxAccessTravelTime = footpathTravelTimeSeconds;
        }
        //std::cerr << "origin_node: " << nodes[std::get<0>(accessFootpath)].get()->name << " - " << Toolbox::convertSecondsToFormattedTime(nodesTentativeTime[std::get<0>(accessFootpath)]) << std::endl;
        //std::cerr << std::to_string(nodes[std::get<0>(accessFootpath)].get()->id) + ",";
      }
    }
  
    if (!params.returnAllNodesResult || arrivalTimeSeconds > -1)
    {
      if (resetAccessPaths)
      {
        // fetch nodes footpaths accessible to destination using params or osrm fetcher if not provided:
        if(odTrip != nullptr)
        {

          if (params.debugDisplay)
            std::cerr << "  using odTrip with " << odTrip->destinationNodesIdx.size() << " egressible nodes" << std::endl;

          egressFootpaths.clear();
          j = 0;
          for (auto & egressNodeIdx : odTrip->destinationNodesIdx)
          {
            egressFootpaths.push_back(std::make_tuple(egressNodeIdx, odTrip->destinationNodesTravelTimesSeconds[j], odTrip->destinationNodesDistancesMeters[j]));
            j++;
          }
        }
        else if (params.egressNodesIdx.size() > 0 && params.egressNodeTravelTimesSeconds.size() == params.egressNodesIdx.size())
        {
          egressFootpaths.clear();
          j = 0;
          for (auto & egressNodeIdx : params.egressNodesIdx)
          {
            int distanceMeters{-1};
            if (params.egressNodeDistancesMeters.size() == params.egressNodesIdx.size())
            {
              distanceMeters = params.egressNodeDistancesMeters[j];
            }
            egressFootpaths.push_back(std::make_tuple(egressNodeIdx, params.egressNodeTravelTimesSeconds[j], distanceMeters));
            j++;
          }
        }
        else
        {
          egressFootpaths = std::move(OsrmFetcher::getAccessibleNodesFootpathsFromPoint(*parameters.getDestination(), nodes, params.accessMode, params, parameters.getMaxEgressWalkingTravelTimeSeconds(), params.walkingSpeedMetersPerSecond));
          if (egressFootpaths.size() == 0) {
            throw NoRoutingFoundException(NoRoutingReason::NO_ACCESS_AT_DESTINATION); 
          }
        }
      }
      
      if (params.debugDisplay)
        std::cout << "  parsing egress footpaths to find min/max egress travel times" << std::endl;

      int footpathTravelTimeSeconds;
      int footpathDistanceMeters;
      for (auto & egressFootpath : egressFootpaths)
      {
        footpathTravelTimeSeconds  = (int)ceil((float)(std::get<1>(egressFootpath)) / params.walkingSpeedFactor);
        footpathDistanceMeters     = std::get<2>(egressFootpath);
        nodesEgressTravelTime[std::get<0>(egressFootpath)]     = footpathTravelTimeSeconds;
        nodesEgressDistance[std::get<0>(egressFootpath)]       = footpathDistanceMeters;
        reverseJourneysSteps[std::get<0>(egressFootpath)]      = std::make_tuple(-1, -1, -1, -1, footpathTravelTimeSeconds, -1, footpathDistanceMeters);
        nodesReverseTentativeTime[std::get<0>(egressFootpath)] = arrivalTimeSeconds - footpathTravelTimeSeconds;
        if (footpathTravelTimeSeconds > maxEgressTravelTime)
        {
          maxEgressTravelTime = footpathTravelTimeSeconds;
        }
        if (footpathTravelTimeSeconds < minEgressTravelTime)
        {
          minEgressTravelTime = footpathTravelTimeSeconds;
        }
        //nodesD[std::get<0>(egressFootpath)]                = std::get<1>(egressFootpath);
        //result.json += "destination_node: " + nodes[std::get<0>(egressFootpath)].get()->name + " - " + Toolbox::convertSecondsToFormattedTime(nodesTentativeTime[std::get<0>(accessFootpath)]) + "\n";
        //result.json += std::to_string((int)(ceil(std::get<1>(egressFootpath)))) + ",";
      }
    }
    
    //std::cerr << "-- maxEgressTravelTime = " << maxEgressTravelTime << std::endl;

    if (params.debugDisplay)
      std::cerr << "-- access and egress footpaths -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
    
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();


    // disable trips according to parameters:

    if (resetFilters)
    {

      if (params.debugDisplay)
        std::cout << "  resetting filters" << std::endl;

      if (params.calculateAllOdTrips)
      {
        // reset connections demand:
        for (auto & tripConnectionDemand : tripConnectionDemands)
        {
          std::generate(tripConnectionDemand.begin(), tripConnectionDemand.end(), []() { return std::make_unique<float>(0.0); });
        }
      }

      int i {0};
      for (auto & trip : trips)
      {
        if (tripsEnabled[i] == 1 && parameters.getOnlyServicesIdx()->size() > 0)
        {
          if (std::find(parameters.getOnlyServicesIdx()->begin(), parameters.getOnlyServicesIdx()->end(), trip->serviceIdx) == parameters.getOnlyServicesIdx()->end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && parameters.getOnlyLinesIdx()->size() > 0)
        {
          if (std::find(parameters.getOnlyLinesIdx()->begin(), parameters.getOnlyLinesIdx()->end(), trip->lineIdx) == parameters.getOnlyLinesIdx()->end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && parameters.getOnlyModesIdx()->size() > 0)
        {
          if (std::find(parameters.getOnlyModesIdx()->begin(), parameters.getOnlyModesIdx()->end(), trip->modeIdx) == parameters.getOnlyModesIdx()->end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && parameters.getOnlyNodesIdx()->size() > 0)
        {
          // FIXME: This is not right, it should look for a node, not the mode
          if (std::find(parameters.getOnlyNodesIdx()->begin(), parameters.getOnlyNodesIdx()->end(), trip->modeIdx) == parameters.getOnlyNodesIdx()->end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && parameters.getOnlyAgenciesIdx()->size() > 0)
        {
          if (std::find(parameters.getOnlyAgenciesIdx()->begin(), parameters.getOnlyAgenciesIdx()->end(), trip->agencyIdx) == parameters.getOnlyAgenciesIdx()->end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && parameters.getExceptServicesIdx()->size() > 0)
        {
          if (std::find(parameters.getExceptServicesIdx()->begin(), parameters.getExceptServicesIdx()->end(), trip->serviceIdx) != parameters.getExceptServicesIdx()->end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && parameters.getExceptLinesIdx()->size() > 0)
        {
          if (std::find(parameters.getExceptLinesIdx()->begin(), parameters.getExceptLinesIdx()->end(), trip->lineIdx) != parameters.getExceptLinesIdx()->end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && parameters.getExceptNodesIdx()->size() > 0)
        {
          // FIXME: This is not right, it should look for a node, not the mode
          if (std::find(parameters.getExceptNodesIdx()->begin(), parameters.getExceptNodesIdx()->end(), trip->modeIdx) != parameters.getExceptNodesIdx()->end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && parameters.getExceptModesIdx()->size() > 0)
        {
          if (std::find(parameters.getExceptModesIdx()->begin(), parameters.getExceptModesIdx()->end(), trip->modeIdx) != parameters.getExceptModesIdx()->end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && parameters.getExceptAgenciesIdx()->size() > 0)
        {
          if (std::find(parameters.getExceptAgenciesIdx()->begin(), parameters.getExceptAgenciesIdx()->end(), trip->agencyIdx) != parameters.getExceptAgenciesIdx()->end())
          {
            tripsEnabled[i] = -1;
          }
        }
        i++;
      }
    }

    
    if (params.debugDisplay)
      benchmarking["reset"] += algorithmCalculationTime.getEpoch() - benchmarkingStart;


    if (params.debugDisplay)
      std::cerr << "-- filter trips -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";

    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

  }

}
