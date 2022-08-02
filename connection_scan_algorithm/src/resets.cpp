#include "spdlog/spdlog.h"
#include "calculator.hpp"
#include "parameters.hpp"
#include "trip.hpp"
#include "osrm_fetcher.hpp"
#include "toolbox.hpp" //MAX_INT
#include "od_trip.hpp"
#include "routing_result.hpp"
#include "mode.hpp"
#include "agency.hpp"
#include "service.hpp"
#include "line.hpp"

namespace TrRouting
{

  void Calculator::reset(RouteParameters &parameters, bool resetAccessPaths, bool resetFilters)
  {
    
    using JourneyTuple = std::tuple<int,int,int,int,int,short,int>;

    int benchmarkingStart = algorithmCalculationTime.getEpoch();
    bool accessFootpathOk = true;
    bool egressFootpathOk = true;

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

    spdlog::debug("-- reset and preparations -- {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);

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

        spdlog::debug("  resetting access paths ");

        if(odTrip != nullptr)
        {
          spdlog::debug("  using odTrip with {} accessible nodes", odTrip->originNodesIdx.size());

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
          spdlog::debug("  fetching nodes with osrm with mode {}", params.accessMode);

          accessFootpaths = std::move(OsrmFetcher::getAccessibleNodesFootpathsFromPoint(*parameters.getOrigin(), nodes, params.accessMode, parameters.getMaxAccessWalkingTravelTimeSeconds(), params.walkingSpeedMetersPerSecond));
          if (accessFootpaths.size() == 0) {
            accessFootpathOk = false;
          }
        }
      }

      spdlog::debug("  parsing access footpaths to find min/max access travel times");

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
      }
    }
  
    if (!params.returnAllNodesResult || arrivalTimeSeconds > -1)
    {
      if (resetAccessPaths)
      {
        // fetch nodes footpaths accessible to destination using params or osrm fetcher if not provided:
        if(odTrip != nullptr)
        {

          spdlog::debug("  using odTrip with {} egressible nodes", odTrip->destinationNodesIdx.size());

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
          egressFootpaths = std::move(OsrmFetcher::getAccessibleNodesFootpathsFromPoint(*parameters.getDestination(), nodes, params.accessMode, parameters.getMaxEgressWalkingTravelTimeSeconds(), params.walkingSpeedMetersPerSecond));
          if (egressFootpaths.size() == 0) {
            egressFootpathOk = false;
          }
        }
      }
      
      spdlog::debug("  parsing egress footpaths to find min/max egress travel times");

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

    // Throw proper exceptions when no access at origin and/or destination
    if (!egressFootpathOk && !accessFootpathOk) {
      throw NoRoutingFoundException(NoRoutingReason::NO_ACCESS_AT_ORIGIN_AND_DESTINATION);
    } else if (!accessFootpathOk) {
      throw NoRoutingFoundException(NoRoutingReason::NO_ACCESS_AT_ORIGIN);
    } else if (!egressFootpathOk) {
      throw NoRoutingFoundException(NoRoutingReason::NO_ACCESS_AT_DESTINATION);
    }
    

    spdlog::debug("-- access and egress footpaths -- {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);
    
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();


    // disable trips according to parameters:

    if (resetFilters)
    {

      spdlog::debug("  resetting filters");

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
        if (tripsEnabled[i] == 1 && parameters.getOnlyServices().size() > 0)
        {
          if (std::find(parameters.getOnlyServices().begin(), parameters.getOnlyServices().end(), trip->service) == parameters.getOnlyServices().end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && parameters.getOnlyLines().size() > 0)
        {
          if (std::find(parameters.getOnlyLines().begin(), parameters.getOnlyLines().end(), trip->line) == parameters.getOnlyLines().end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && parameters.getOnlyModes().size() > 0)
        {
          if (std::find(parameters.getOnlyModes().begin(), parameters.getOnlyModes().end(), trip->mode) == parameters.getOnlyModes().end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && parameters.getOnlyNodesIdx()->size() > 0)
        {
          // FIXME: This is not right, it should look for a node, not the mode
          // FIXME2: Commented out, since mode is now typed, it won't match
          /*if (std::find(parameters.getOnlyNodesIdx()->begin(), parameters.getOnlyNodesIdx()->end(), trip->modeIdx) == parameters.getOnlyNodesIdx()->end())
          {
            tripsEnabled[i] = -1;
          }(*/
        }

        if (tripsEnabled[i] == 1 && parameters.getOnlyAgencies().size() > 0)
        {
          if (std::find(parameters.getOnlyAgencies().begin(), parameters.getOnlyAgencies().end(), trip->agency) == parameters.getOnlyAgencies().end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && parameters.getExceptServices().size() > 0)
        {
          if (std::find(parameters.getExceptServices().begin(), parameters.getExceptServices().end(), trip->service) != parameters.getExceptServices().end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && parameters.getExceptLines().size() > 0)
        {
          if (std::find(parameters.getExceptLines().begin(), parameters.getExceptLines().end(), trip->line) != parameters.getExceptLines().end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && parameters.getExceptNodesIdx()->size() > 0)
        {
          // FIXME: This is not right, it should look for a node, not the mode
          // FIXME2: Commented out, since mode is now typed, it won't match
          /*
          if (std::find(parameters.getExceptNodesIdx()->begin(), parameters.getExceptNodesIdx()->end(), trip->modeIdx) != parameters.getExceptNodesIdx()->end())
          {
            tripsEnabled[i] = -1;
            }*/
        }

        if (tripsEnabled[i] == 1 && parameters.getExceptModes().size() > 0)
        {
          if (std::find(parameters.getExceptModes().begin(), parameters.getExceptModes().end(), trip->mode) != parameters.getExceptModes().end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && parameters.getExceptAgencies().size() > 0)
        {
          if (std::find(parameters.getExceptAgencies().begin(), parameters.getExceptAgencies().end(), trip->agency) != parameters.getExceptAgencies().end())
          {
            tripsEnabled[i] = -1;
          }
        }
        i++;
      }
    }

    
    benchmarking["reset"] += algorithmCalculationTime.getEpoch() - benchmarkingStart;
    
    
    spdlog::debug("-- filter trips -- {} microseconds ", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);

    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

  }

}
