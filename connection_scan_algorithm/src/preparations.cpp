#include "calculator.hpp"

namespace TrRouting
{
    
  void Calculator::prepare()
  {
    
    if (params.debugDisplay)
      std::cerr << "preparing nodes, routes, trips, connections and footpaths..." << std::endl;
    if (params.dataFetcherShortname == "cache")
    {
      std::tie(agencies, agencyIndexesByUuid)                = params.cacheFetcher->getAgencies(params.projectShortname);
      //std::tie(nodes, nodeIndexesById)                   = params.cacheFetcher->getNodes(params.projectShortname);
      //if (params.updateOdTrips == 1) // only update od trips if set as parameter (1) when launching app, because this takes a long time. Call only if nodes and/or od trips were modified.
      //{
      //  std::tie(odTrips, odTripIndexesById, odTripFootpaths) = params.databaseFetcher->getOdTrips(params.projectShortname, nodes, params);
      //}
      //else
      //{
      //  std::tie(odTrips, odTripIndexesById)             = params.cacheFetcher->getOdTrips(params.projectShortname, nodes, params);
      //  odTripFootpaths                                  = params.cacheFetcher->getOdTripFootpaths(params.projectShortname, params);
      //}
      //std::tie(routes, routeIndexesById)                 = params.cacheFetcher->getRoutes(params.projectShortname);
      //std::tie(trips, tripIndexesById)                   = params.cacheFetcher->getTrips(params.projectShortname);
      //std::tie(forwardConnections, reverseConnections)   = params.cacheFetcher->getConnections(params.projectShortname, nodeIndexesById, tripIndexesById);
      //std::tie(footpaths, footpathsRanges)               = params.cacheFetcher->getFootpaths(params.projectShortname, nodeIndexesById);
    }
    //else if (params.dataFetcherShortname == "gtfs")
    //{
    //  std::tie(nodes, nodeIndexesById)                 = params.gtfsFetcher->getNodes(params.projectShortname);
    //  std::tie(routes, routeIndexesById)               = params.gtfsFetcher->getRoutes(params.projectShortname);
    //  std::tie(trips, tripIndexesById)                 = params.gtfsFetcher->getTrips(params.projectShortname);
    //  std::tie(forwardConnections, reverseConnections) = params.gtfsFetcher->getConnections(params.projectShortname, nodeIndexesById, tripIndexesById);
    //  std::tie(footpaths, footpathsRanges)             = params.gtfsFetcher->getFootpaths(params.projectShortname, nodeIndexesById);
    //}
    //else if (params.dataFetcherShortname == "csv")
    //{
    //  std::tie(nodes, nodeIndexesById)                 = params.csvFetcher->getNodes(params.projectShortname);
    //  std::tie(routes, routeIndexesById)               = params.csvFetcher->getRoutes(params.projectShortname);
    //  std::tie(trips, tripIndexesById)                 = params.csvFetcher->getTrips(params.projectShortname);
    //  std::tie(forwardConnections, reverseConnections) = params.csvFetcher->getConnections(params.projectShortname, nodeIndexesById, tripIndexesById);
    //  std::tie(footpaths, footpathsRanges)             = params.csvFetcher->getFootpaths(params.projectShortname, nodeIndexesById);
    //}
    
    std::cout << "preparing nodes tentative times, trips enter connections and journeys..." << std::endl;
    
    //nodesTentativeTime                     = std::vector<int>(nodes.size());
    //nodesReverseTentativeTime              = std::vector<int>(nodes.size());
    //nodesAccessTravelTime                  = std::vector<int>(nodes.size());
    //nodesEgressTravelTime                  = std::vector<int>(nodes.size());
    //tripsEnterConnection                   = std::vector<int>(trips.size());
    //tripsExitConnection                    = std::vector<int>(trips.size());
    //tripsEnterConnectionTransferTravelTime = std::vector<int>(trips.size());
    //tripsExitConnectionTransferTravelTime  = std::vector<int>(trips.size());
    //tripsEnabled                           = std::vector<int>(trips.size());
    //tripsUsable                            = std::vector<int>(trips.size());
    //forwardJourneys                        = std::vector<std::tuple<int,int,long long,int,int,short>>(nodes.size());
    //forwardEgressJourneys                  = std::vector<std::tuple<int,int,long long,int,int,short>>(nodes.size());
    //reverseJourneys                        = std::vector<std::tuple<int,int,long long,int,int,short>>(nodes.size());
    //reverseAccessJourneys                  = std::vector<std::tuple<int,int,long long,int,int,short>>(nodes.size());
    
    
  }
  
}
