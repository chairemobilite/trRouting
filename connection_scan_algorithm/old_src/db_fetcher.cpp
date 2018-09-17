#include "db_fetcher.hpp"

namespace TrRouting
{
  
  
  const std::map<unsigned long long,Connection> DbFetcher::getConnectionsById(std::string applicationShortname, std::string dataFetcher, std::string connectionsSqlWhereClause, Parameters& params)
  {
    
    else if(dataFetcher == "csv")
    {
      
      //CalculationTime::algorithmCalculationTime.startStep();
      std::cout << "Fetching connections from csv files..." << std::endl;
      
      std::string csvFilePath{applicationShortname + "_connections.csv"};
      std::ifstream csvFile{csvFilePath.c_str()};
      if (!csvFile.is_open()) 
      {
        std::cout << "Can't open csv file" << std::endl;
      }
      
      typedef boost::tokenizer< boost::escaped_list_separator<char > > Tokenizer;
      std::string csvLine;
      
      // set cout number of decimals to 2 for displaying progress percentage:
      std::cout << std::fixed;
      std::cout << std::setprecision(2);
      
      int i {0};
      
      std::getline(csvFile,csvLine); // ignore first line (header)
      
      while (std::getline(csvFile,csvLine))
      {
        Tokenizer tok(csvLine);
        
        // create a new Connection for each row:
        Connection * connection = new Connection();
        Tokenizer::iterator it = tok.begin();
        std::vector<std::shared_ptr<SimplifiedJourneyStep> > journeySteps;

        // set trConnection attributes from row:
        connection->id                                  = boost::lexical_cast<unsigned long long> (*it); std::advance(it,1);
        connection->serviceId                           = boost::lexical_cast<unsigned long long> (*it); std::advance(it,1);
        connection->agencyId                            = boost::lexical_cast<unsigned long long> (*it); std::advance(it,1);
        connection->routeId                             = boost::lexical_cast<unsigned long long> (*it); std::advance(it,1);
        connection->routeTypeId                         = boost::lexical_cast<unsigned long long> (*it); std::advance(it,1);
        connection->tripId                              = boost::lexical_cast<unsigned long long> (*it); std::advance(it,1);
        connection->sequence                            = boost::lexical_cast<unsigned long long> (*it); std::advance(it,1);
        connection->departureFromOriginTimeMinuteOfDay  = boost::lexical_cast<int>                (*it); std::advance(it,1);
        connection->arrivalAtDestinationTimeMinuteOfDay = boost::lexical_cast<int>                (*it); std::advance(it,1);
        connection->pathStopSequenceStartId             = boost::lexical_cast<unsigned long long> (*it); std::advance(it,1);
        connection->pathStopSequenceEndId               = boost::lexical_cast<unsigned long long> (*it); std::advance(it,1);
        connection->stopStartId                         = boost::lexical_cast<unsigned long long> (*it); std::advance(it,1);
        connection->stopEndId                           = boost::lexical_cast<unsigned long long> (*it); std::advance(it,1);
        connection->canBoard                            = boost::lexical_cast<unsigned long long> (*it); std::advance(it,1);
        connection->canUnboard                          = boost::lexical_cast<unsigned long long> (*it); std::advance(it,1);
        connection->reachable                           = 0;
        connection->nextConnectionId                    = boost::lexical_cast<long long>          (*it); std::advance(it,1);
        connection->previousConnectionId                = boost::lexical_cast<long long>          (*it); std::advance(it,1);
        connection->enabled                             = boost::lexical_cast<unsigned long long> (*it);
        connection->journeySteps                        = journeySteps;
        connection->lastJourneyStepIndex                = 0;
        
        // add the trConnection to the connections vector:
        connectionsById[connection->id] = *connection;
        
        // show loading progress:
        i++;
        if (i % 100 == 0)
        {
          std::cout << i << " connections\r"; // \r is used to stay on the same line
        }
        
      }
      std::cout << std::endl;
      
      csvFile.close();
      
      //CalculationTime::algorithmCalculationTime.stopStep();
      //std::cout << "-- Put connections data into a map --" << CalculationTime::algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
      
      // save connections to binary cache file:
      saveToCacheFile(applicationShortname, connectionsById, "connections");
      
    }
    
    return connectionsById;
    
  }
  
