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
      
      // set cout number of decimals to 2 for displaying progress percentage:
      std::cout << std::fixed;
      std::cout << std::setprecision(2);
      
      for (pqxx::result::const_iterator c = pgResult.begin(); c != pgResult.end(); ++c) {
        
        // create a new stop for each row:
        Stop * stop   = new Stop();
        Point * point = new Point();
        // set stop attributes from row:
        stop->id                                  = c[0].as<unsigned long long>();
        stop->code                                = c[1].as<std::string>();
        stop->name                                = c[2].as<std::string>();
        stop->stationId                           = c[3].as<long long>();
        stop->point                               = *point;
        stop->point.latitude                      = c[4].as<double>();
        stop->point.longitude                     = c[5].as<double>();
        
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
      DataFetcher::saveToCacheFile(applicationShortname, stops, "stops");
      DataFetcher::saveToCacheFile(applicationShortname, stopIndexesById, "stop_indexes");
      
    } else {
      std::cerr << "Can't open database" << std::endl;
    }
    
    return std::make_pair(stops, stopIndexesById);
    
  }
  
  const std::pair<std::vector<Route>, std::map<unsigned long long, int>> DatabaseFetcher::getRoutes(std::string applicationShortname)
  {
    std::vector<Route> routes;
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
        Route * route = new Route();
        // set route attributes from row:
        route->id                                  = c[0].as<unsigned long long>();
        route->agencyId                            = c[1].as<unsigned long long>();
        route->routeTypeId                         = c[2].as<unsigned long long>();
        route->agencyAcronym                       = c[3].as<std::string>();
        route->agencyName                          = c[4].as<std::string>();
        route->shortname                           = c[5].as<std::string>();
        route->longname                            = c[6].as<std::string>();
        route->routeTypeName                       = c[7].as<std::string>();
        
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
      
      // save routes and stop indexes to binary cache file:
      DataFetcher::saveToCacheFile(applicationShortname, routes, "routes");
      DataFetcher::saveToCacheFile(applicationShortname, routeIndexesById, "route_indexes");
    
    } else {
      std::cout << "Can't open database" << std::endl;
    }
    
    return std::make_pair(routes, routeIndexesById);
    
  }
  
  const std::pair<std::vector<Trip>, std::map<unsigned long long, int>> DatabaseFetcher::getTrips(std::string applicationShortname)
  {
    std::vector<Trip> trips;
    std::map<unsigned long long, int> tripIndexesById;
    
    openConnection();
    
    std::cout << "Fetching trips from database..." << std::endl;
    std::string sqlQuery = "SELECT t.id, r.id, r.type_id, r.agency_id, t.service_id"
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
        Trip * trip = new Trip();
        // set trip attributes from row:
        trip->id          = c[0].as<unsigned long long>();
        trip->routeId     = c[1].as<unsigned long long>();
        trip->routeTypeId = c[2].as<unsigned long long>();
        trip->agencyId    = c[3].as<unsigned long long>();
        trip->serviceId   = c[4].as<unsigned long long>();
        
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
      
      // save trips and stop indexes to binary cache file:
      DataFetcher::saveToCacheFile(applicationShortname, trips, "trips");
      DataFetcher::saveToCacheFile(applicationShortname, tripIndexesById, "trip_indexes");
    
    } else {
      std::cout << "Can't open database" << std::endl;
    }
    
    return std::make_pair(trips, tripIndexesById);
    
  }
  
}
