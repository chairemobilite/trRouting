#include "spdlog/spdlog.h"
#include "calculator.hpp"
#include "parameters.hpp"
#include "trip.hpp"
#include "toolbox.hpp" //MAX_INT
#include "od_trip.hpp"
#include "routing_result.hpp"
#include "mode.hpp"
#include "agency.hpp"
#include "service.hpp"
#include "line.hpp"
#include "node.hpp"
#include "transit_data.hpp"
#include "connection_set.hpp"
#include "geofilter.hpp"

namespace TrRouting
{

  void Calculator::reset(CommonParameters &parameters, std::optional<std::reference_wrapper<const Point>> origin, std::optional<std::reference_wrapper<const Point>> destination, bool resetAccessPaths, bool doResetFilters)
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
    tripsQueryOverlay.assign(Trip::getMaxUid()+1, TripQueryData());
    forwardJourneysSteps.clear();
    reverseJourneysSteps.clear();

    
    departureTimeSeconds = -1;
    arrivalTimeSeconds   = -1;
    
    if(odTripGlob.has_value() && parameters.isForwardCalculation())
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

    //TODO Question, do we only use accessFootpath when those condtion are true? The whole calculation should probably
    // be a different path in this case.
    if (origin.has_value())
    {
      if (resetAccessPaths)
      {
        accessFootpathOk = resetAccessFootpaths(parameters, origin.value());
      }

      spdlog::debug("  parsing access footpaths to find min/max access travel times");

      int footpathTravelTimeSeconds;
      int footpathDistanceMeters;
      nodesAccess.clear();
      forwardJourneysSteps.assign(Node::getMaxUid() + 1, JourneyStep());
      nodesTentativeTime.assign(Node::getMaxUid() + 1, MAX_INT); //Assign default values to all indexes
      
      for (auto & accessFootpath : accessFootpaths)
      {
        footpathTravelTimeSeconds = (int)ceil((float)(accessFootpath.time) / parameters.getWalkingSpeedFactor());
        footpathDistanceMeters    = accessFootpath.distance;

        nodesAccess.emplace(accessFootpath.node.uid, NodeTimeDistance(accessFootpath.node,
                                                                      footpathTravelTimeSeconds,
                                                                      footpathDistanceMeters));
        forwardJourneysSteps.at(accessFootpath.node.uid) = JourneyStep(std::nullopt, std::nullopt, std::nullopt, footpathTravelTimeSeconds, false, footpathDistanceMeters);
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
  
    if (destination.has_value())
    {
      if (resetAccessPaths)
      {
        egressFootpathOk = resetEgressFootpaths(parameters, destination.value());
      }
      
      spdlog::debug("  parsing egress footpaths to find min/max egress travel times");

      int footpathTravelTimeSeconds;
      int footpathDistanceMeters;
      nodesEgress.clear();
      reverseJourneysSteps.assign(Node::getMaxUid() + 1, JourneyStep());
      nodesReverseTentativeTime.assign(Node::getMaxUid() + 1, -1); //Assign default values to all indexes
      for (auto & egressFootpath : egressFootpaths)
      {
        footpathTravelTimeSeconds  = (int)ceil((float)(egressFootpath.time) / parameters.getWalkingSpeedFactor());
        footpathDistanceMeters     = egressFootpath.distance;

        nodesEgress.emplace(egressFootpath.node.uid, NodeTimeDistance(egressFootpath.node,
                                                                       footpathTravelTimeSeconds,
                                                                       footpathDistanceMeters));

        reverseJourneysSteps.at(egressFootpath.node.uid) = JourneyStep(std::nullopt, std::nullopt, std::nullopt, footpathTravelTimeSeconds, false, footpathDistanceMeters);
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

    if (doResetFilters)
    {
      resetFilters(parameters);
    }

    benchmarking["reset"] += algorithmCalculationTime.getEpoch() - benchmarkingStart;

    spdlog::debug("-- filter trips -- {} microseconds ", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);

    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

  }

  bool Calculator::resetAccessFootpaths(const CommonParameters &parameters, const Point& origin) {
    spdlog::debug("  resetting access paths ");
    bool accessFootpathOk = true;

    if(odTripGlob.has_value()) {
      spdlog::debug("  using odTrip with {} accessible nodes", odTripGlob.value().get().originNodes.size());

      accessFootpaths.clear();
      //TODO This can be a std::copy
      for (auto & accessNode : odTripGlob.value().get().originNodes) {
        accessFootpaths.push_back(accessNode);
      }
    }
    else if (params.accessNodesRef.size() > 0 && params.accessNodeTravelTimesSeconds.size() == params.accessNodesRef.size()) {
      accessFootpaths.clear();
      int j = 0;
      for (auto & accessNode : params.accessNodesRef) {
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
      spdlog::debug("  fetching nodes with osrm");

      accessFootpaths = geoFilter.getAccessibleNodesFootpathsFromPoint(origin, transitData.getNodes(), parameters.getMaxAccessWalkingTravelTimeSeconds(), parameters.getWalkingSpeedMetersPerSecond());
      if (accessFootpaths.size() == 0) {
        accessFootpathOk = false;
      }
    }
    return accessFootpathOk;
  }

  bool Calculator::resetEgressFootpaths(const CommonParameters &parameters, const Point & destination) {
    bool egressFootpathOk = true;

    // fetch nodes footpaths accessible to destination using params or osrm fetcher if not provided:
    if(odTripGlob.has_value())
    {
      spdlog::debug("  using odTrip with {} egressible nodes", odTripGlob.value().get().destinationNodes.size());

      egressFootpaths.clear();
      //TODO This could be a std::copy
      for (auto & egressNode : odTripGlob.value().get().destinationNodes) {
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
      egressFootpaths = geoFilter.getAccessibleNodesFootpathsFromPoint(destination, transitData.getNodes(), parameters.getMaxEgressWalkingTravelTimeSeconds(), parameters.getWalkingSpeedMetersPerSecond());
      if (egressFootpaths.size() == 0) {
        egressFootpathOk = false;
      }
    }
    return egressFootpathOk;
  }

  void Calculator::resetFilters(const CommonParameters &parameters) {
    spdlog::debug("  resetting filters");

    // TODO med term. Instead of the scenario as cache key, it could be the parameters, with services, lines and agencies
    connectionSet = transitData.getConnectionsForScenario(parameters.getScenario());

    // This loop is required for alternatives, where parameters have more
    // exclusions than the scenario (the combinations of lines). It is not
    // redundant with the one in the connection cache generator
    // TODO: This code is very similar to the one in getConnectionsForScenario, extract it
    tripsDisabled.clear();
    for (auto & tripIte : connectionSet.get()->getTrips())
    {
      const Trip & trip = tripIte.get();
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
      if (!enabled) {
        tripsDisabled[trip.uid] = true;
      }
    }

  }

}