  const std::map<unsigned long long,Stop> DbFetcher::getStopsById(std::string applicationShortname, std::string dataFetcher, int maxTimeValue)
  {
    
    else if(dataFetcher == "csv")
    {
      
      //CalculationTime::algorithmCalculationTime.startStep();
      std::cout << "Fetching stops from csv files..." << std::endl;
      
      std::string csvFilePath{applicationShortname + "_stops.csv"};
      std::ifstream csvFile{csvFilePath.c_str()};
      if (!csvFile.is_open()) 
      {
        std::cout << "Can't open csv file" << std::endl;
      }
      
      typedef boost::tokenizer< boost::escaped_list_separator<char > > Tokenizer;
      std::string csvLine;
      
      // set cout number of decimals to 2 for displaying progress percentage:
      std::cout << std::fixed;
      std::cout << std::setprecision(2);
      
      int i {0};
      
      std::getline(csvFile,csvLine); // ignore first line
      
      while (std::getline(csvFile,csvLine))
      {
        Tokenizer tok(csvLine);
        
        // create a new Stop for each row:
        Stop * stop = new Stop();
        Point * point = new Point();
        Tokenizer::iterator it = tok.begin();
        std::vector<std::shared_ptr<SimplifiedJourneyStep> > journeySteps;
        
        // set trConnection attributes from row:
        stop->id              = boost::lexical_cast<unsigned long long> (*it); std::advance(it,1);
        stop->code            = boost::lexical_cast<std::string>        (*it); std::advance(it,1);
        stop->name            = boost::lexical_cast<std::string>        (*it); std::advance(it,1);
        stop->stationId       = boost::lexical_cast<long long>          (*it); std::advance(it,1);
        stop->point           = *point;
        stop->point.latitude  = boost::lexical_cast<double>             (*it); std::advance(it,1);
        stop->point.longitude = boost::lexical_cast<double>             (*it);
        stop->arrivalTimeMinuteOfDay  = maxTimeValue;
        stop->journeySteps            = journeySteps;
        stop->canUnboardToDestination = false;

        // add the Stop to the connections vector:
        stopsById[stop->id] = *stop;
        
        // show loading progress:
        i++;
        if (i % 100 == 0)
        {
          std::cout << i << " stops\r"; // \r is used to stay on the same line
        }
        
      }
      std::cout << std::endl;
      
      csvFile.close();
      
      //CalculationTime::algorithmCalculationTime.stopStep();
      //std::cout << "-- Put stops data into a map --" << CalculationTime::algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
      
      // save connections to binary cache file:
      saveToCacheFile(applicationShortname, stopsById, "stops");
       
    }
    
    return stopsById;
    
  }
  
