#include <boost/uuid/uuid.hpp>
#include "spdlog/spdlog.h"

#include "calculator.hpp"
#include "trip.hpp"
#include "mode.hpp"
#include "agency.hpp"
#include "data_source.hpp"
#include "node.hpp"
#include "line.hpp"
#include "path.hpp"
#include "scenario.hpp"
#include "service.hpp"
#include "od_trip.hpp"
#include "place.hpp"
#include "household.hpp"
#include "person.hpp"


namespace TrRouting
{

  Calculator::Calculator(DataFetcher& fetcher) :
    dataFetcher(fetcher),
    odTripGlob(std::nullopt),
    algorithmCalculationTime(CalculationTime()),
    departureTimeSeconds(0),
    initialDepartureTimeSeconds(0),
    arrivalTimeSeconds(-1),
    maxTimeValue(0),
    minAccessTravelTime(0),
    maxEgressTravelTime(0),
    maxAccessTravelTime(0),
    minEgressTravelTime(0),
    maxAccessWalkingTravelTimeFromOriginToFirstNodeSeconds(0),
    maxAccessWalkingTravelTimeFromLastNodeToDestinationSeconds(0),
    calculationTime(0),
    accessMode("walking"),
    egressMode("walking")
  {

  }

  void Calculator::initializeCalculationData() {
    nodesTentativeTime.clear();
    nodesReverseTentativeTime.clear();
    nodesAccess.clear();
    nodesEgress.clear();
    forwardJourneysSteps.clear();
    forwardEgressJourneysSteps.clear();
    reverseJourneysSteps.clear();
    reverseAccessJourneysSteps.clear();

    tripsEnabled.clear();
    tripsEnabled.shrink_to_fit();
    tripsEnabled.resize(trips.size());
    tripsUsable.clear();
    tripsUsable.shrink_to_fit();
    tripsUsable.resize(trips.size());
    tripsEnterConnection.clear();
    tripsEnterConnection.shrink_to_fit();
    tripsEnterConnection.resize(trips.size());
    tripsExitConnection.clear();
    tripsExitConnection.shrink_to_fit();
    tripsExitConnection.resize(trips.size());
    tripsEnterConnectionTransferTravelTime.clear();
    tripsEnterConnectionTransferTravelTime.shrink_to_fit();
    tripsEnterConnectionTransferTravelTime.resize(trips.size());
    tripsExitConnectionTransferTravelTime.clear();
    tripsExitConnectionTransferTravelTime.shrink_to_fit();
    tripsExitConnectionTransferTravelTime.resize(trips.size());

    spdlog::info("{} connections", forwardConnections.size());;

    //int benchmarkingStart = algorithmCalculationTime.getEpoch();

    int lastConnectionIndex = forwardConnections.size() - 1;

    forwardConnectionsIndexPerDepartureTimeHour = std::vector<int>(32, -1);
    reverseConnectionsIndexPerArrivalTimeHour   = std::vector<int>(32, lastConnectionIndex);

    int hour {0};
    int i = 0;
    for (auto & connection : forwardConnections)
    {
      while (std::get<connectionIndexes::TIME_DEP>(*connection) >= hour * 3600 && forwardConnectionsIndexPerDepartureTimeHour[hour] == -1 && hour < 32)
      {
        forwardConnectionsIndexPerDepartureTimeHour[hour] = i;
        hour++;
      }
      i++;
    }

    hour = 31;
    i = 0;
    for (auto & connection : reverseConnections)
    {
      while (std::get<connectionIndexes::TIME_ARR>(*connection) <= hour * 3600 && reverseConnectionsIndexPerArrivalTimeHour[hour] == lastConnectionIndex && hour >= 0)
      {
        reverseConnectionsIndexPerArrivalTimeHour[hour] = i;
        hour--;
      }
      i++;
    }

    for (int h = 0; h < 32; h++)
    {
      if (forwardConnectionsIndexPerDepartureTimeHour[h] == -1)
      {
        forwardConnectionsIndexPerDepartureTimeHour[h] = lastConnectionIndex;
      }
    }

  }

