#include "database_fetcher.hpp"

namespace TrRouting
{
  
  void DatabaseFetcher::disconnect()
  {
    if (pgConnectionPtr)
    {
      pgConnectionPtr->disconnect();
      delete pgConnectionPtr;
      pgConnectionPtr = NULL;
    }
  }
  
  bool DatabaseFetcher::isConnectionOpen()
  {
    return (*getConnectionPtr()).is_open();
  }
  
  pqxx::connection* DatabaseFetcher::getConnectionPtr()
  {
    openConnection();
    return pgConnectionPtr;
  }
  
  void DatabaseFetcher::openConnection()
  {
    if (pgConnectionPtr == NULL)
    {
      pgConnectionPtr = new pqxx::connection(customDbSetupStr);
    }
  }
  
  const std::pair<std::vector<Stop>, std::map<unsigned long long, int>> DatabaseFetcher::getStops(std::string applicationShortname)
  {
    std::vector<Stop> stops;
    //ProtoStops protoStops;

    ::capnp::MallocMessageBuilder capnpStopsCollectionMessage;
    stopsCollection::StopsCollection::Builder capnpStopsCollection = capnpStopsCollectionMessage.initRoot<stopsCollection::StopsCollection>();
    std::map<unsigned long long, int> stopIndexesById;
    
    openConnection();
    
    std::cout << "Fetching stops from database..." << std::endl;
    std::string sqlQuery = "SELECT s.id, COALESCE(s.code,'?'), COALESCE(s.name,'?'), COALESCE(s.station_id, -1), ST_Y(s.geography::geometry), ST_X(s.geography::geometry)"
    " FROM " + applicationShortname + ".tr_stops s "
    " ORDER BY s.id";
    
    std::cerr << sqlQuery << std::endl;
    
    if (isConnectionOpen())
    {
      
      pqxx::nontransaction pgNonTransaction(*getConnectionPtr());
      pqxx::result pgResult( pgNonTransaction.exec( sqlQuery ));
      unsigned long long resultCount = pgResult.size();
      unsigned long long i = 0;
      
      ::capnp::List<stopsCollection::Stop>::Builder capnpStops = capnpStopsCollection.initStops(resultCount);

      // set cout number of decimals to 2 for displaying progress percentage:
      std::cout << std::fixed;
      std::cout << std::setprecision(2);
      
      for (pqxx::result::const_iterator c = pgResult.begin(); c != pgResult.end(); ++c) {
        
        // create a new stop for each row:
        Stop * stop           = new Stop();
        //ProtoStop * protoStop = protoStops.add_stops();
        stopsCollection::Stop::Builder capnpStop = capnpStops[i];
        Point * point         = new Point();
        // set stop attributes from row:
        stop->id                                  = c[0].as<unsigned long long>();
        stop->code                                = c[1].as<std::string>();
        stop->name                                = c[2].as<std::string>();
        stop->stationId                           = c[3].as<long long>();
        stop->point                               = *point;
        stop->point.latitude                      = c[4].as<double>();
        stop->point.longitude                     = c[5].as<double>();
        
        capnpStop.setId(stop->id);
        capnpStop.setCode(stop->code);
        capnpStop.setName(stop->name);
        capnpStop.setStationId(stop->stationId);
        capnpStop.setLatitude(stop->point.latitude);
        capnpStop.setLongitude(stop->point.longitude);

        //protoStop->set_id(stop->id);
        //protoStop->set_code(stop->code);
        //protoStop->set_name(stop->name);
        //protoStop->set_station_id(stop->stationId);
        //protoStop->set_latitude(stop->point.latitude);
        //protoStop->set_longitude(stop->point.longitude);

        // append stop:
        stops.push_back(*stop);
        stopIndexesById[stop->id] = stops.size() - 1;
        
        // show loading progress in percentage:
        i++;
        if (i % 1000 == 0)
        {
          std::cout << ((((double) i) / resultCount) * 100) << "%\r"; // \r is used to stay on the same line
        }
      }
      std::cout << std::endl;
      
      // save stops and stop indexes to binary cache file:
      std::cout << "Saving stops to cache..." << std::endl;
      //CacheFetcher::saveToProtobufCacheFile(applicationShortname, protoStops, "stops");
      CacheFetcher::saveToCapnpCacheFile(applicationShortname, capnpStopsCollectionMessage, "stops");
      
    } else {
      std::cerr << "Can't open database" << std::endl;
    }
    
    return std::make_pair(stops, stopIndexesById);
    
  }
  
