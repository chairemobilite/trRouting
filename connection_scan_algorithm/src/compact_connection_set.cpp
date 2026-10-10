#include "compact_connection_set.hpp"
#include "connection_set.hpp"
#include "connection.hpp"
#include "node.hpp"
#include "line.hpp"
#include "trip.hpp"

namespace TrRouting {

  CompactConnectionSet::CompactConnectionSet(const ConnectionSet &connectionSet) {
    const auto &trips = connectionSet.getTrips();
    std::vector<int32_t> tripIndexByUid(Trip::getMaxUid() + 1, -1);
    tripLineUid.reserve(trips.size());
    for (const Trip &trip : trips) {
      tripIndexByUid[trip.uid] = tripLineUid.size();
      tripLineUid.push_back(trip.line.uid);
    }

    const auto &connections = connectionSet.getForwardConnections();
    const size_t count = connections.size();
    departureNodeUid.reserve(count);
    arrivalNodeUid.reserve(count);
    departureTime.reserve(count);
    arrivalTime.reserve(count);
    tripIndex.reserve(count);
    minWaitingTime.reserve(count);
    flags.reserve(count);
    for (const Connection &connection : connections) {
      departureNodeUid.push_back(connection.getDepartureNode().uid);
      arrivalNodeUid.push_back(connection.getArrivalNode().uid);
      departureTime.push_back(connection.getDepartureTime());
      arrivalTime.push_back(connection.getArrivalTime());
      tripIndex.push_back(tripIndexByUid[connection.getTrip().uid]);
      minWaitingTime.push_back(connection.getMinWaitingTime());
      flags.push_back((connection.canBoard() ? CAN_BOARD : 0) | (connection.canUnboard() ? CAN_UNBOARD : 0));
    }
  }

}
