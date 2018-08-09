#include "cache_fetcher.hpp"

namespace TrRouting
{
  
  const std::pair<std::vector<Stop>, std::map<unsigned long long, int>> CacheFetcher::getStops(std::string applicationShortname)
  {
    std::vector<Stop> stops;
    ProtoStops protoStops;
    std::map<unsigned long long, int> stopIndexesById;
    
    std::cout << "Fetching stops from cache..." << std::endl;
    if (CacheFetcher::protobufCacheFileExists(applicationShortname, "stops"))
    {
      protoStops = loadFromProtobufCacheFile(protoStops, applicationShortname, "stops");
      for (int i = 0; i < protoStops.stops_size(); i++)
      {
        const ProtoStop&  protoStop  = protoStops.stops(i);
        const ProtoPoint& protoPoint = protoStop.point();

        Stop  * stop          = new Stop();
        Point * point         = new Point();
        stop->id              = protoStop.id();
        stop->code            = protoStop.code();
        stop->name            = protoStop.name();
        stop->stationId       = protoStop.station_id();
        stop->point           = *point;
        stop->point.latitude  = protoPoint.latitude();
        stop->point.longitude = protoPoint.longitude();

        stops.push_back(*stop);
        stopIndexesById[stop->id] = stops.size() - 1;
      }
    }
    else
    {
      std::cerr << "missing stops cache file!" << std::endl;
    }
    return std::make_pair(stops, stopIndexesById);
  }
  
  const std::pair<std::vector<Route>, std::map<unsigned long long, int>> CacheFetcher::getRoutes(std::string applicationShortname)
  {
    std::vector<Route> routes;
    std::map<unsigned long long, int> routeIndexesById;
    
    std::cout << "Fetching routes from cache..." << std::endl;
    if (CacheFetcher::cacheFileExists(applicationShortname, "routes"))
    {
      routes = loadFromCacheFile(routes, applicationShortname, "routes");
    }
    else
    {
      std::cerr << "missing routes cache file!" << std::endl;
    }
    if (CacheFetcher::cacheFileExists(applicationShortname, "route_indexes"))
    {
      routeIndexesById = loadFromCacheFile(routeIndexesById, applicationShortname, "route_indexes");
    }
    else
    {
      std::cerr << "missing route_indexes cache file!" << std::endl;
    }
    return std::make_pair(routes, routeIndexesById);
  }
  
  const std::pair<std::vector<Trip>, std::map<unsigned long long, int>> CacheFetcher::getTrips(std::string applicationShortname)
  {
    std::vector<Trip> trips;
    std::map<unsigned long long, int> tripIndexesById;
    
    std::cout << "Fetching trips from cache..." << std::endl;
    if (CacheFetcher::cacheFileExists(applicationShortname, "trips"))
    {
      trips = loadFromCacheFile(trips, applicationShortname, "trips");
    }
    else
    {
      std::cerr << "missing trips cache file!" << std::endl;
    }
    if (CacheFetcher::cacheFileExists(applicationShortname, "trip_indexes"))
    {
      tripIndexesById = loadFromCacheFile(tripIndexesById, applicationShortname, "trip_indexes");
    }
    else
    {
      std::cerr << "missing trip_indexes cache file!" << std::endl;
    }
    return std::make_pair(trips, tripIndexesById);
  }
  
  const std::pair<std::vector<std::tuple<int,int,int,int,int,short,short,int>>, std::vector<std::tuple<int,int,int,int,int,short,short,int>>> CacheFetcher::getConnections(std::string applicationShortname, std::map<unsigned long long, int> stopIndexesById, std::map<unsigned long long, int> tripIndexesById)
  {
    std::vector<std::tuple<int,int,int,int,int,short,short,int>> forwardConnections;
    std::vector<std::tuple<int,int,int,int,int,short,short,int>> reverseConnections;
    
    std::cout << "Fetching connections from cache..." << std::endl;
    if (CacheFetcher::cacheFileExists(applicationShortname, "connections_forward"))
    {
      forwardConnections = loadFromCacheFile(forwardConnections, applicationShortname, "connections_forward");
    }
    else
    {
      std::cerr << "missing connections_forward cache file!" << std::endl;
    }
    if (CacheFetcher::cacheFileExists(applicationShortname, "connections_forward"))
    {
      reverseConnections = loadFromCacheFile(reverseConnections, applicationShortname, "connections_reverse");
    }
    else
    {
      std::cerr << "missing connections_reverse cache file!" << std::endl;
    }
    return std::make_pair(forwardConnections, reverseConnections);
    
  }
  
  const std::pair<std::vector<std::tuple<int,int,int>>, std::vector<std::pair<int,int>>> CacheFetcher::getFootpaths(std::string applicationShortname, std::map<unsigned long long, int> stopIndexesById)
  {
    std::vector<std::tuple<int,int,int>> footpaths;
    std::vector<std::pair<int,int>>      footpathsRanges;
    
    std::cout << "Fetching footpaths from cache..." << std::endl;
    if (CacheFetcher::cacheFileExists(applicationShortname, "footpaths"))
    {
      footpaths = loadFromCacheFile(footpaths, applicationShortname, "footpaths");
    }
    else
    {
      std::cerr << "missing footpaths cache file!" << std::endl;
    }
    if (CacheFetcher::cacheFileExists(applicationShortname, "footpaths_ranges"))
    {
      footpathsRanges = loadFromCacheFile(footpathsRanges, applicationShortname, "footpaths_ranges");
    }
    else
    {
      std::cerr << "missing footpaths_ranges cache file!" << std::endl;
    }
    return std::make_pair(footpaths, footpathsRanges);
    
  }

  const std::pair<std::vector<OdTrip>, std::map<unsigned long long, int>> CacheFetcher::getOdTrips(std::string applicationShortname, std::vector<Stop> stops, Parameters& params)
  {
    std::vector<OdTrip> odTrips;
    std::map<unsigned long long, int> odTripIndexesById;
    
    std::cout << "Fetching od trips from cache..." << std::endl;
    if (CacheFetcher::cacheFileExists(applicationShortname, "od_trips"))
    {
      odTrips = loadFromCacheFile(odTrips, applicationShortname, "od_trips");
    }
    else
    {
      std::cerr << "missing od trips cache file!" << std::endl;
    }
    if (CacheFetcher::cacheFileExists(applicationShortname, "od_trip_indexes"))
    {
      odTripIndexesById = loadFromCacheFile(odTripIndexesById, applicationShortname, "od_trip_indexes");
    }
    else
    {
      std::cerr << "missing od trip indexes cache file!" << std::endl;
    }
    return std::make_pair(odTrips, odTripIndexesById);
    
  }
  
  
}