  const std::pair<std::vector<Route>, std::map<unsigned long long, int>> DatabaseFetcher::getRoutes(std::string applicationShortname)
  {
    std::vector<Route> routes;
    ProtoRoutes protoRoutes;
    std::map<unsigned long long, int> routeIndexesById;
    
    openConnection();
    
    std::cout << "Fetching routes from database..." << std::endl;
    std::string sqlQuery = "SELECT r.id, r.agency_id, r.type_id, a.acronym, a.name, COALESCE(r.shortname,'?') as route_shortname, COALESCE(r.longname,'?') as route_longname, rt.name"
    " FROM " + applicationShortname + ".tr_routes r "
    " LEFT JOIN " + applicationShortname + ".tr_route_types rt ON rt.id = r.type_id "
    " LEFT JOIN " + applicationShortname + ".tr_agencies a ON a.id = r.agency_id"
    " ORDER BY r.id";
    
    std::cerr << sqlQuery << std::endl;
    
    if (isConnectionOpen())
    {
      
      pqxx::nontransaction pgNonTransaction(*(getConnectionPtr()));
      pqxx::result pgResult( pgNonTransaction.exec( sqlQuery ));
      unsigned long long resultCount = pgResult.size();
      unsigned long long i = 0;
      
      // set cout number of decimals to 2 for displaying progress percentage:
      std::cout << std::fixed;
      std::cout << std::setprecision(2);
      
      for (pqxx::result::const_iterator c = pgResult.begin(); c != pgResult.end(); ++c) {
        
        // create a new route for each row:
        Route      * route      = new Route();
        ProtoRoute * protoRoute = protoRoutes.add_routes();

        // set route attributes from row:
        route->id                                  = c[0].as<unsigned long long>();
        route->agencyId                            = c[1].as<unsigned long long>();
        route->routeTypeId                         = c[2].as<unsigned long long>();
        route->agencyAcronym                       = c[3].as<std::string>();
        route->agencyName                          = c[4].as<std::string>();
        route->shortname                           = c[5].as<std::string>();
        route->longname                            = c[6].as<std::string>();
        route->routeTypeName                       = c[7].as<std::string>();
        
        protoRoute->set_id(route->id);
        protoRoute->set_agency_id(route->agencyId);
        protoRoute->set_route_type_id(route->routeTypeId);
        protoRoute->set_agency_acronym(route->agencyAcronym);
        protoRoute->set_agency_name(route->agencyName);
        protoRoute->set_shortname(route->shortname);
        protoRoute->set_longname(route->longname);
        protoRoute->set_route_type_name(route->routeTypeName);

        // append route:
        routes.push_back(*route);
        routeIndexesById[route->id] = routes.size() - 1;
        
        // show loading progress in percentage:
        i++;
        if (i % 1000 == 0)
        {
          std::cout << ((((double) i) / resultCount) * 100) << "%\r"; // \r is used to stay on the same line
        }
      }
      std::cout << std::endl;
      
      // save routes and route indexes to binary cache file:
      std::cout << "Saving routes to cache..." << std::endl;
      CacheFetcher::saveToProtobufCacheFile(applicationShortname, protoRoutes, "routes");
    
    } else {
      std::cout << "Can't open database" << std::endl;
    }
    
    return std::make_pair(routes, routeIndexesById);
    
  }
  