  const std::map<unsigned long long,Route> DbFetcher::getRoutesById(std::string applicationShortname, std::string dataFetcher)
  {
    
    else if(dataFetcher == "csv")
    {
      
      //CalculationTime::algorithmCalculationTime.startStep();
      std::cout << "Fetching routes from csv files..." << std::endl;
      
      std::string csvFilePath{applicationShortname + "_routes.csv"};
      std::ifstream csvFile{csvFilePath.c_str()};
      if (!csvFile.is_open()) 
      {
        std::cout << "Can't open csv file" << std::endl;
      }
      
      typedef boost::tokenizer< boost::escaped_list_separator<char > > Tokenizer;
      std::string csvLine;
      
      // set cout number of decimals to 2 for displaying progress percentage:
      std::cout << std::fixed;
      std::cout << std::setprecision(2);
      
      int i {0};
      
      std::getline(csvFile,csvLine); // ignore first line
      
      while (std::getline(csvFile,csvLine))
      {
        Tokenizer tok(csvLine);
        
        // create a new Route for each row:
        Route * route = new Route();
        Tokenizer::iterator it = tok.begin();
        
        // set Route attributes from row:
        route->id                                  = boost::lexical_cast<unsigned long long> (*it); std::advance(it,1);
        route->agencyId                            = boost::lexical_cast<unsigned long long> (*it); std::advance(it,1);
        route->routeTypeId                         = boost::lexical_cast<unsigned long long> (*it); std::advance(it,1);
        route->agencyAcronym                       = boost::lexical_cast<std::string>        (*it); std::advance(it,1);
        route->agencyName                          = boost::lexical_cast<std::string>        (*it); std::advance(it,1);
        route->shortname                           = boost::lexical_cast<std::string>        (*it); std::advance(it,1);
        route->longname                            = boost::lexical_cast<std::string>        (*it); std::advance(it,1);
        route->routeTypeName                       = boost::lexical_cast<std::string>        (*it);
        
        // add the route to the map:
        routesById[route->id] = *route;
        
        // show loading progress:
        i++;
        if (i % 100 == 0)
        {
          std::cout << i << " routes\r"; // \r is used to stay on the same line
        }
        
      }
      std::cout << std::endl;
      
      csvFile.close();
      
      //CalculationTime::algorithmCalculationTime.stopStep();
      //std::cout << "-- Put routes data into a map --" << CalculationTime::algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
      
      // save connections to binary cache file:
      saveToCacheFile(applicationShortname, routesById, "routes");
       
    }
    
    return routesById;
    
  }
  
  const std::map<unsigned long long,std::map<unsigned long long, int> > DbFetcher::getTransferDurationsByStopId(std::string applicationShortname, std::string dataFetcher, int maxTransferWalkingTravelTimeMinutes, std::string transfersSqlWhereClause)
  {
    
    else if(dataFetcher == "csv")
    {
      
      //CalculationTime::algorithmCalculationTime.startStep();
      std::cout << "Fetching transfers from csv files..." << std::endl;
      
      std::string csvFilePath{applicationShortname + "_transfers.csv"};
      std::ifstream csvFile{csvFilePath.c_str()};
      if (!csvFile.is_open()) 
      {
        std::cout << "Can't open csv file" << std::endl;
      }
      
      typedef boost::tokenizer< boost::escaped_list_separator<char > > Tokenizer;
      std::string csvLine;
      
      // set cout number of decimals to 2 for displaying progress percentage:
      std::cout << std::fixed;
      std::cout << std::setprecision(2);
      
      int i {0};
      unsigned long long stopId1;
      unsigned long long stopId2;
      int walkingDurationMinutes;
      
      std::getline(csvFile,csvLine); // ignore first line
      
      while (std::getline(csvFile,csvLine))
      {
        Tokenizer tok(csvLine);
        
        Tokenizer::iterator it = tok.begin();
        
        stopId1                = boost::lexical_cast<unsigned long long> (*it); std::advance(it,1);
        stopId2                = boost::lexical_cast<unsigned long long> (*it); std::advance(it,1);
        walkingDurationMinutes = boost::lexical_cast<int>                (*it);
        transferDurationsByStopId[stopId1][stopId2] = walkingDurationMinutes;
        
        // show loading progress:
        i++;
        if (i % 100 == 0)
        {
          std::cout << i << " transfers\r"; // \r is used to stay on the same line
        }
        
      }
      std::cout << std::endl;
      
      csvFile.close();
      
      //CalculationTime::algorithmCalculationTime.stopStep();
      //std::cout << "-- Put transfers data into a map --" << CalculationTime::algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
      
      // save connections to binary cache file:
      saveToCacheFile(applicationShortname, transferDurationsByStopId, "transfers");
       
    }
    
    return transferDurationsByStopId;
    
  }
  
