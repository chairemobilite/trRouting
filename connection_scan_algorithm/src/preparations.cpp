#include "calculator.hpp"

namespace TrRouting
{
    
  void Calculator::prepare()
  {
    
    if (params.debugDisplay)
      std::cerr << "preparing stops, routes, trips, connections and footpaths..." << std::endl;
    if (params.dataFetcherShortname == "database")
    {
      std::tie(stops, stopIndexesById)                 = params.databaseFetcher->getStops(params.applicationShortname);
      std::tie(routes, routeIndexesById)               = params.databaseFetcher->getRoutes(params.applicationShortname);
      std::tie(trips, tripIndexesById)                 = params.databaseFetcher->getTrips(params.applicationShortname);
      std::tie(forwardConnections, reverseConnections) = params.databaseFetcher->getConnections(params.applicationShortname, stopIndexesById, tripIndexesById);
      std::tie(footpaths, footpathsRanges)             = params.databaseFetcher->getFootpaths(params.applicationShortname, stopIndexesById);
      std::tie(odTrips, odTripIndexesById)             = params.cacheFetcher->getOdTrips(params.applicationShortname, stops, params);
      if (params.updateOdTrips == 1) // only update od trips if set as parameter (1) when launching app, because this takes a long time. Call only if stops and/or od trips were modified.
      {
        std::tie(odTrips, odTripIndexesById)           = params.databaseFetcher->getOdTrips(params.applicationShortname, stops, params);
      }
    }
    else if (params.dataFetcherShortname == "cache")
    {
      std::tie(stops, stopIndexesById)                   = params.cacheFetcher->getStops(params.applicationShortname);
      //if (stops.size() == 0)
      //{
      //  std::tie(stops, stopIndexesById)                 = params.databaseFetcher->getStops(params.applicationShortname);
      //}
      std::tie(routes, routeIndexesById)                 = params.cacheFetcher->getRoutes(params.applicationShortname);
      //if (routes.size() == 0)
      //{
      //  std::tie(routes, routeIndexesById)               = params.databaseFetcher->getRoutes(params.applicationShortname);
      //}
      std::tie(trips, tripIndexesById)                   = params.cacheFetcher->getTrips(params.applicationShortname);
      //if (trips.size() == 0)
      //{
      //  std::tie(trips, tripIndexesById)                 = params.databaseFetcher->getTrips(params.applicationShortname);
      //}
      std::tie(forwardConnections, reverseConnections)   = params.cacheFetcher->getConnections(params.applicationShortname, stopIndexesById, tripIndexesById);
      //if (forwardConnections.size() == 0)
      //{
      //  std::tie(forwardConnections, reverseConnections) = params.databaseFetcher->getConnections(params.applicationShortname, stopIndexesById, tripIndexesById);
      //}
      std::tie(footpaths, footpathsRanges)               = params.cacheFetcher->getFootpaths(params.applicationShortname, stopIndexesById);
      //if (footpaths.size() == 0)
      //{
      //  std::tie(footpaths, footpathsRanges)             = params.databaseFetcher->getFootpaths(params.applicationShortname, stopIndexesById);
      //}
      std::tie(odTrips, odTripIndexesById)               = params.cacheFetcher->getOdTrips(params.applicationShortname, stops, params);
      //if (odTrips.size() == 0)
      //{
      //  std::tie(odTrips, odTripIndexesById)             = params.databaseFetcher->getOdTrips(params.applicationShortname, stops, params);
      //}
    }
    else if (params.dataFetcherShortname == "gtfs")
    {
      std::tie(stops, stopIndexesById)                 = params.gtfsFetcher->getStops(params.applicationShortname);
      std::tie(routes, routeIndexesById)               = params.gtfsFetcher->getRoutes(params.applicationShortname);
      std::tie(trips, tripIndexesById)                 = params.gtfsFetcher->getTrips(params.applicationShortname);
      std::tie(forwardConnections, reverseConnections) = params.gtfsFetcher->getConnections(params.applicationShortname, stopIndexesById, tripIndexesById);
      std::tie(footpaths, footpathsRanges)             = params.gtfsFetcher->getFootpaths(params.applicationShortname, stopIndexesById);
    }
    else if (params.dataFetcherShortname == "csv")
    {
      std::tie(stops, stopIndexesById)                 = params.csvFetcher->getStops(params.applicationShortname);
      std::tie(routes, routeIndexesById)               = params.csvFetcher->getRoutes(params.applicationShortname);
      std::tie(trips, tripIndexesById)                 = params.csvFetcher->getTrips(params.applicationShortname);
      std::tie(forwardConnections, reverseConnections) = params.csvFetcher->getConnections(params.applicationShortname, stopIndexesById, tripIndexesById);
      std::tie(footpaths, footpathsRanges)             = params.csvFetcher->getFootpaths(params.applicationShortname, stopIndexesById);
    }
    
    std::cout << "preparing stops tentative times, trips enter connections and journeys..." << std::endl;
    
    stopsTentativeTime                     = std::vector<int>(stops.size());
    stopsReverseTentativeTime              = std::vector<int>(stops.size()); //std::vector<std::deque<std::pair<int,int>>>(stops.size());
    //stopsD                                 = std::vector<int>(stops.size());
    stopsAccessTravelTime                  = std::vector<int>(stops.size());
    stopsEgressTravelTime                  = std::vector<int>(stops.size());
    tripsEnterConnection                   = std::vector<int>(trips.size());
    tripsExitConnection                    = std::vector<int>(trips.size());
    //tripsReverseTime                       = std::vector<int>(trips.size());
    tripsEnterConnectionTransferTravelTime = std::vector<int>(trips.size());
    tripsExitConnectionTransferTravelTime  = std::vector<int>(trips.size());
    tripsEnabled                           = std::vector<int>(trips.size());
    tripsUsable                            = std::vector<int>(trips.size());
    forwardJourneys                        = std::vector<std::tuple<int,int,int,int,int,short>>(stops.size());
    forwardEgressJourneys                  = std::vector<std::tuple<int,int,int,int,int,short>>(stops.size());
    reverseJourneys                        = std::vector<std::tuple<int,int,int,int,int,short>>(stops.size());
    reverseAccessJourneys                  = std::vector<std::tuple<int,int,int,int,int,short>>(stops.size());
    
    // uncomment this to see max access footpath travel time found in all odTrips:
    //int maxAccessTravelTime = 0;
    //for (auto & odTrip : odTrips)
    //{
    //  for (auto & accessFootpath : odTrip.accessFootpaths)
    //  {
    //    if (maxAccessTravelTime < accessFootpath.second)
    //    {
    //      maxAccessTravelTime = accessFootpath.second;
    //    }
    //  }
    //  for (auto & egressFootpath : odTrip.egressFootpaths)
    //  {
    //    if (maxAccessTravelTime < egressFootpath.second)
    //    {
    //      maxAccessTravelTime = egressFootpath.second;
    //    }
    //  }
    //}
    //std::cerr << "maxAccessTravelTime = " << maxAccessTravelTime << std::endl;

    
  }
  
}