  const std::pair<std::vector<Trip>, std::map<unsigned long long, int>> DatabaseFetcher::getTrips(std::string applicationShortname)
  {
    std::vector<Trip> trips;
    ProtoTrips protoTrips;
    std::map<unsigned long long, int> tripIndexesById;

    openConnection();
    
    std::cout << "Fetching trips from database..." << std::endl;
    std::string sqlQuery = "SELECT t.id, r.id, t.path_id, r.type_id, r.agency_id, t.service_id"
    " FROM " + applicationShortname + ".tr_trips t "
    " LEFT JOIN " + applicationShortname + ".tr_route_paths rp ON rp.id = t.path_id "
    " LEFT JOIN " + applicationShortname + ".tr_routes r ON r.id = rp.route_id"
    " ORDER BY t.id";
    
    std::cerr << sqlQuery << std::endl;
    
    if (isConnectionOpen())
    {
      
      pqxx::nontransaction pgNonTransaction(*(getConnectionPtr()));
      pqxx::result pgResult( pgNonTransaction.exec( sqlQuery ));
      unsigned long long resultCount = pgResult.size();
      unsigned long long i = 0;
      
      // set cout number of decimals to 2 for displaying progress percentage:
      std::cout << std::fixed;
      std::cout << std::setprecision(2);
      
      for (pqxx::result::const_iterator c = pgResult.begin(); c != pgResult.end(); ++c) {
        
        // create a new trip for each row:
        Trip      * trip      = new Trip();
        ProtoTrip * protoTrip = protoTrips.add_trips();

        // set trip attributes from row:
        trip->id              = c[0].as<unsigned long long>();
        trip->routeId         = c[1].as<unsigned long long>();
        trip->routePathId     = c[2].as<unsigned long long>();
        trip->routeTypeId     = c[3].as<unsigned long long>();
        trip->agencyId        = c[4].as<unsigned long long>();
        trip->serviceId       = c[5].as<unsigned long long>();

        protoTrip->set_id(trip->id);
        protoTrip->set_route_id(trip->routeId);
        protoTrip->set_route_path_id(trip->routePathId);
        protoTrip->set_route_type_id(trip->routeTypeId);
        protoTrip->set_agency_id(trip->agencyId);
        protoTrip->set_service_id(trip->serviceId);

        // append trip:
        trips.push_back(*trip);
        tripIndexesById[trip->id] = trips.size() - 1;
        
        // show loading progress in percentage:
        i++;
        if (i % 1000 == 0)
        {
          std::cout << ((((double) i) / resultCount) * 100) << "%\r"; // \r is used to stay on the same line
        }
      }
      std::cout << std::endl;
      
      // save trips and trip indexes to binary cache file:
      std::cout << "Saving trips to cache..." << std::endl;
      CacheFetcher::saveToProtobufCacheFile(applicationShortname, protoTrips, "trips");
    
    } else {
      std::cout << "Can't open database" << std::endl;
    }
    
    return std::make_pair(trips, tripIndexesById);
    
  }
  
  
  const std::pair<std::vector<std::tuple<int,int,int,int,int,short,short,int>>, std::vector<std::tuple<int,int,int,int,int,short,short,int>>> DatabaseFetcher::getConnections(std::string applicationShortname, std::map<unsigned long long, int> stopIndexesById, std::map<unsigned long long, int> tripIndexesById)
  {
    std::vector<std::tuple<int,int,int,int,int,short,short,int>> forwardConnections;
    std::vector<std::tuple<int,int,int,int,int,short,short,int>> reverseConnections;
    ProtoConnections protoConnections;
    enum connectionIndexes : short { STOP_DEP = 0, STOP_ARR = 1, TIME_DEP = 2, TIME_ARR = 3, TRIP = 4, CAN_BOARD = 5, CAN_UNBOARD = 6, SEQUENCE = 7 };

    openConnection();
    
    std::cout << "Fetching connections from database..." << std::endl;
    
    // query for connections:
    std::string sqlQuery = "SELECT trip_id, COALESCE(can_board,1), COALESCE(can_unboard,1), departure_time_seconds, arrival_time_seconds, stop_departure_id, stop_arrival_id, sequence FROM " + applicationShortname + ".mv_tr_connections_v2 WHERE COALESCE(stop_arrival_enabled, TRUE) IS TRUE AND COALESCE(stop_departure_enabled, TRUE) IS TRUE ORDER BY departure_time_seconds, trip_id, sequence";
    
    std::cout << sqlQuery << std::endl;
    
    if (isConnectionOpen())
    {
      
      //CalculationTime::algorithmCalculationTime.startStep();
      
      pqxx::nontransaction pgNonTransaction(*(getConnectionPtr()));
      pqxx::result pgResult( pgNonTransaction.exec( sqlQuery ));
      unsigned long long resultCount = pgResult.size();
      unsigned long long i = 0;
      
      // set cout number of decimals to 2 for displaying progress percentage:
      std::cout << std::fixed;
      std::cout << std::setprecision(2);

      for (pqxx::result::const_iterator c = pgResult.begin(); c != pgResult.end(); ++c) {
        
        forwardConnections.push_back(std::make_tuple(stopIndexesById[c[5].as<unsigned long long>()], stopIndexesById[c[6].as<unsigned long long>()], c[3].as<int>(), c[4].as<int>(), tripIndexesById[c[0].as<unsigned long long>()], c[1].as<short>(), c[2].as<short>(), c[7].as<int>())); // departureStopIndex, arrivalStopIndex, departureTimeSeconds, arrivalTimeSeconds, tripIndex, canBoard, canUnboard, sequence in trip
        //reverseConnections.push_back(std::make_tuple(stopIndexesById[c[6].as<unsigned long long>()], stopIndexesById[c[5].as<unsigned long long>()], MAX_INT - c[4].as<int>(), MAX_INT - c[3].as<int>(), tripIndexesById[c[0].as<unsigned long long>()], c[2].as<short>(), c[1].as<short>(), c[7].as<int>())); // departureStopIndex, arrivalStopIndex, departureTimeSeconds, arrivalTimeSeconds, tripIndex, canBoard, canUnboard, sequence in trip
        
        // show loading progress in percentage:
        i++;
        if (i % 1000 == 0)
        {
          std::cerr << ((((double) i) / resultCount) * 100) << "%      \r"; // \r is used to stay on the same line
        }
      }
      std::cerr << std::endl;
      
      std::cout << "Sorting reverse connections..." << std::endl;
      //std::reverse(reverseConnections.begin(), reverseConnections.end());
      reverseConnections = forwardConnections;
      std::stable_sort(reverseConnections.begin(), reverseConnections.end(), [](std::tuple<int,int,int,int,int,short,short,int> connectionA, std::tuple<int,int,int,int,int,short,short,int> connectionB)
      {
        // { STOP_DEP = 0, STOP_ARR = 1, TIME_DEP = 2, TIME_ARR = 3, TRIP = 4, CAN_BOARD = 5, CAN_UNBOARD = 6, SEQUENCE = 7 };
        if (std::get<3>(connectionA) > std::get<3>(connectionB))
        {
          return true;
        }
        else if (std::get<3>(connectionA) < std::get<3>(connectionB))
        {
          return false;
        }
        if (std::get<4>(connectionA) > std::get<4>(connectionB)) // here we need to reverse sequence!
        {
          return true;
        }
        else if (std::get<4>(connectionA) < std::get<4>(connectionB))
        {
          return false;
        }
        if (std::get<7>(connectionA) > std::get<7>(connectionB)) // here we need to reverse sequence!
        {
          return true;
        }
        else if (std::get<7>(connectionA) < std::get<7>(connectionB))
        {
          return false;
        }
        return false;
      });

      for(auto & connection : forwardConnections)
      {
        ProtoConnection * protoConnection = protoConnections.add_forward_connections();
        protoConnection->set_stop_dep_idx(std::get<connectionIndexes::STOP_DEP>   (connection));
        protoConnection->set_stop_arr_idx(std::get<connectionIndexes::STOP_ARR>   (connection));
        protoConnection->set_time_dep    (std::get<connectionIndexes::TIME_DEP>   (connection));
        protoConnection->set_time_arr    (std::get<connectionIndexes::TIME_ARR>   (connection));
        protoConnection->set_trip_idx    (std::get<connectionIndexes::TRIP>       (connection));
        protoConnection->set_can_board   (std::get<connectionIndexes::CAN_BOARD>  (connection));
        protoConnection->set_can_unboard (std::get<connectionIndexes::CAN_UNBOARD>(connection));
        protoConnection->set_sequence    (std::get<connectionIndexes::SEQUENCE>   (connection));
      }

      for(auto & connection : reverseConnections)
      {
        ProtoConnection * protoConnection = protoConnections.add_reverse_connections();
        protoConnection->set_stop_dep_idx(std::get<connectionIndexes::STOP_DEP>   (connection));
        protoConnection->set_stop_arr_idx(std::get<connectionIndexes::STOP_ARR>   (connection));
        protoConnection->set_time_dep    (std::get<connectionIndexes::TIME_DEP>   (connection));
        protoConnection->set_time_arr    (std::get<connectionIndexes::TIME_ARR>   (connection));
        protoConnection->set_trip_idx    (std::get<connectionIndexes::TRIP>       (connection));
        protoConnection->set_can_board   (std::get<connectionIndexes::CAN_BOARD>  (connection));
        protoConnection->set_can_unboard (std::get<connectionIndexes::CAN_UNBOARD>(connection));
        protoConnection->set_sequence    (std::get<connectionIndexes::SEQUENCE>   (connection));
      }

      // save connections to binary cache file:
      std::cout << "Saving connections to cache..." << std::endl;
      CacheFetcher::saveToProtobufCacheFile(applicationShortname, protoConnections, "connections");
      
    } else {
      std::cout << "Can't open database" << std::endl;
    }
    
    return std::make_pair(forwardConnections, reverseConnections);
    
  }
  
