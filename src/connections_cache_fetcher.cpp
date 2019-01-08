
#ifndef TR_CONNECTIONS_CACHE_FETCHER
#define TR_CONNECTIONS_CACHE_FETCHER

#include <string>
#include <vector>

#include "cache_fetcher.hpp"
#include "capnp/connectionsCollection.capnp.h"

namespace TrRouting
{

  const std::pair<std::vector<std::tuple<int,int,int,int,int,short,short,int>>, std::vector<std::tuple<int,int,int,int,int,short,short,int>>> CacheFetcher::getConnections(std::string projectShortname, std::map<unsigned long long, int> nodeIndexesByUuid, std::map<unsigned long long, int> tripIndexesByUuid)
  {
    
  }

}

#endif // TR_CONNECTIONS_CACHE_FETCHER