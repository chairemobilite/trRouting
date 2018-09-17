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

        Stop  * stop          = new Stop();
        Point * point         = new Point();
        stop->id              = protoStop.id();
        stop->code            = protoStop.code();
        stop->name            = protoStop.name();
        stop->stationId       = protoStop.station_id();
        stop->point           = *point;
        stop->point.latitude  = protoStop.latitude();
        stop->point.longitude = protoStop.longitude();

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
    ProtoRoutes protoRoutes;
    std::map<unsigned long long, int> routeIndexesById;
    
    std::cout << "Fetching routes from cache..." << std::endl;
    if (CacheFetcher::protobufCacheFileExists(applicationShortname, "routes"))
    {
      protoRoutes = loadFromProtobufCacheFile(protoRoutes, applicationShortname, "routes");
      for (int i = 0; i < protoRoutes.routes_size(); i++)
      {
        const ProtoRoute& protoRoute = protoRoutes.routes(i);

        Route  * route          = new Route();
        route->id               = protoRoute.id();
        route->agencyId         = protoRoute.agency_id();
        route->routeTypeId      = protoRoute.route_type_id();
        route->agencyAcronym    = protoRoute.agency_acronym();
        route->agencyName       = protoRoute.agency_name();
        route->shortname        = protoRoute.shortname();
        route->longname         = protoRoute.longname();
        route->routeTypeName    = protoRoute.route_type_name();
       
        routes.push_back(*route);
        routeIndexesById[route->id] = routes.size() - 1;
      }
    }
    else
    {
      std::cerr << "missing routes cache file!" << std::endl;
    }
    return std::make_pair(routes, routeIndexesById);
  }
  
  const std::pair<std::vector<Trip>, std::map<unsigned long long, int>> CacheFetcher::getTrips(std::string applicationShortname)
  {
    std::vector<Trip> trips;
    ProtoTrips protoTrips;
    std::map<unsigned long long, int> tripIndexesById;
    
    if (CacheFetcher::protobufCacheFileExists(applicationShortname, "trips"))
    {
      protoTrips = loadFromProtobufCacheFile(protoTrips, applicationShortname, "trips");
      for (int i = 0; i < protoTrips.trips_size(); i++)
      {
        const ProtoTrip& protoTrip = protoTrips.trips(i);

        Trip * trip       = new Trip();
        trip->id          = protoTrip.id();
        trip->routeId     = protoTrip.route_id();
        trip->routePathId = protoTrip.route_path_id();
        trip->routeTypeId = protoTrip.route_type_id();
        trip->agencyId    = protoTrip.agency_id();
        trip->serviceId   = protoTrip.service_id();

        trips.push_back(*trip);
        tripIndexesById[trip->id] = trips.size() - 1;
      }
    }
    else
    {
      std::cerr << "missing trips cache file!" << std::endl;
    }
    return std::make_pair(trips, tripIndexesById);
  }
  
  const std::pair<std::vector<std::tuple<int,int,int,int,int,short,short,int>>, std::vector<std::tuple<int,int,int,int,int,short,short,int>>> CacheFetcher::getConnections(std::string applicationShortname, std::map<unsigned long long, int> stopIndexesById, std::map<unsigned long long, int> tripIndexesById)
  {
    std::vector<std::tuple<int,int,int,int,int,short,short,int>> forwardConnections;
    std::vector<std::tuple<int,int,int,int,int,short,short,int>> reverseConnections; 
    ProtoConnections protoConnections;
    
    std::cout << "Fetching connections from cache..." << std::endl;
    if (CacheFetcher::protobufCacheFileExists(applicationShortname, "connections"))
    {
      protoConnections = loadFromProtobufCacheFile(protoConnections, applicationShortname, "connections");

      for (int i = 0; i < protoConnections.forward_connections_size(); i++)
      {
        const ProtoConnection& protoConnection = protoConnections.forward_connections(i);
        forwardConnections.push_back(std::make_tuple(
          protoConnection.stop_dep_idx(),
          protoConnection.stop_arr_idx(),
          protoConnection.time_dep(),
          protoConnection.time_arr(),
          protoConnection.trip_idx(),
          protoConnection.can_board(),
          protoConnection.can_unboard(),
          protoConnection.sequence()
        ));
      }

      for (int i = 0; i < protoConnections.reverse_connections_size(); i++)
      {
        const ProtoConnection& protoConnection = protoConnections.reverse_connections(i);
        reverseConnections.push_back(std::make_tuple(
          protoConnection.stop_dep_idx(),
          protoConnection.stop_arr_idx(),
          protoConnection.time_dep(),
          protoConnection.time_arr(),
          protoConnection.trip_idx(),
          protoConnection.can_board(),
          protoConnection.can_unboard(),
          protoConnection.sequence()
        ));
      }

    }
    else
    {
      std::cerr << "missing connections cache file!" << std::endl;
    }
    return std::make_pair(forwardConnections, reverseConnections);
    
  }
  
  const std::pair<std::vector<std::tuple<int,int,int>>, std::vector<std::pair<int,int>>> CacheFetcher::getFootpaths(std::string applicationShortname, std::map<unsigned long long, int> stopIndexesById)
  {
    std::vector<std::tuple<int,int,int>> footpaths;
    ProtoFootpaths protoFootpaths;
    std::vector<std::pair<int,int>>      footpathsRanges;
    
    std::cout << "Fetching footpaths from cache..." << std::endl;
    if (CacheFetcher::protobufCacheFileExists(applicationShortname, "footpaths"))
    {
      protoFootpaths = loadFromProtobufCacheFile(protoFootpaths, applicationShortname, "footpaths");
      for (int i = 0; i < protoFootpaths.footpaths_size(); i++)
      {
        const ProtoFootpath& protoFootpath = protoFootpaths.footpaths(i);

        footpaths.push_back(std::make_tuple(
          protoFootpath.stop_1_idx(),
          protoFootpath.stop_2_idx(),
          protoFootpath.travel_time()
        ));
      
      }

      for (int i = 0; i < protoFootpaths.footpath_ranges_size(); i++)
      {
        const ProtoFootpathRange& protoFootpathRange = protoFootpaths.footpath_ranges(i);

        footpathsRanges.push_back(std::make_pair(
          protoFootpathRange.footpaths_start_idx(),
          protoFootpathRange.footpaths_end_idx()
        ));

      }
    }
    else
    {
      std::cerr << "missing trips cache file!" << std::endl;
    }
    return std::make_pair(footpaths, footpathsRanges);
    
  }

  const std::pair<std::vector<OdTrip>, std::map<unsigned long long, int>> CacheFetcher::getOdTrips(std::string applicationShortname, std::vector<Stop> stops, Parameters& params)
  {
    std::vector<OdTrip> odTrips;
    ProtoOdTrips protoOdTrips;
    std::map<unsigned long long, int> odTripIndexesById;
    
    std::cout << "Fetching od trips from cache..." << std::endl;
    if (CacheFetcher::protobufCacheFileExists(applicationShortname, "footpaths"))
    {
      protoOdTrips = loadFromProtobufCacheFile(protoOdTrips, applicationShortname, "od_trips");
      for (int i = 0; i < protoOdTrips.od_trips_size(); i++)
      {
        const ProtoOdTrip& protoOdTrip      = protoOdTrips.od_trips(i);

        OdTrip * odTrip       = new OdTrip();
        Point  * origin       = new Point();
        Point  * destination  = new Point();
        Point  * homeLocation = new Point();
        std::vector<std::pair<int,int>> accessFootpaths;
        std::vector<std::pair<int,int>> egressFootpaths;

        odTrip->id                       = protoOdTrip.id();
        odTrip->origin                   = *origin;
        odTrip->destination              = *destination;
        odTrip->homeLocation             = *homeLocation;
        odTrip->personId                 = protoOdTrip.person_id();
        odTrip->householdId              = protoOdTrip.household_id();
        odTrip->age                      = protoOdTrip.age();
        odTrip->origin.latitude          = protoOdTrip.origin_latitude();
        odTrip->origin.longitude         = protoOdTrip.origin_longitude();
        odTrip->destination.latitude     = protoOdTrip.destination_latitude();
        odTrip->destination.longitude    = protoOdTrip.destination_longitude();
        odTrip->homeLocation.latitude    = protoOdTrip.home_latitude();
        odTrip->homeLocation.longitude   = protoOdTrip.home_longitude();
        odTrip->ageGroup                 = protoOdTrip.age_group();
        odTrip->occupation               = protoOdTrip.occupation();
        odTrip->originActivity           = protoOdTrip.activity_origin();
        odTrip->destinationActivity      = protoOdTrip.activity_destination();
        odTrip->gender                   = protoOdTrip.gender();
        odTrip->mode                     = protoOdTrip.mode();
        odTrip->departureTimeSeconds     = protoOdTrip.departure_time_seconds();
        odTrip->arrivalTimeSeconds       = protoOdTrip.arrival_time_seconds();
        odTrip->expansionFactor          = protoOdTrip.expansion_factor();
        odTrip->walkingTravelTimeSeconds = protoOdTrip.walking_travel_time_seconds();
        odTrip->cyclingTravelTimeSeconds = protoOdTrip.cycling_travel_time_seconds();
        odTrip->drivingTravelTimeSeconds = protoOdTrip.driving_travel_time_seconds();

        for (int j = 0; j < protoOdTrip.access_footpaths_size(); j++)
        {
          const ProtoOdTripFootpath& protoOdTripFootpath = protoOdTrip.access_footpaths(j);
          accessFootpaths.push_back(std::make_pair(
            protoOdTripFootpath.stop_idx(),
            protoOdTripFootpath.travel_time()
          ));
        }
        odTrip->accessFootpaths = accessFootpaths;

        for (int j = 0; j < protoOdTrip.egress_footpaths_size(); j++)
        {
          const ProtoOdTripFootpath& protoOdTripFootpath = protoOdTrip.egress_footpaths(j);
          egressFootpaths.push_back(std::make_pair(
            protoOdTripFootpath.stop_idx(),
            protoOdTripFootpath.travel_time()
          ));
        }
        odTrip->egressFootpaths = accessFootpaths;

        odTrips.push_back(*odTrip);
        odTripIndexesById[odTrip->id] = odTrips.size() - 1;
      }

    }

    else
    {
      std::cerr << "missing od trip indexes cache file!" << std::endl;
    }
    return std::make_pair(odTrips, odTripIndexesById);
    
  }
  
  
}