  const std::pair<std::vector<std::tuple<int,int,int>>, std::vector<std::pair<int,int>>> DatabaseFetcher::getFootpaths(std::string applicationShortname, std::map<unsigned long long, int> stopIndexesById)
  {
    std::vector<std::tuple<int,int,int>> footpaths;
    ProtoFootpaths protoFootpaths;
    std::vector<std::pair<int,int>>      footpathsRanges(stopIndexesById.size());
    
    for (auto & stop : stopIndexesById)
    {
      footpathsRanges[stop.second] = std::make_pair(-1, -1);
    }
    
    openConnection();
    
    std::cout << "Fetching transfer walking durations from database..." << std::endl;
    std::cout << "max transfer time seconds: 1200" << std::endl;
    std::string sqlQuery = "SELECT "
      "stop_1_id as s1, "
      "stop_2_id as s2, "
      "MAX(CEIL(COALESCE(network_walking_duration_seconds::float, network_distance::float/1.38, distance::float/1.38))) as tts "
      "FROM " + applicationShortname + ".tr_matrix_stop_distances "
      "WHERE CEIL(COALESCE(network_walking_duration_seconds::float, network_distance::float/1.38, distance::float/1.38)) <= 1200 "
      "GROUP BY stop_1_id, stop_2_id "
      "ORDER BY stop_1_id, MAX(CEIL(COALESCE(network_walking_duration_seconds::float, network_distance::float/1.38, distance::float/1.38))) ";
    
    std::cout << sqlQuery << std::endl;
    
    if (isConnectionOpen())
    {
      
      pqxx::nontransaction pgNonTransaction(*(getConnectionPtr()));
      pqxx::result pgResult( pgNonTransaction.exec( sqlQuery ));
      unsigned long long resultCount = pgResult.size();
      unsigned long long i = 1;
      
      std::cout << std::fixed;
      std::cout << std::setprecision(2);
      int stop1Index              = -1;
      int stop2Index              = -1;
      int travelTimeSeconds       = -1;
      int footpathIndex           = -1;
      ProtoFootpathRange * protoFootpathRange;
      
      for (pqxx::result::const_iterator c = pgResult.begin(); c != pgResult.end(); ++c) {
        
        stop1Index                    = stopIndexesById[c[0].as<unsigned long long>()];
        stop2Index                    = stopIndexesById[c[1].as<unsigned long long>()];
        travelTimeSeconds             = c[2].as<int>();
        ProtoFootpath * protoFootpath = protoFootpaths.add_footpaths();

        protoFootpath->set_stop_1_idx(stop1Index);
        protoFootpath->set_stop_2_idx(stop2Index);
        protoFootpath->set_travel_time(travelTimeSeconds);

        footpaths.push_back(std::make_tuple(stop1Index, stop2Index, travelTimeSeconds));
        footpathIndex = footpaths.size() - 1;
                
        if (footpathsRanges[stop1Index].first == -1)
        {
          footpathsRanges[stop1Index].first = footpathIndex;
        }
        footpathsRanges[stop1Index].second = footpathIndex;
        
        if (i % 1000 == 0)
        {
          std::cout << ((((double) i) / resultCount) * 100) << "%\r";
        }
        
        i++;
      }
      std::cout << std::endl;
      
      for(auto & footpathRange : footpathsRanges)
      {
        ProtoFootpathRange * protoFootpathRange = protoFootpaths.add_footpath_ranges();
        protoFootpathRange->set_footpaths_start_idx(std::get<0>(footpathRange));
        protoFootpathRange->set_footpaths_end_idx(std::get<1>(footpathRange));
      }
      
      std::cout << "Saving footpaths to cache..." << std::endl;
      CacheFetcher::saveToProtobufCacheFile(applicationShortname, protoFootpaths, "footpaths");
      
    } else {
      std::cout << "Can't open database" << std::endl;
    }
    
    return std::make_pair(footpaths, footpathsRanges);
  }
  