  const std::pair<int, int> DbFetcher::getTripTravelTimeAndDistance(Point startingPoint, Point endingPoint, std::string mode, Parameters& params)
  {
    //float speedMetersPerSecond;
    std::string routingPort;
    std::string routingHost;
    
    int travelTimeMinutes{-1};
    int distanceMeters{-1};
    
    if (mode ==  "walking")
    {
      //speedMetersPerSecond = params.walkingSpeedMetersPerSecond;
      routingPort          = params.osrmRoutingWalkingPort;
      routingHost          = params.osrmRoutingWalkingHost;
    }
    else if (mode ==  "driving")
    {
      //speedMetersPerSecond = params.drivingSpeedMetersPerSecond;
      routingPort          = params.osrmRoutingDrivingPort;
      routingHost          = params.osrmRoutingDrivingHost;
    }
    else if (mode == "cycling")
    {
      //speedMetersPerSecond = params.cyclingSpeedMetersPerSecond;
      routingPort          = params.osrmRoutingCyclingPort;
      routingHost          = params.osrmRoutingCyclingHost;
    }
    else
    {
      return std::make_pair(travelTimeMinutes, distanceMeters);
    }
    
    std::cout << "mode = " << mode << " port = " << routingPort << " host = " << routingHost << " " << std::endl;
    
    std::cout << std::fixed;
    std::cout << std::setprecision(6);
    
    std::string queryString = "GET /route/v1/" + mode + "/" + std::to_string(startingPoint.longitude) +  "," + std::to_string(startingPoint.latitude) + ";" + std::to_string(endingPoint.longitude) +  "," + std::to_string(endingPoint.latitude) + "?alternatives=false&steps=false&overview=full&geometries=geojson";
    
    boost::asio::ip::tcp::iostream s;
    s.connect(routingHost, routingPort);
    queryString += " HTTP/1.1\r\n\r\n";
        
    s << queryString;
    
    std::string header;
    while (std::getline(s, header) && header != "\r"){} // ignore first line
    
    std::stringstream responseJsonSs;
    responseJsonSs << s.rdbuf();
    
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(responseJsonSs, pt);
    
    using boost::property_tree::ptree;
    
    //std::cout << "duration count = " << pt.count("durations") << std::endl;
    
    if (pt.count("routes") >= 1)
    {
      ptree routes = pt.get_child("routes");
    
      int i = 0;
      
      for (const auto& v : routes) {
        for (const auto& v2 : v.second) {
          
          //std::cerr << "     v.second:" << v.second.get<float>("duration", 0) << std::endl;
          
          travelTimeMinutes = (int)ceil(v.second.get<float>("duration", 0)/60);
          distanceMeters    = (int)ceil(v.second.get<float>("distance", 0));
          break; // We take only the first route
          
        }
      }
    }
    
    return std::make_pair(travelTimeMinutes, distanceMeters);
    
  }
  
