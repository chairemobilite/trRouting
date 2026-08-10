#include "spdlog/spdlog.h"
#include "calculator.hpp"
#include "parameters.hpp"
#include "trip.hpp"
#include "toolbox.hpp" //MAX_INT
#include "routing_result.hpp"
#include "mode.hpp"
#include "agency.hpp"
#include "service.hpp"
#include "line.hpp"
#include "node.hpp"
#include "transit_data.hpp"
#include "connection_set.hpp"
#include "geofilter.hpp"
#include "alternative_filter.hpp"

namespace TrRouting
{

  void Calculator::reset(const CommonParameters &parameters, std::optional<std::reference_wrapper<const Point>> origin, std::optional<std::reference_wrapper<const Point>> destination, bool resetAccessPaths, AlternativeFilter *alternativeFilter)
  {
    
    //TODO Should we just check the size of accessFootpath and egressFootpath instead of adding a flag?
    bool accessFootpathOk = true;
    bool egressFootpathOk = true;

    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

    if (resetAccessPaths)
    {
      accessFootpaths.clear();
      egressFootpaths.clear();
    }
    tripsQueryOverlay.assign(Trip::getMaxUid()+1, TripQueryData());
    forwardJourneysSteps.clear();
    reverseJourneysSteps.clear();
    nodesAccess.clear();
    nodesEgress.clear();

    
    departureTimeSeconds = -1;
    arrivalTimeSeconds   = -1;
    
    if (parameters.isForwardCalculation())
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

    // Save a copy of the current connection set
    connectionSet = transitData.getConnectionsForScenario(parameters.getScenario());

    // disable trips according for alternatives. The disabled flags were already cleared by the
    // tripsQueryOverlay.assign() above, so we don't carry the previous filter around.
    if (alternativeFilter) {
      alternativeFilter->runFilter(tripsQueryOverlay, (*connectionSet));
    }

    spdlog::debug("-- filter trips -- {} microseconds ", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);

    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

  }

  bool Calculator::resetAccessFootpaths(const CommonParameters &parameters, const Point& origin) {
    spdlog::debug("  resetting access paths ");
    bool accessFootpathOk = true;

    spdlog::debug("  fetching nodes with osrm");

    accessFootpaths = geoFilter.getAccessibleNodesFootpathsFromPoint(origin, transitData.getNodes(), parameters.getMaxAccessWalkingTravelTimeSeconds(), parameters.getWalkingSpeedMetersPerSecond());
    if (accessFootpaths.size() == 0) {
      accessFootpathOk = false;
    }

    return accessFootpathOk;
  }

  bool Calculator::resetEgressFootpaths(const CommonParameters &parameters, const Point & destination) {
    bool egressFootpathOk = true;

    // fetch nodes footpaths accessible to destination using params or osrm fetcher if not provided:
    egressFootpaths = geoFilter.getAccessibleNodesFootpathsFromPoint(destination, transitData.getNodes(), parameters.getMaxEgressWalkingTravelTimeSeconds(), parameters.getWalkingSpeedMetersPerSecond());
    if (egressFootpaths.size() == 0) {
      egressFootpathOk = false;
    }

    return egressFootpathOk;
  }

}
