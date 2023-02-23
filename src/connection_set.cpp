#include "connection_set.hpp"
#include "spdlog/spdlog.h"
#include "connection.hpp"


namespace TrRouting {

  const int CONNECTION_ITERATOR_CACHE_BEGIN_HOUR = 0;
  const int CONNECTION_ITERATOR_CACHE_END_HOUR = 32;

  std::vector<std::reference_wrapper<Connection>>::const_iterator ConnectionSet::getForwardConnectionsBeginAtDepartureHour(int hour) const
  {
    if (hour > CONNECTION_ITERATOR_CACHE_END_HOUR || hour  < CONNECTION_ITERATOR_CACHE_BEGIN_HOUR) {
      return forwardConnections.cend();
    }

    return forwardConnectionsBeginIteratorCache[hour];
  };

  std::vector<std::reference_wrapper<Connection>>::const_iterator ConnectionSet::getReverseConnectionsBeginAtArrivalHour(int hour) const
  {
    if (hour < CONNECTION_ITERATOR_CACHE_BEGIN_HOUR) {
      return reverseConnections.cend();
    } else if (hour > CONNECTION_ITERATOR_CACHE_END_HOUR - 1) {
      return reverseConnections.begin();
    }

    return reverseConnectionsBeginIteratorCache[hour];
  };

  // Create a cache with a begin iterator which match the connection closest to the specified hour
  void ConnectionSet::generateConnectionsIteratorCache() {
    int currentHour = CONNECTION_ITERATOR_CACHE_BEGIN_HOUR;

    // Create first part of the forward cache
    // For each hour, we save the iterator matching the connection which as a departure time bigger
    // than this hour
    for (std::vector<std::reference_wrapper<Connection>>::const_iterator ite = forwardConnections.cbegin();
         ite != forwardConnections.cend();
         ite++) {
      // We can have a gap of multiple hours between 2 connections, so the current iterator
      // can be valid for multiple hour slots
      while ((*ite).get().getDepartureTime() >= currentHour * 3600) {
        forwardConnectionsBeginIteratorCache.push_back(ite);
        currentHour++;
      }
    }

    //Finish filling forward cache
    //We reached the end of the forwardConnections in the previous loop, so here we
    //fill the rest of the forward cache with end iterator
    for (;currentHour < CONNECTION_ITERATOR_CACHE_END_HOUR; currentHour++) {
      forwardConnectionsBeginIteratorCache.push_back(forwardConnections.cend());
    }

    // Create the reverse cache
    currentHour = CONNECTION_ITERATOR_CACHE_END_HOUR - 1;
    for (std::vector<std::reference_wrapper<Connection>>::const_iterator ite = reverseConnections.cbegin();
         ite != reverseConnections.cend();
         ite++) {
      // Fill cache with same iterator if we have multi hour gap between connection
      while ((*ite).get().getArrivalTime() <= currentHour * 3600 && currentHour > CONNECTION_ITERATOR_CACHE_BEGIN_HOUR) {
        reverseConnectionsBeginIteratorCache.insert(reverseConnectionsBeginIteratorCache.begin(),ite);
        currentHour--;
      }
    }

    //Finish filling reverse cache
    //We reached the end of the reverse connections in the previous loop, so here we
    //fill the rest of the reverse cache with end iterator
    for (;currentHour >= CONNECTION_ITERATOR_CACHE_BEGIN_HOUR; currentHour--) {
      reverseConnectionsBeginIteratorCache.insert(reverseConnectionsBeginIteratorCache.begin(), reverseConnections.cend());
    }
  }

  ConnectionSet::ConnectionSet(
    const std::vector<std::reference_wrapper<const Trip>> _trips,
    const std::vector<std::reference_wrapper<Connection>> _forwardConnections,
    const std::vector<std::reference_wrapper<Connection>> _reverseConnections
  ): trips(_trips), forwardConnections(_forwardConnections), reverseConnections(_reverseConnections) {
    generateConnectionsIteratorCache();
  }

}