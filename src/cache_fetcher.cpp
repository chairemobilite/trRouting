#include "cache_fetcher.hpp"

namespace TrRouting
{
  
  const std::pair<std::vector<Stop>, std::map<unsigned long long, int>> CacheFetcher::getStops(std::string applicationShortname)
  {
    std::vector<Stop> stops;
    std::string cacheFileName{"stops"};
    std::map<unsigned long long, int> stopIndexesById;
    
    std::cout << "Fetching stops from cache..." << std::endl;
    if (CacheFetcher::capnpCacheFileExists(applicationShortname, cacheFileName))
    {
      int fd = open((applicationShortname + "_" + cacheFileName + ".capnpbin").c_str(), O_RDWR);
      ::capnp::PackedFdMessageReader capnpStopsCollectionMessage(fd, {64 * 1024 * 1024});
      stopsCollection::StopsCollection::Reader capnpStopsCollection = capnpStopsCollectionMessage.getRoot<stopsCollection::StopsCollection>();
      for (stopsCollection::Stop::Reader capnpStop : capnpStopsCollection.getStops())
      {

        Stop  * stop          = new Stop();
        Point * point         = new Point();
        stop->id              = capnpStop.getId();
        stop->code            = capnpStop.getCode();
        stop->name            = capnpStop.getName();
        stop->stationId       = capnpStop.getStationId();
        stop->point           = *point;
        stop->point.latitude  = capnpStop.getLatitude();
        stop->point.longitude = capnpStop.getLongitude();

        stops.push_back(*stop);
        stopIndexesById[stop->id] = stops.size() - 1;
      }
      close(fd);
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
    std::string cacheFileName{"routes"};
    std::map<unsigned long long, int> routeIndexesById;
    
    std::cout << "Fetching routes from cache..." << std::endl;
    if (CacheFetcher::capnpCacheFileExists(applicationShortname, cacheFileName))
    {
      int fd = open((applicationShortname + "_" + cacheFileName + ".capnpbin").c_str(), O_RDWR);
      ::capnp::PackedFdMessageReader capnpRoutesCollectionMessage(fd, {64 * 1024 * 1024});
      routesCollection::RoutesCollection::Reader capnpRoutesCollection = capnpRoutesCollectionMessage.getRoot<routesCollection::RoutesCollection>();
      for (routesCollection::Route::Reader capnpRoute : capnpRoutesCollection.getRoutes())
      {
        Route * route           = new Route();
        route->id               = capnpRoute.getId();
        route->agencyId         = capnpRoute.getAgencyId();
        route->routeTypeId      = capnpRoute.getRouteTypeId();
        route->agencyAcronym    = capnpRoute.getAgencyAcronym();
        route->agencyName       = capnpRoute.getAgencyName();
        route->shortname        = capnpRoute.getShortname();
        route->longname         = capnpRoute.getLongname();
        route->routeTypeName    = capnpRoute.getRouteTypeName();
       
        routes.push_back(*route);
        routeIndexesById[route->id] = routes.size() - 1;
      }
      close(fd);
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
    std::string cacheFileName{"trips"};
    std::map<unsigned long long, int> tripIndexesById;
    
    if (CacheFetcher::capnpCacheFileExists(applicationShortname, cacheFileName))
    {
      int fd = open((applicationShortname + "_" + cacheFileName + ".capnpbin").c_str(), O_RDWR);
      ::capnp::PackedFdMessageReader capnpTripsCollectionMessage(fd, {64 * 1024 * 1024});
      tripsCollection::TripsCollection::Reader capnpTripsCollection = capnpTripsCollectionMessage.getRoot<tripsCollection::TripsCollection>();
      for (tripsCollection::Trip::Reader capnpTrip : capnpTripsCollection.getTrips())
      {
        Trip * trip       = new Trip();
        trip->id          = capnpTrip.getId();
        trip->routeId     = capnpTrip.getRouteId();
        trip->routePathId = capnpTrip.getRoutePathId();
        trip->routeTypeId = capnpTrip.getRouteTypeId();
        trip->agencyId    = capnpTrip.getAgencyId();
        trip->serviceId   = capnpTrip.getServiceId();

        trips.push_back(*trip);
        tripIndexesById[trip->id] = trips.size() - 1;
      }
      close(fd);
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
    std::string cacheFileName{"connections"};
    
    std::cout << "Fetching connections from cache..." << std::endl;
    if (CacheFetcher::capnpCacheFileExists(applicationShortname, cacheFileName))
    {
      int fd = open((applicationShortname + "_" + cacheFileName + ".capnpbin").c_str(), O_RDWR);
      ::capnp::PackedFdMessageReader capnpConnectionsCollectionMessage(fd, {64 * 1024 * 1024});
      connectionsCollection::ConnectionsCollection::Reader capnpConnectionsCollection = capnpConnectionsCollectionMessage.getRoot<connectionsCollection::ConnectionsCollection>();
      for (connectionsCollection::Connection::Reader capnpConnection : capnpConnectionsCollection.getForwardConnections())
      {
        forwardConnections.push_back(std::make_tuple(
          capnpConnection.getStopDepIdx(),
          capnpConnection.getStopArrIdx(),
          capnpConnection.getTimeDep(),
          capnpConnection.getTimeArr(),
          capnpConnection.getTripIdx(),
          capnpConnection.getCanBoard(),
          capnpConnection.getCanUnboard(),
          capnpConnection.getSequence()
        ));
      }

      for (connectionsCollection::Connection::Reader capnpConnection : capnpConnectionsCollection.getReverseConnections())
      {
        reverseConnections.push_back(std::make_tuple(
          capnpConnection.getStopDepIdx(),
          capnpConnection.getStopArrIdx(),
          capnpConnection.getTimeDep(),
          capnpConnection.getTimeArr(),
          capnpConnection.getTripIdx(),
          capnpConnection.getCanBoard(),
          capnpConnection.getCanUnboard(),
          capnpConnection.getSequence()
        ));
      }

      close(fd);

    }
    else
    {
      std::cerr << "missing connections cache file!" << std::endl;
    }
    return std::make_pair(forwardConnections, reverseConnections);
    
  }
  
  const std::pair<std::vector<std::tuple<int,int,int>>, std::vector<std::pair<long long,long long>>> CacheFetcher::getFootpaths(std::string applicationShortname, std::map<unsigned long long, int> stopIndexesById)
  {
    std::vector<std::tuple<int,int,int>> footpaths;
    std::string cacheFileName{"footpaths"};
    std::vector<std::pair<long long,long long>>      footpathsRanges;
    
    std::cout << "Fetching footpaths from cache..." << std::endl;
    if (CacheFetcher::capnpCacheFileExists(applicationShortname, cacheFileName))
    {
      int fd = open((applicationShortname + "_" + cacheFileName + ".capnpbin").c_str(), O_RDWR);
      ::capnp::PackedFdMessageReader capnpFootpathsCollectionMessage(fd, {64 * 1024 * 1024});
      footpathsCollection::FootpathsCollection::Reader capnpFootpathsCollection = capnpFootpathsCollectionMessage.getRoot<footpathsCollection::FootpathsCollection>();
      for (footpathsCollection::Footpath::Reader capnpFootpath : capnpFootpathsCollection.getFootpaths())
      {
        footpaths.push_back(std::make_tuple(
          capnpFootpath.getStop1Idx(),
          capnpFootpath.getStop2Idx(),
          capnpFootpath.getTravelTime()
        ));
      }
      for (footpathsCollection::FootpathRange::Reader capnpFootpathRange : capnpFootpathsCollection.getFootpathRanges())
      {
        footpathsRanges.push_back(std::make_pair(
          capnpFootpathRange.getFootpathsStartIdx(),
          capnpFootpathRange.getFootpathsEndIdx()
        ));
      }
      close(fd);
    }
    else
    {
      std::cerr << "missing footpaths cache file!" << std::endl;
    }
    return std::make_pair(footpaths, footpathsRanges);
    
  }

  const std::vector<std::pair<int,int>> CacheFetcher::getOdTripFootpaths(std::string applicationShortname, Parameters& params)
  {
    std::vector<std::pair<int,int>> odTripFootpaths;
    std::string cacheFileName{"od_trip_footpaths"};
    
    std::cout << "Fetching od trip footpaths from cache..." << std::endl;
    if (CacheFetcher::capnpCacheFileExists(applicationShortname, cacheFileName))
    {
      int fd = open((applicationShortname + "_" + cacheFileName + ".capnpbin").c_str(), O_RDWR);
      ::capnp::PackedFdMessageReader capnpOdTripFootpathsCollectionMessage(fd, {64 * 1024 * 1024});
      odTripFootpathsCollection::OdTripFootpathsCollection::Reader capnpOdTripFootpathsCollection = capnpOdTripFootpathsCollectionMessage.getRoot<odTripFootpathsCollection::OdTripFootpathsCollection>();

      for (odTripFootpathsCollection::OdTripFootpath::Reader capnpOdTripFoopath : capnpOdTripFootpathsCollection.getOdTripFootpaths())
      {
        odTripFootpaths.push_back(std::make_pair(
          capnpOdTripFoopath.getStopIdx(),
          capnpOdTripFoopath.getTravelTime()
        ));
      }
      close(fd);
    }

    else
    {
      std::cerr << "missing od trip cache file!" << std::endl;
    }
    return odTripFootpaths;
    
  }

  const std::pair<std::vector<OdTrip>, std::map<unsigned long long, int>> CacheFetcher::getOdTrips(std::string applicationShortname, std::vector<Stop> stops, Parameters& params)
  {
    std::vector<OdTrip> odTrips;
    std::string cacheFileName{"od_trips"};
    std::map<unsigned long long, int> odTripIndexesById;
    
    std::cout << "Fetching od trips from cache..." << std::endl;
    if (CacheFetcher::capnpCacheFileExists(applicationShortname, cacheFileName))
    {
      int fd = open((applicationShortname + "_" + cacheFileName + ".capnpbin").c_str(), O_RDWR);
      ::capnp::PackedFdMessageReader capnpOdTripsCollectionMessage(fd, {64 * 1024 * 1024});
      odTripsCollection::OdTripsCollection::Reader capnpOdTripsCollection = capnpOdTripsCollectionMessage.getRoot<odTripsCollection::OdTripsCollection>();

      for (odTripsCollection::OdTrip::Reader capnpOdTrip : capnpOdTripsCollection.getOdTrips())
      {
        OdTrip * odTrip       = new OdTrip();
        Point  * origin       = new Point();
        Point  * destination  = new Point();
        Point  * homeLocation = new Point();
        //std::vector<std::pair<int,int>> accessFootpaths;
        //std::vector<std::pair<int,int>> egressFootpaths;

        odTrip->id                        = capnpOdTrip.getId();
        odTrip->origin                    = *origin;
        odTrip->destination               = *destination;
        odTrip->homeLocation              = *homeLocation;
        odTrip->personId                  = capnpOdTrip.getPersonId();
        odTrip->householdId               = capnpOdTrip.getHouseholdId();
        odTrip->age                       = capnpOdTrip.getAge();
        odTrip->origin.latitude           = capnpOdTrip.getOriginLatitude();
        odTrip->origin.longitude          = capnpOdTrip.getOriginLongitude();
        odTrip->destination.latitude      = capnpOdTrip.getDestinationLatitude();
        odTrip->destination.longitude     = capnpOdTrip.getDestinationLongitude();
        odTrip->homeLocation.latitude     = capnpOdTrip.getHomeLatitude();
        odTrip->homeLocation.longitude    = capnpOdTrip.getHomeLongitude();
        odTrip->ageGroup                  = capnpOdTrip.getAgeGroup();
        odTrip->occupation                = capnpOdTrip.getOccupation();
        odTrip->originActivity            = capnpOdTrip.getOriginActivity();
        odTrip->destinationActivity       = capnpOdTrip.getDestinationActivity();
        odTrip->gender                    = capnpOdTrip.getGender();
        odTrip->mode                      = capnpOdTrip.getMode();
        odTrip->departureTimeSeconds      = capnpOdTrip.getDepartureTimeSeconds();
        odTrip->arrivalTimeSeconds        = capnpOdTrip.getArrivalTimeSeconds();
        odTrip->expansionFactor           = capnpOdTrip.getExpansionFactor();
        odTrip->walkingTravelTimeSeconds  = capnpOdTrip.getWalkingTravelTimeSeconds();
        odTrip->cyclingTravelTimeSeconds  = capnpOdTrip.getCyclingTravelTimeSeconds();
        odTrip->drivingTravelTimeSeconds  = capnpOdTrip.getDrivingTravelTimeSeconds();
        odTrip->accessFootpathsStartIndex = capnpOdTrip.getAccessFootpathsStartIdx();
        odTrip->accessFootpathsEndIndex   = capnpOdTrip.getAccessFootpathsEndIdx();
        odTrip->egressFootpathsStartIndex = capnpOdTrip.getEgressFootpathsStartIdx();
        odTrip->egressFootpathsEndIndex   = capnpOdTrip.getEgressFootpathsEndIdx();

        odTrips.push_back(*odTrip);
        odTripIndexesById[odTrip->id] = odTrips.size() - 1;
      }
      close(fd);
    }

    else
    {
      std::cerr << "missing od trip cache file!" << std::endl;
    }
    return std::make_pair(odTrips, odTripIndexesById);
    
  }
  
  
}