  const std::map<unsigned long long, int> DbFetcher::getNearestStopsIds(std::string applicationShortname, std::string dataFetcher, Point point, std::map<unsigned long long, Stop> stopsById, Parameters& params, std::string mode, int maxTravelTimeMinutes)
  {
        
    float speedMetersPerSecond;
    std::string routingPort;
    std::string routingHost;
    
    if (mode ==  "walking")
    {
      speedMetersPerSecond = params.walkingSpeedMetersPerSecond;
      routingPort          = params.osrmRoutingWalkingPort;
      routingHost          = params.osrmRoutingWalkingHost;
    }
    else if (mode ==  "driving")
    {
      speedMetersPerSecond = params.drivingSpeedMetersPerSecond;
      routingPort          = params.osrmRoutingDrivingPort;
      routingHost          = params.osrmRoutingDrivingHost;
    }
    else if (mode == "cycling")
    {
      speedMetersPerSecond = params.cyclingSpeedMetersPerSecond;
      routingPort          = params.osrmRoutingCyclingPort;
      routingHost          = params.osrmRoutingCyclingHost;
    }
    
    std::cout << "mode = " << mode << " speed = " << speedMetersPerSecond << " maxTravelTime = " << maxTravelTimeMinutes << " port = " << routingPort << " host = " << routingHost << " " << std::endl;
    
    std::cout << std::fixed;
    std::cout << std::setprecision(6);
    
    std::map<unsigned long long, int> nearestStopsIds;
    
    std::string queryString = "GET /table/v1/" + mode + "/" + std::to_string(point.longitude) +  "," + std::to_string(point.latitude);
    
    //if (dataFetcher == "database")
    //{
    //
    //  std::string sqlQuery = "SELECT tr_stops.id, CEIL((ST_DISTANCE(ST_GEOMFROMTEXT('" + point.asWKT() + "'), geography))/(" + std::to_string(speedMetersPerSecond) + "*60)) as ttm FROM " + applicationShortname + ".tr_stops WHERE CEIL((ST_DISTANCE(ST_GEOMFROMTEXT('" + point.asWKT() + "'), geography))/(" + std::to_string(speedMetersPerSecond) + "*60)) <= " + std::to_string(maxTravelTimeMinutes);
    //  
    //  if (isConnectionOpen())
    //  {
    //    pqxx::nontransaction pgNonTransaction(*(getConnectionPtr()));
    //    pqxx::result pgResult( pgNonTransaction.exec( sqlQuery ));
    //    unsigned long long resultCount = pgResult.size();
    //    unsigned long long i = 1;
    //    unsigned long long stopId;
    //    
    //    for (pqxx::result::const_iterator c = pgResult.begin(); c != pgResult.end(); ++c) {
    //      stopId                  = c[0].as<unsigned long long>();
    //      nearestStopsIds[stopId] = c[1].as<int>();
    //      queryString += ";" + std::to_string(stopsById[stopId].point.longitude) +  "," + std::to_string(stopsById[stopId].point.latitude);
    //    }
    //  
    //  } else {
    //    std::cout << "Can't open database" << std::endl;
    //  }
    //  
    //}
    //else
    //{
      
      // here we select stops within reasonable distance using degrees to meters conversion (simplified):
      // https://knowledge.safe.com/articles/725/calculating-accurate-length-in-meters-for-lat-long.html
      
      float lengthOfOneDegreeOfLongitude = 111412.84 * cos(point.latitude * M_PI / 180) -93.5 * cos (3 * point.latitude * M_PI / 180);
      float lengthOfOneDegreeOflatitude  = 111132.92 - 559.82 * cos(2 * point.latitude * M_PI / 180) + 1.175 * cos(4 * point.latitude * M_PI / 180);
      float maxDistanceMeters            = maxTravelTimeMinutes * 60 * speedMetersPerSecond;
      float distanceMeters;
      float distanceXMeters;
      float distanceYMeters;
      Point stopPoint;
      
      for (auto & stopId : stopsById)
      {
        stopPoint       = stopsById[stopId.first].point;
        distanceXMeters = (stopPoint.longitude - point.longitude) * lengthOfOneDegreeOfLongitude;
        distanceYMeters = (stopPoint.latitude  - point.latitude)  * lengthOfOneDegreeOflatitude ;
        distanceMeters  = sqrt(distanceXMeters * distanceXMeters + distanceYMeters * distanceYMeters);
        //std::cerr << distanceMeters;
        if (distanceMeters <= maxDistanceMeters)
        {
          nearestStopsIds[stopId.first] = (distanceMeters / speedMetersPerSecond) / 60; // in minutes
          queryString += ";" + std::to_string(stopPoint.longitude) +  "," + std::to_string(stopPoint.latitude);
        }
      }
      
    //}
    
        
    boost::asio::ip::tcp::iostream s;
    s.connect(routingHost, routingPort);
    queryString += "?sources=0";
    queryString += " HTTP/1.1\r\n\r\n";
        
    s << queryString;
    
    std::string header;
    while (std::getline(s, header) && header != "\r"){} // ignore first line
    
    std::stringstream responseJsonSs;
    responseJsonSs << s.rdbuf();
    
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(responseJsonSs, pt);
    
    using boost::property_tree::ptree;
    
    std::map<unsigned long long, int>::iterator iter = nearestStopsIds.begin();
    
    //std::cout << "duration count = " << pt.count("durations") << std::endl;
    
    if (pt.count("durations") == 1)
    {
      ptree durations = pt.get_child("durations");
    
      int travelTimeMinutes;
      int i = 0;
      
      for (const auto& v : durations) {
        for (const auto& v2 : v.second) {
          if(i > 0) // first value is duration with starting stop, we must ignore it
          {
            travelTimeMinutes = (int)ceil(std::stod(v2.second.data())/60);
            if (travelTimeMinutes < 9999)
            {
              iter->second = travelTimeMinutes;
            }
            if (iter->second > maxTravelTimeMinutes)
            {
              nearestStopsIds.erase(iter++); // erase value if walking duration is more than max (default value was using bird distance)
            }
            else
            {
              ++iter;
            }
          }
          else
          {
            i++;
          }
        }
      }
    }
    
    return nearestStopsIds;
    
  }
  
