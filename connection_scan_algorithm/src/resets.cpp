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
#include "node.hpp"
#include "transit_data.hpp"

namespace TrRouting
{

  void Calculator::reset(RouteParameters &parameters, bool resetAccessPaths, bool resetFilters)
  {
    
    int benchmarkingStart = algorithmCalculationTime.getEpoch();
    //TODO Should we just check the size of accessFootpath and egressFootpath instead of adding a flag?
    bool accessFootpathOk = true;
    bool egressFootpathOk = true;

    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

    if (resetAccessPaths)
    {
      accessFootpaths.clear();
      accessFootpaths.shrink_to_fit();
      egressFootpaths.clear();
      egressFootpaths.shrink_to_fit();
    }
    tripsQueryOverlay.clear();
    forwardJourneysSteps.clear();
    forwardEgressJourneysSteps.clear();
    reverseJourneysSteps.clear();
    reverseAccessJourneysSteps.clear();

    
    departureTimeSeconds = -1;
    arrivalTimeSeconds   = -1;
    
    if(odTripGlob.has_value() && params.forwardCalculation == true)
    {
      departureTimeSeconds = odTripGlob.value().get().departureTimeSeconds;
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

    if (!params.returnAllNodesResult || departureTimeSeconds > -1)
    {
      if (resetAccessPaths)
      {

        spdlog::debug("  resetting access paths ");

        if(odTripGlob.has_value())
        {
          spdlog::debug("  using odTrip with {} accessible nodes", odTripGlob.value().get().originNodes.size());

          accessFootpaths.clear();
          //TODO This can be a std::copy
          for (auto & accessNode : odTripGlob.value().get().originNodes)
          {
            accessFootpaths.push_back(accessNode);
          }
        }
        else if (params.accessNodesRef.size() > 0 && params.accessNodeTravelTimesSeconds.size() == params.accessNodesRef.size())
        {
          accessFootpaths.clear();
          int j = 0;
          for (auto & accessNode : params.accessNodesRef)
          {
            int distanceMeters{-1};
            if (params.accessNodeDistancesMeters.size() == params.accessNodesRef.size())
            {
              distanceMeters = params.accessNodeDistancesMeters[j];
            }
            accessFootpaths.push_back(NodeTimeDistance(accessNode, params.accessNodeTravelTimesSeconds[j], distanceMeters));
            j++;
          }
        }
        else
        {
          spdlog::debug("  fetching nodes with osrm with mode {}", params.accessMode);

          accessFootpaths = OsrmFetcher::getAccessibleNodesFootpathsFromPoint(*parameters.getOrigin(), transitData.getNodes(), params.accessMode, parameters.getMaxAccessWalkingTravelTimeSeconds(), params.walkingSpeedMetersPerSecond);
          if (accessFootpaths.size() == 0) {
            accessFootpathOk = false;
          }
        }
      }

      spdlog::debug("  parsing access footpaths to find min/max access travel times");

      int footpathTravelTimeSeconds;
      int footpathDistanceMeters;
      nodesAccess.clear();
      forwardJourneysSteps.clear();
      nodesTentativeTime.clear();
      for (auto & accessFootpath : accessFootpaths)
      {
        footpathTravelTimeSeconds = (int)ceil((float)(accessFootpath.time) / params.walkingSpeedFactor);
        footpathDistanceMeters    = accessFootpath.distance;

        nodesAccess.emplace(accessFootpath.node.uid, NodeTimeDistance(accessFootpath.node,
                                                                      footpathTravelTimeSeconds,
                                                                      footpathDistanceMeters));
        forwardJourneysSteps[accessFootpath.node.uid]  = std::make_tuple(std::nullopt, std::nullopt, std::nullopt, footpathTravelTimeSeconds, -1, footpathDistanceMeters);
        nodesTentativeTime[accessFootpath.node.uid]    = departureTimeSeconds + footpathTravelTimeSeconds;
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
        if(odTripGlob.has_value())
        {

          spdlog::debug("  using odTrip with {} egressible nodes", odTripGlob.value().get().destinationNodes.size());

          egressFootpaths.clear();
          //TODO This could be a std::copy
          for (auto & egressNode : odTripGlob.value().get().destinationNodes)
          {
            egressFootpaths.push_back(egressNode);
          }
        }
        else if (params.egressNodesRef.size() > 0 && params.egressNodeTravelTimesSeconds.size() == params.egressNodesRef.size())
        {
          egressFootpaths.clear();
          int j = 0;
          for (auto & egressNode : params.egressNodesRef)
          {
            int distanceMeters{-1};
            if (params.egressNodeDistancesMeters.size() == params.egressNodesRef.size())
            {
              distanceMeters = params.egressNodeDistancesMeters[j];
            }
            egressFootpaths.push_back(NodeTimeDistance(egressNode, params.egressNodeTravelTimesSeconds[j], distanceMeters));
            j++;
          }
        }
        else
        {
          egressFootpaths = OsrmFetcher::getAccessibleNodesFootpathsFromPoint(*parameters.getDestination(), transitData.getNodes(), params.accessMode, parameters.getMaxEgressWalkingTravelTimeSeconds(), params.walkingSpeedMetersPerSecond);
          if (egressFootpaths.size() == 0) {
            egressFootpathOk = false;
          }
        }
      }
      
      spdlog::debug("  parsing egress footpaths to find min/max egress travel times");

      int footpathTravelTimeSeconds;
      int footpathDistanceMeters;
      nodesEgress.clear();
      reverseJourneysSteps.clear();
      nodesReverseTentativeTime.clear();
      for (auto & egressFootpath : egressFootpaths)
      {
        footpathTravelTimeSeconds  = (int)ceil((float)(egressFootpath.time) / params.walkingSpeedFactor);
        footpathDistanceMeters     = egressFootpath.distance;

        nodesEgress.emplace(egressFootpath.node.uid, NodeTimeDistance(egressFootpath.node,
                                                                       footpathTravelTimeSeconds,
                                                                       footpathDistanceMeters));

        reverseJourneysSteps[egressFootpath.node.uid] = std::make_tuple(std::nullopt, std::nullopt, std::nullopt, footpathTravelTimeSeconds, -1, footpathDistanceMeters);
        nodesReverseTentativeTime[egressFootpath.node.uid] = arrivalTimeSeconds - footpathTravelTimeSeconds;
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

      for (auto & tripIte : transitData.getTrips())
      {
        const Trip & trip = tripIte.second;
        bool enabled = true;

        if (enabled && parameters.getOnlyServices().size() > 0)
        {
          if (std::find(parameters.getOnlyServices().begin(), parameters.getOnlyServices().end(), trip.service) == parameters.getOnlyServices().end())
          {
            enabled = false;
          }
        }

        if (enabled && parameters.getOnlyLines().size() > 0)
        {
          if (std::find(parameters.getOnlyLines().begin(), parameters.getOnlyLines().end(), trip.line) == parameters.getOnlyLines().end())
          {
            enabled = false;
          }
        }

        if (enabled && parameters.getOnlyModes().size() > 0)
        {
          if (std::find(parameters.getOnlyModes().begin(), parameters.getOnlyModes().end(), trip.mode) == parameters.getOnlyModes().end())
          {
            enabled = false;
          }
        }

        if (enabled  && parameters.getOnlyNodes().size() > 0)
        {
          // FIXME: This is not right, it should look for a node, not the mode
          // FIXME2: Commented out, since mode is now typed, it won't match
          /*if (std::find(parameters.getOnlyNodesIdx()->begin(), parameters.getOnlyNodesIdx()->end(), trip->modeIdx) == parameters.getOnlyNodesIdx()->end())
          {
            enabled = -1;
          }(*/
        }

        if (enabled && parameters.getOnlyAgencies().size() > 0)
        {
          if (std::find(parameters.getOnlyAgencies().begin(), parameters.getOnlyAgencies().end(), trip.agency) == parameters.getOnlyAgencies().end())
          {
            enabled = false;
          }
        }

        if (enabled && parameters.getExceptServices().size() > 0)
        {
          if (std::find(parameters.getExceptServices().begin(), parameters.getExceptServices().end(), trip.service) != parameters.getExceptServices().end())
          {
            enabled = false;
          }
        }

        if (enabled && parameters.getExceptLines().size() > 0)
        {
          if (std::find(parameters.getExceptLines().begin(), parameters.getExceptLines().end(), trip.line) != parameters.getExceptLines().end())
          {
            enabled = false;
          }
        }

        if (enabled && parameters.getExceptNodes().size() > 0)
        {
          // FIXME: This is not right, it should look for a node, not the mode
          // FIXME2: Commented out, since mode is now typed, it won't match
          /*
          if (std::find(parameters.getExceptNodesIdx()->begin(), parameters.getExceptNodesIdx()->end(), trip.modeIdx) != parameters.getExceptNodesIdx()->end())
          {
            enabled = false;
            }*/
        }

        if (enabled && parameters.getExceptModes().size() > 0)
        {
          if (std::find(parameters.getExceptModes().begin(), parameters.getExceptModes().end(), trip.mode) != parameters.getExceptModes().end())
          {
            enabled = false;
          }
        }

        if (enabled && parameters.getExceptAgencies().size() > 0)
        {
          if (std::find(parameters.getExceptAgencies().begin(), parameters.getExceptAgencies().end(), trip.agency) != parameters.getExceptAgencies().end())
          {
            enabled = false;
          }
        }
        tripsEnabled[trip.uid] = enabled;
      }
    }

    
    benchmarking["reset"] += algorithmCalculationTime.getEpoch() - benchmarkingStart;
    
    
    spdlog::debug("-- filter trips -- {} microseconds ", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);

    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

  }

}