  //TODO This should probably take a reference to the connections object and later not use the std::move semantic
  int Calculator::setConnections(std::vector<std::shared_ptr<ConnectionTuple>> connections)
  {

    // Copy the connections to both forward and reverse vectors
    forwardConnections.clear();
    reverseConnections.clear();
    for (int i=0; i<connections.size(); i++)
    {
      std::shared_ptr<ConnectionTuple> reverseConnection = connections[i];
      forwardConnections.push_back(std::move(connections[i]));
      reverseConnections.push_back(std::move(reverseConnection));
    }
    forwardConnections.shrink_to_fit();
    reverseConnections.shrink_to_fit();

    int tripIdx {-1};
    try
    {
      spdlog::info("Sorting connections...");
      // Sort forward connections by departure time, trip id, sequence
      std::stable_sort(forwardConnections.begin(), forwardConnections.end(), [](const std::shared_ptr<ConnectionTuple>& connectionA, const std::shared_ptr<ConnectionTuple>& connectionB)
      {
        // Copied from calculator.hpp
        // { NODE_DEP = 0, NODE_ARR = 1, TIME_DEP = 2, TIME_ARR = 3, TRIP = 4, CAN_BOARD = 5, CAN_UNBOARD = 6, SEQUENCE = 7, CAN_TRANSFER_SAME_LINE = 8, MIN_WAITING_TIME_SECONDS = 8 };
        if (std::get<connectionIndexes::TIME_DEP>(*connectionA) < std::get<connectionIndexes::TIME_DEP>(*connectionB))
        {
          return true;
        }
        else if (std::get<connectionIndexes::TIME_DEP>(*connectionA) > std::get<connectionIndexes::TIME_DEP>(*connectionB))
        {
          return false;
        }
        if (std::get<connectionIndexes::TRIP>(*connectionA) < std::get<connectionIndexes::TRIP>(*connectionB))
        {
          return true;
        }
        else if (std::get<connectionIndexes::TRIP>(*connectionA) > std::get<connectionIndexes::TRIP>(*connectionB))
        {
          return false;
        }
        if (std::get<connectionIndexes::SEQUENCE>(*connectionA) < std::get<connectionIndexes::SEQUENCE>(*connectionB))
        {
          return true;
        }
        else if (std::get<connectionIndexes::SEQUENCE>(*connectionA) > std::get<connectionIndexes::SEQUENCE>(*connectionB))
        {
          return false;
        }
        return false;
      });
      // Sort reverse connection by arrival time, trip and sequence
      std::stable_sort(reverseConnections.begin(), reverseConnections.end(), [](const std::shared_ptr<ConnectionTuple>& connectionA, const std::shared_ptr<ConnectionTuple>& connectionB)
      {
        if (std::get<connectionIndexes::TIME_ARR>(*connectionA) > std::get<connectionIndexes::TIME_ARR>(*connectionB))
        {
          return true;
        }
        else if (std::get<connectionIndexes::TIME_ARR>(*connectionA) < std::get<connectionIndexes::TIME_ARR>(*connectionB))
        {
          return false;
        }
        if (std::get<connectionIndexes::TRIP>(*connectionA) > std::get<connectionIndexes::TRIP>(*connectionB)) // here we need to reverse sequence!
        {
          return true;
        }
        else if (std::get<connectionIndexes::TRIP>(*connectionA) < std::get<connectionIndexes::TRIP>(*connectionB))
        {
          return false;
        }
        if (std::get<connectionIndexes::SEQUENCE>(*connectionA) > std::get<connectionIndexes::SEQUENCE>(*connectionB)) // here we need to reverse sequence!
        {
          return true;
        }
        else if (std::get<connectionIndexes::SEQUENCE>(*connectionA) < std::get<connectionIndexes::SEQUENCE>(*connectionB))
        {
          return false;
        }
        return false;
      });

      CalculationTime algorithmCalculationTime = CalculationTime();
      algorithmCalculationTime.start();
      long long       calculationTime;
      calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

      // assign connections to trips:
      int i {0};
      for(auto & connection : forwardConnections)
      {
        tripIdx = std::get<connectionIndexes::TRIP>(*connection);
        trips[tripIdx]->forwardConnectionsIdx.push_back(i);
        i++;
      }

      i = 0;
      for(auto & connection : reverseConnections)
      {
        tripIdx = std::get<connectionIndexes::TRIP>(*connection);
        trips[tripIdx]->reverseConnectionsIdx.push_back(i);
        i++;
      }

      spdlog::debug("-- assign connections to trips -- {} microseconds", algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime);
      return 0;
    }
    catch (const std::exception& ex)
    {
      spdlog::error("-- Error assigning connections to trips -- {} ", ex.what());
      return -EINVAL;
    }
  }


}