  const std::map<std::pair<unsigned long long, unsigned long long>, double> DbFetcher::getTravelTimeByStopsPair(std::string applicationShortname, std::string mode)
  {
    
    std::map<std::pair<unsigned long long, unsigned long long>, double> travelTimeByStopsPair;
    
    // if cache file is not empty, load travel times from cache:
    if (isCacheFileNotEmpty(applicationShortname, "travel_time_by_stops_pair"))
    {
      travelTimeByStopsPair = loadFromCacheFile(travelTimeByStopsPair, applicationShortname, "travel_time_by_stops_pair");;
      //std::cout << "-- Input from travel_time_by_stops_pair cache file --" << CalculationTime::algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
      return travelTimeByStopsPair;
    }
    
    //CalculationTime::algorithmCalculationTime.startStep();
    std::cout << "Fetching travel time from database..." << std::endl;
    std::cout << "using mode: " << mode << std::endl;
    std::string sqlQuery = "SELECT "
      "stop_1_id as s1, "
      "stop_2_id as s2, "
      "network_" + mode + "_duration_seconds as travel_time "
      "FROM " + applicationShortname + ".tr_matrix_stop_distances "
      "LEFT JOIN " + applicationShortname + ".tr_stops stop_1 ON stop_1.id = stop_1_id "
      "LEFT JOIN " + applicationShortname + ".tr_stops stop_2 ON stop_2.id = stop_2_id "
      "WHERE stop_1.is_genetic_algorithm_stop IS TRUE AND stop_2.is_genetic_algorithm_stop IS TRUE "
      "ORDER BY stop_1_id, network_" + mode + "_duration_seconds";
      
    if (isConnectionOpen())
    {
      
      //CalculationTime::algorithmCalculationTime.startStep();
      
      pqxx::nontransaction pgNonTransaction(*(getConnectionPtr()));
      pqxx::result pgResult( pgNonTransaction.exec( sqlQuery ));
      unsigned long long resultCount = pgResult.size();
      unsigned long long i = 1;
      
      std::cout << std::fixed;
      std::cout << std::setprecision(2);
      
      //CalculationTime::algorithmCalculationTime.stopStep();
      //std::cout << "-- Fetching travel time data from database --" << CalculationTime::algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
      //CalculationTime::algorithmCalculationTime.startStep();
      
      std::pair<unsigned long long, unsigned long long> stopPair;
      
      for (pqxx::result::const_iterator c = pgResult.begin(); c != pgResult.end(); ++c) {
        stopPair                        = std::make_pair(c[0].as<unsigned long long>(), c[1].as<unsigned long long>());
        travelTimeByStopsPair[stopPair] = c[2].as<double>();
        i++;
        
        if (i % 1000 == 0)
        {
          std::cout << ((((double) i) / resultCount) * 100) << "%\r";
        }
      }
      std::cout << std::endl;
      
      //CalculationTime::algorithmCalculationTime.stopStep();
      //std::cout << "-- Put travel time data into a map --" << CalculationTime::algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
      
      saveToCacheFile(applicationShortname, travelTimeByStopsPair, "travel_time_by_stops_pair");
      

    } else {
      std::cout << "Can't open database" << std::endl;
    }
    
    return travelTimeByStopsPair;
    
  }
  
}