  const std::pair<std::vector<OdTrip>, std::map<unsigned long long, int>> DatabaseFetcher::getOdTrips(std::string applicationShortname, std::vector<Stop> stops, Parameters& params)
  {
    
    std::vector<OdTrip> odTrips;
    ProtoOdTrips protoOdTrips;
    std::map<unsigned long long, int> odTripIndexesById;
    std::map<std::string,std::vector<std::pair<int,int>>> footpathsByPoint;
    
    // fetch existing so we can append:
    if (!params.updateOdTrips)
    {
      std::tie(odTrips, odTripIndexesById) = params.cacheFetcher->getOdTrips(params.applicationShortname, stops, params);
    }
    
    openConnection();
    
    std::cout << "Fetching od trips from database..." << std::endl;
    
    // query for connections:
    std::string sqlQuery = "SELECT id, user_interview_id, household_interview_id, COALESCE(age,-1), origin_lat, origin_lon, destination_lat, destination_lon, COALESCE(age_group_sn, 'unknown'), COALESCE(occupation_sn, 'unknown'), COALESCE(activity_sn, 'unknown'),  COALESCE(gender_sn, 'unknown'), COALESCE(mode_sn, 'unknown'), start_at_seconds, COALESCE(expansion_factor,1), COALESCE(walking_travel_time_seconds,-1), COALESCE(cycling_travel_time_seconds,-1), COALESCE(driving_travel_time_seconds,-1), ST_Y(home_geography::geometry), ST_X(home_geography::geometry), '-', COALESCE(end_at_seconds, -1) FROM " + applicationShortname + ".tr_od_trips WHERE origin_geography IS NOT NULL AND destination_geography IS NOT NULL AND start_at_seconds >= 0 AND user_interview_id iS NOT NULL AND household_interview_id IS NOT NULL ORDER BY id";
    
    std::cout << sqlQuery << std::endl;
    
    if (isConnectionOpen())
    {
      
      pqxx::nontransaction pgNonTransaction(*(getConnectionPtr()));
      pqxx::result pgResult( pgNonTransaction.exec( sqlQuery ));
      unsigned long long resultCount = pgResult.size();
      unsigned long long i = 0;
      
      // set cout number of decimals to 2 for displaying progress percentage:
      std::cout << std::fixed;
      std::cout << std::setprecision(2);
      
      unsigned long long odTripId;
      for (pqxx::result::const_iterator c = pgResult.begin(); c != pgResult.end(); ++c) {
        
        // set trip attributes from row:
        odTripId = c[0].as<unsigned long long>();
        if (odTripIndexesById.find(odTripId) != odTripIndexesById.end()) // ignore if already saved
        {
          i++;
          continue;
        }
        
        // create a new trip for each row:
        OdTrip * odTrip       = new OdTrip();
        ProtoOdTrip * protoOdTrip = protoOdTrips.add_od_trips();
        Point  * origin       = new Point();
        Point  * destination  = new Point();
        Point  * homeLocation = new Point();

        odTrip->id                       = odTripId;
        odTrip->origin                   = *origin;
        odTrip->destination              = *destination;
        odTrip->homeLocation             = *homeLocation;
        odTrip->personId                 = c[1 ].as<unsigned long long>();
        odTrip->householdId              = c[2 ].as<unsigned long long>();
        odTrip->age                      = c[3 ].as<int>();
        odTrip->origin.latitude          = c[4 ].as<double>();
        odTrip->origin.longitude         = c[5 ].as<double>();
        odTrip->destination.latitude     = c[6 ].as<double>();
        odTrip->destination.longitude    = c[7 ].as<double>();
        odTrip->homeLocation.latitude    = c[18].as<double>();
        odTrip->homeLocation.longitude   = c[19].as<double>();
        odTrip->ageGroup                 = c[8 ].as<std::string>();
        odTrip->occupation               = c[9 ].as<std::string>();
        odTrip->originActivity           = c[20].as<std::string>();
        odTrip->destinationActivity      = c[10].as<std::string>();
        odTrip->gender                   = c[11].as<std::string>();
        odTrip->mode                     = c[12].as<std::string>();
        odTrip->departureTimeSeconds     = c[13].as<int>();
        odTrip->arrivalTimeSeconds       = c[21].as<int>();
        odTrip->expansionFactor          = c[14].as<float>();
        odTrip->walkingTravelTimeSeconds = c[15].as<int>();
        odTrip->cyclingTravelTimeSeconds = c[16].as<int>();
        odTrip->drivingTravelTimeSeconds = c[17].as<int>();
        
        protoOdTrip->set_id(odTripId);
        protoOdTrip->set_origin_latitude(odTrip->origin.latitude);
        protoOdTrip->set_origin_longitude(odTrip->origin.longitude);
        protoOdTrip->set_destination_latitude(odTrip->destination.latitude);
        protoOdTrip->set_destination_longitude(odTrip->destination.longitude);
        protoOdTrip->set_home_latitude(odTrip->homeLocation.latitude);
        protoOdTrip->set_home_longitude(odTrip->homeLocation.longitude);
        protoOdTrip->set_person_id(odTrip->personId);
        protoOdTrip->set_household_id(odTrip->householdId);
        protoOdTrip->set_age(odTrip->age);
        protoOdTrip->set_age_group(odTrip->ageGroup);
        protoOdTrip->set_occupation(odTrip->occupation);
        protoOdTrip->set_activity_origin(odTrip->originActivity);
        protoOdTrip->set_activity_destination(odTrip->destinationActivity);
        protoOdTrip->set_gender(odTrip->gender);
        protoOdTrip->set_mode(odTrip->mode);
        protoOdTrip->set_departure_time_seconds(odTrip->departureTimeSeconds);
        protoOdTrip->set_arrival_time_seconds(odTrip->arrivalTimeSeconds);
        protoOdTrip->set_expansion_factor(odTrip->expansionFactor);
        protoOdTrip->set_walking_travel_time_seconds(odTrip->walkingTravelTimeSeconds);
        protoOdTrip->set_cycling_travel_time_seconds(odTrip->cyclingTravelTimeSeconds);
        protoOdTrip->set_driving_travel_time_seconds(odTrip->drivingTravelTimeSeconds);

        std::string originPointStr     {std::to_string(round(odTrip->origin.latitude * 100000.0)/100000.0)      + ',' + std::to_string(round(odTrip->origin.longitude * 100000.0)/100000.0)     };
        std::string destinationPointStr{std::to_string(round(odTrip->destination.latitude * 100000.0)/100000.0) + ',' + std::to_string(round(odTrip->destination.longitude * 100000.0)/100000.0)};

        if (footpathsByPoint.find(originPointStr) == footpathsByPoint.end())
        {
          footpathsByPoint[originPointStr] = OsrmFetcher::getAccessibleStopsFootpathsFromPoint(odTrip->origin, stops, "walking", params);
        }
        if (footpathsByPoint.find(destinationPointStr) == footpathsByPoint.end())
        {
          footpathsByPoint[destinationPointStr] = OsrmFetcher::getAccessibleStopsFootpathsFromPoint(odTrip->destination, stops, "walking", params);
        }
        odTrip->accessFootpaths = footpathsByPoint[originPointStr];
        odTrip->egressFootpaths = footpathsByPoint[destinationPointStr];

        for (auto & accessFootpath : odTrip->accessFootpaths)
        {
          ProtoOdTripFootpath * protoOdTripFootpath = protoOdTrip->add_access_footpaths();
          protoOdTripFootpath->set_stop_idx(accessFootpath.first);
          protoOdTripFootpath->set_travel_time(accessFootpath.second);
        }

        for (auto & egressFootpath : odTrip->egressFootpaths)
        {
          ProtoOdTripFootpath * protoOdTripFootpath = protoOdTrip->add_egress_footpaths();
          protoOdTripFootpath->set_stop_idx(egressFootpath.first);
          protoOdTripFootpath->set_travel_time(egressFootpath.second);
        }

        // append od trip:
        odTrips.push_back(*odTrip);
        odTripIndexesById[odTrip->id] = odTrips.size() - 1;

        i++;
        if (i % 10 == 0)
        {
          std::cerr << ((((double) i) / resultCount) * 100) << "% (" << i << "/" << resultCount << ")             \r"; // \r is used to stay on the same line
        }
        if (i % 500 == 0)
        {
          CacheFetcher::saveToProtobufCacheFile(applicationShortname, protoOdTrips, "od_trips");
        }
      }

      std::cerr << std::endl;

      // save od trips and od trip indexes to binary cache file:
      std::cout << "Saving od trips to cache..." << std::endl;
      CacheFetcher::saveToProtobufCacheFile(applicationShortname, protoOdTrips, "od_trips");
    
    } else {
      std::cerr << "Can't open database" << std::endl;
    }
    
    return std::make_pair(odTrips, odTripIndexesById);
    
  }
  
}
