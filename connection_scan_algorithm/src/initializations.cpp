#include "calculator.hpp"

namespace TrRouting
{
  
  Calculator::Calculator(Parameters& theParams) : params(theParams)
  {
    
    algorithmCalculationTime = CalculationTime();

  }

  void Calculator::initializeCalculationData() {
    nodesTentativeTime.clear();
    nodesTentativeTime.shrink_to_fit();
    nodesTentativeTime.resize(nodes.size());
    nodesReverseTentativeTime.clear();
    nodesReverseTentativeTime.shrink_to_fit();
    nodesReverseTentativeTime.resize(nodes.size());
    nodesAccessTravelTime.clear();
    nodesAccessTravelTime.shrink_to_fit();
    nodesAccessTravelTime.resize(nodes.size());
    nodesAccessDistance.clear();
    nodesAccessDistance.shrink_to_fit();
    nodesAccessDistance.resize(nodes.size());
    nodesEgressTravelTime.clear();
    nodesEgressTravelTime.shrink_to_fit();
    nodesEgressTravelTime.resize(nodes.size());
    nodesEgressDistance.clear();
    nodesEgressDistance.shrink_to_fit();
    nodesEgressDistance.resize(nodes.size());
    forwardJourneysSteps.clear();
    forwardJourneysSteps.shrink_to_fit();
    forwardJourneysSteps.resize(nodes.size());
    forwardEgressJourneysSteps.clear();
    forwardEgressJourneysSteps.shrink_to_fit();
    forwardEgressJourneysSteps.resize(nodes.size());
    reverseJourneysSteps.clear();
    reverseJourneysSteps.shrink_to_fit();
    reverseJourneysSteps.resize(nodes.size());
    reverseAccessJourneysSteps.clear();
    reverseAccessJourneysSteps.shrink_to_fit();
    reverseAccessJourneysSteps.resize(nodes.size());

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

    std::cout << forwardConnections.size() << " connections" << std::endl; 

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
        //std::cout << hour << ":" << i << ":" << std::get<connectionIndexes::TIME_DEP>(connection) << std::endl;
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
        //std::cout << hour << ":" << i << ":" << std::get<connectionIndexes::TIME_ARR>(connection) << std::endl;
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
      //std::cout << h << ": " << forwardConnectionsIndexPerDepartureTimeHour[h] << std::endl;
    }

  }
  

}
