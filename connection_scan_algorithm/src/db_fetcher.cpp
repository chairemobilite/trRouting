#include "db_fetcher.hpp"

namespace TrRouting
{
  
  pqxx::connection* DbFetcher::pgConnectionPtr = NULL;
  std::string DbFetcher::dbSetupStr = "";
  
  void DbFetcher::setDbSetupStr(std::string customDbSetupStr)
  {
    
    dbSetupStr = customDbSetupStr;
    
  }
  
  pqxx::connection* DbFetcher::getConnectionPtr()
  {
    
    if (pgConnectionPtr != NULL)
    {
      return pgConnectionPtr;
    }
    else
    {
      pgConnectionPtr = new pqxx::connection(dbSetupStr);
      return pgConnectionPtr;
    }
    
  }
  
  void DbFetcher::disconnect()
  {
    
    if (pgConnectionPtr)
    {
      pgConnectionPtr->disconnect();
      delete pgConnectionPtr;
      pgConnectionPtr = NULL;
    }
    
  }
  
  bool DbFetcher::isConnectionOpen()
  {
    
    return (*getConnectionPtr()).is_open();
    
  }
  
  const std::map<std::string,int> DbFetcher::getPickUpTypes(std::string applicationShortname)
  {
    
    std::map<std::string,int> pickUpTypes;
    //if (dataFetcher == "database")
    //{
    //  
    //  std::string sqlQuery = "SELECT id, shortname FROM " + applicationShortname + ".tr_pickup_types";
    //  
    //  if (isConnectionOpen())
    //  {
    //    pqxx::nontransaction pgNonTransaction(*(getConnectionPtr()));
    //    pqxx::result pgResult( pgNonTransaction.exec( sqlQuery ));
    //    for (pqxx::result::const_iterator c = pgResult.begin(); c != pgResult.end(); ++c) {
    //      pickUpTypes[c[1].as<std::string>()] = c[0].as<int>();
    //    }
    //  } else {
    //    std::cout << "Can't open database" << std::endl;
    //  }
    //  
    //}
    //else if(dataFetcher == "csv")
    //{
      
      pickUpTypes["regular"]                     = 0;
      pickUpTypes["no_pickup"]                   = 1;
      pickUpTypes["must_phone"]                  = 2;
      pickUpTypes["must_coordinate_with_driver"] = 3;
      
    //}
    
    return pickUpTypes;
    
  }
  
  const std::map<std::string,int> DbFetcher::getDropOffTypes(std::string applicationShortname)
  {
    
    std::map<std::string,int> dropOffTypes;
    
    //if (dataFetcher == "database")
    //{
    //  
    //  std::string sqlQuery = "SELECT id, shortname FROM " + applicationShortname + ".tr_drop_off_types";
    //
    //  if (isConnectionOpen())
    //  {
    //    pqxx::nontransaction pgNonTransaction(*(getConnectionPtr()));
    //    pqxx::result pgResult( pgNonTransaction.exec( sqlQuery ));
    //    for (pqxx::result::const_iterator c = pgResult.begin(); c != pgResult.end(); ++c) {
    //      dropOffTypes[c[1].as<std::string>()] = c[0].as<int>();
    //    }
    //  } else {
    //    std::cout << "Can't open database" << std::endl;
    //  }
    //  
    //}
    //else if(dataFetcher == "csv")
    //{
      
      dropOffTypes["regular"]                     = 0;
      dropOffTypes["no_drop_off"]                 = 1;
      dropOffTypes["must_phone"]                  = 2;
      dropOffTypes["must_coordinate_with_driver"] = 3;
      
    //}
    
    return dropOffTypes;
    
  }
  
  template<class T>
  void DbFetcher::saveToCacheFile(std::string applicationShortname, T& data, std::string cacheFileName)
  {
    CalculationTime::algorithmCalculationTime.startStep();
    
    std::ofstream oCacheFile;
    
    oCacheFile.open(applicationShortname + "_" + cacheFileName + ".cache", std::ios::out | std::ios::trunc | std::ios::binary);
    boost::archive::binary_oarchive oarch(oCacheFile);
    
    oarch << data;
    oCacheFile.close();
    
    CalculationTime::algorithmCalculationTime.stopStep();
    std::cout << "-- Saved cached data to file " << cacheFileName << " -- " << CalculationTime::algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
  
  }
  
  bool DbFetcher::isCacheFileNotEmpty(std::string applicationShortname, std::string cacheFileName)
  {
    std::ifstream iCacheFile;
    bool notEmpty = false;
    
    iCacheFile.open(applicationShortname + "_" + cacheFileName + ".cache", std::ios::in | std::ios::binary | std::ios::ate);

    notEmpty = iCacheFile.tellg() > 0;
    iCacheFile.close();
    
    return notEmpty;
  
  }
  
  template<class T>
  const T DbFetcher::loadFromCacheFile(T& data, std::string applicationShortname, std::string cacheFileName)
  {
    CalculationTime::algorithmCalculationTime.startStep();
    
    std::ifstream iCacheFile;
    
    iCacheFile.open(applicationShortname + "_" + cacheFileName + ".cache", std::ios::in | std::ios::binary);
    
    boost::archive::binary_iarchive iarch(iCacheFile);
    iarch >> data;
    iCacheFile.close();
    
    CalculationTime::algorithmCalculationTime.stopStep();
    std::cout << "-- Loaded cached data from file " << cacheFileName << " -- " << CalculationTime::algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
    
    return data;
  
  }
  
  const std::map<unsigned long long,Connection> DbFetcher::getConnectionsById(std::string applicationShortname, std::string dataFetcher, std::string connectionsSqlWhereClause, Parameters& params)
  {
    std::map<unsigned long long,Connection> connectionsById;
    // if cache file is not empty, load connections from cache:
    if (isCacheFileNotEmpty(applicationShortname, "connections"))
    {
      std::cout << "Fetching connections from cache..." << std::endl;
      connectionsById = loadFromCacheFile(connectionsById, applicationShortname, "connections");;
      CalculationTime::algorithmCalculationTime.startStep();
      for (auto & connection : connectionsById)
      {
        std::vector<std::shared_ptr<SimplifiedJourneyStep> > journeySteps;
        
        connection.second.reachable    = 0;
        connection.second.journeySteps = journeySteps;
        //connection.second.journeySteps.reserve(10000);
      }
      CalculationTime::algorithmCalculationTime.stopStep();
      std::cout << "-- Input from connections cache file --" << CalculationTime::algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
      return connectionsById;
    }
    
    if (dataFetcher == "database")
    {
      
      CalculationTime::algorithmCalculationTime.startStep();
      std::cout << "Fetching connections from database..." << std::endl;
      
      // query for connections:
      std::string sqlQuery = "SELECT i, a_id, r_id, t_id, seq, rt_id, sv_id, pss_o, pss_d, can_board, can_unboard, atm_d, dtm_o, COALESCE(cs_next,-1), COALESCE(cs_prev,-1), s_o, s_d"
      " FROM " + applicationShortname + ".mv_tr_connections_csa_with_single_next_and_prev " + connectionsSqlWhereClause + " ORDER BY i";
        
      std::cout << sqlQuery << std::endl;
      
      if (isConnectionOpen())
      {
        
        CalculationTime::algorithmCalculationTime.startStep();
        
        pqxx::nontransaction pgNonTransaction(*(getConnectionPtr()));
        pqxx::result pgResult( pgNonTransaction.exec( sqlQuery ));
        unsigned long long resultCount = pgResult.size();
        unsigned long long i = 0;
        
        // set cout number of decimals to 2 for displaying progress percentage:
        std::cout << std::fixed;
        std::cout << std::setprecision(2);
        
        CalculationTime::algorithmCalculationTime.stopStep();
        std::cout << "-- Fetching connections data from database --" << CalculationTime::algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
        CalculationTime::algorithmCalculationTime.startStep();
        
        for (pqxx::result::const_iterator c = pgResult.begin(); c != pgResult.end(); ++c) {
          
          // create a new Connection for each row:
          Connection * connection = new Connection();
          std::vector<std::shared_ptr<SimplifiedJourneyStep> > journeySteps;
          
          // set trConnection attributes from row:
          connection->id                                  = c[0].as<unsigned long long>();
          connection->serviceId                           = c[6].as<unsigned long long>();
          connection->agencyId                            = c[1].as<unsigned long long>();
          connection->routeId                             = c[2].as<unsigned long long>();
          connection->routeTypeId                         = c[5].as<unsigned long long>();
          connection->tripId                              = c[3].as<unsigned long long>();
          connection->sequence                            = c[4].as<unsigned long long>();
          connection->departureFromOriginTimeMinuteOfDay  = c[12].as<int>();
          connection->arrivalAtDestinationTimeMinuteOfDay = c[11].as<int>();
          connection->pathStopSequenceStartId             = c[7].as<unsigned long long>();
          connection->pathStopSequenceEndId               = c[8].as<unsigned long long>();
          connection->stopStartId                         = c[15].as<unsigned long long>();
          connection->stopEndId                           = c[16].as<unsigned long long>();
          connection->canBoard                            = c[9].as<unsigned long long>();
          connection->canUnboard                          = c[10].as<unsigned long long>();
          connection->reachable                           = 0;
          connection->nextConnectionId                    = c[13].as<long long>();
          connection->previousConnectionId                = c[14].as<long long>();
          connection->journeySteps                        = journeySteps;
          
          // add the trConnection to the connections vector:
          connectionsById[connection->id] = *connection;
          
          // show loading progress in percentage:
          i++;
          if (i % 1000 == 0)
          {
            std::cout << ((((double) i) / resultCount) * 100) << "%\r"; // \r is used to stay on the same line
          }
        }
        std::cout << std::endl;
        
        CalculationTime::algorithmCalculationTime.stopStep();
        std::cout << "-- Put connections data into a map --" << CalculationTime::algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
        
        // save connections to binary cache file:
        saveToCacheFile(applicationShortname, connectionsById, "connections");
  
      } else {
        std::cout << "Can't open database" << std::endl;
      }
      
    }
    else if(dataFetcher == "csv")
    {
      
      CalculationTime::algorithmCalculationTime.startStep();
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
        connection->previousConnectionId                = boost::lexical_cast<long long>          (*it);
        connection->journeySteps                        = journeySteps;
        
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
      
      CalculationTime::algorithmCalculationTime.stopStep();
      std::cout << "-- Put connections data into a map --" << CalculationTime::algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
      
      // save connections to binary cache file:
      saveToCacheFile(applicationShortname, connectionsById, "connections");
      
    }
    
    return connectionsById;
    
  }
  
  const std::map<unsigned long long,Stop> DbFetcher::getStopsById(std::string applicationShortname, std::string dataFetcher, int maxTimeValue)
  {
    std::map<unsigned long long,Stop> stopsById;
    // if cache file is not empty, load stops from cache:
    if (isCacheFileNotEmpty(applicationShortname, "stops"))
    {
      std::cout << "Fetching stops from cache..." << std::endl;
      stopsById = loadFromCacheFile(stopsById, applicationShortname, "stops");
      
      for (auto & stop : stopsById)
      {
        std::vector<std::shared_ptr<SimplifiedJourneyStep> > journeySteps;
        stop.second.arrivalTimeMinuteOfDay              = maxTimeValue;
        stop.second.journeySteps                        = journeySteps;
        stop.second.canUnboardToDestination             = false;
      }
      
      return stopsById;
    }
    
    if (dataFetcher == "database")
    {
      
      std::cout << "Fetching stops from database..." << std::endl;
      
      // query for stops:
      std::string sqlQuery = "SELECT s.id, COALESCE(s.code,'?'), COALESCE(s.name,'?'), COALESCE(s.station_id, -1), ST_Y(s.geography::geometry), ST_X(s.geography::geometry)"
      " FROM " + applicationShortname + ".tr_stops s "
      " ORDER BY s.id";
      
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
          
          // create a new Stop for each row:
          Stop * stop = new Stop();
          Point * point = new Point();
          std::vector<std::shared_ptr<SimplifiedJourneyStep> > journeySteps;
          // set Stop attributes from row:
          stop->id                                  = c[0].as<unsigned long long>();
          stop->code                                = c[1].as<std::string>();
          stop->name                                = c[2].as<std::string>();
          stop->stationId                           = c[3].as<long long>();
          stop->point                               = *point;
          stop->point.latitude                      = c[4].as<double>();
          stop->point.longitude                     = c[5].as<double>();
          stop->arrivalTimeMinuteOfDay              = maxTimeValue;
          stop->journeySteps                        = journeySteps;
          stop->canUnboardToDestination             = false;
          
          // add the stop to the map:
          stopsById[stop->id] = *stop;
          
          // show loading progress in percentage:
          i++;
          if (i % 1000 == 0)
          {
            std::cout << ((((double) i) / resultCount) * 100) << "%\r"; // \r is used to stay on the same line
          }
        }
        std::cout << std::endl;
              
        // save stops to binary cache file:
        saveToCacheFile(applicationShortname, stopsById, "stops");
      
      } else {
        std::cout << "Can't open database" << std::endl;
      }
    
    }
    else if(dataFetcher == "csv")
    {
      
      CalculationTime::algorithmCalculationTime.startStep();
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
      
      CalculationTime::algorithmCalculationTime.stopStep();
      std::cout << "-- Put stops data into a map --" << CalculationTime::algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
      
      // save connections to binary cache file:
      saveToCacheFile(applicationShortname, stopsById, "stops");
       
    }
    
    return stopsById;
    
  }
  
  const std::map<unsigned long long,Route> DbFetcher::getRoutesById(std::string applicationShortname, std::string dataFetcher)
  {
    std::map<unsigned long long,Route> routesById;
    // if cache file is not empty, load stops from cache:
    if (isCacheFileNotEmpty(applicationShortname, "routes"))
    {
      std::cout << "Fetching routes from cache..." << std::endl;
      routesById = loadFromCacheFile(routesById, applicationShortname, "routes");
      
      return routesById;
    }
    
    if (dataFetcher == "database")
    {
      
      std::cout << "Fetching routes from database..." << std::endl;
    
      // query for stops:
      std::string sqlQuery = "SELECT r.id, r.agency_id, r.type_id, a.acronym, a.name, COALESCE(r.shortname,'?') as route_shortname, COALESCE(r.longname,'?') as route_longname, rt.name"
      " FROM " + applicationShortname + ".tr_routes r "
      " LEFT JOIN " + applicationShortname + ".tr_route_types rt ON rt.id = r.type_id "
      " LEFT JOIN " + applicationShortname + ".tr_agencies a ON a.id = r.agency_id"
      " ORDER BY r.id";
          
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
        
        for (pqxx::result::const_iterator c = pgResult.begin(); c != pgResult.end(); ++c) {
          
          // create a new Route for each row:
          Route * route = new Route();
          // set Route attributes from row:
                  
          route->id                                  = c[0].as<unsigned long long>();
          route->agencyId                            = c[1].as<unsigned long long>();
          route->routeTypeId                         = c[2].as<unsigned long long>();
          route->agencyAcronym                       = c[3].as<std::string>();
          route->agencyName                          = c[4].as<std::string>();
          route->shortname                           = c[5].as<std::string>();
          route->longname                            = c[6].as<std::string>();
          route->routeTypeName                       = c[7].as<std::string>();
          
          // add the route to the map:
          routesById[route->id] = *route;
          
          // show loading progress in percentage:
          i++;
          if (i % 1000 == 0)
          {
            std::cout << ((((double) i) / resultCount) * 100) << "%\r"; // \r is used to stay on the same line
          }
        }
        std::cout << std::endl;
        
        // save stops to binary cache file:
        saveToCacheFile(applicationShortname, routesById, "routes");
      
      } else {
        std::cout << "Can't open database" << std::endl;
      }
    
    }
    else if(dataFetcher == "csv")
    {
      
      CalculationTime::algorithmCalculationTime.startStep();
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
      
      CalculationTime::algorithmCalculationTime.stopStep();
      std::cout << "-- Put routes data into a map --" << CalculationTime::algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
      
      // save connections to binary cache file:
      saveToCacheFile(applicationShortname, routesById, "routes");
       
    }
    
    return routesById;
    
  }
  
  const std::map<unsigned long long,PathStopSequence> DbFetcher::getPathStopSequencesById(std::string applicationShortname, std::string dataFetcher)
  {
    std::map<unsigned long long,PathStopSequence> pathStopSequencesById;
    // if cache file is not empty, load path stop sequences from cache:
    if (isCacheFileNotEmpty(applicationShortname, "path_stop_sequences"))
    {
      std::cout << "Fetching path stop sequences from cache..." << std::endl;
      pathStopSequencesById = loadFromCacheFile(pathStopSequencesById, applicationShortname, "path_stop_sequences");
      
      CalculationTime::algorithmCalculationTime.startStep();
      
      //for (auto & pathStopSequence : pathStopSequencesById)
      //{
      //  //std::vector<*JourneyStep> journeySteps;
      //  //pathStopSequence.second.arrivalTimeMinuteOfDay               = maxTimeValue;
      //  //pathStopSequence.second.walkingTimeNeededAfterUnboardMinutes = -1;
      //  //pathStopSequence.second.journeySteps                         = journeySteps; 
      //}
      
      CalculationTime::algorithmCalculationTime.stopStep();
      std::cout << "-- Input from path stop sequences cache file --" << CalculationTime::algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
      
      return pathStopSequencesById;
    }
    
    if (dataFetcher == "database")
    {
    
      CalculationTime::algorithmCalculationTime.startStep();
      std::cout << "Fetching path stop sequences from database..." << std::endl;
      
      // query for path stop sequences:
      std::string sqlQuery = "SELECT pss.id, pss.sequence, pss.stop_id, pss.path_id, rp.route_id, r.type_id, r.agency_id, COALESCE(s.station_id, -1)"
      " FROM " + applicationShortname + ".tr_path_stop_sequences pss "
      " LEFT JOIN " + applicationShortname + ".tr_route_paths rp ON rp.id = pss.path_id "
      " LEFT JOIN " + applicationShortname + ".tr_stops s ON s.id = pss.stop_id "
      " LEFT JOIN " + applicationShortname + ".tr_routes r ON r.id = rp.route_id "
      " ORDER BY pss.path_id, pss.sequence";
          
      if (isConnectionOpen())
      {
        
        CalculationTime::algorithmCalculationTime.startStep();
        
        pqxx::nontransaction pgNonTransaction(*(getConnectionPtr()));
        pqxx::result pgResult( pgNonTransaction.exec( sqlQuery ));
        unsigned long long resultCount = pgResult.size();
        unsigned long long i = 0;
        
        // set cout number of decimals to 2 for displaying progress percentage:
        std::cout << std::fixed;
        std::cout << std::setprecision(2);
        
        CalculationTime::algorithmCalculationTime.stopStep();
        std::cout << "-- Fetching path stop sequences from database --" << CalculationTime::algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
        CalculationTime::algorithmCalculationTime.startStep();
        
        for (pqxx::result::const_iterator c = pgResult.begin(); c != pgResult.end(); ++c) {
          
          // create a new PathStopSequence for each row:
          PathStopSequence * pathStopSequence = new PathStopSequence();
          // set PathStopSequence attributes from row:
          pathStopSequence->id                                   = c[0].as<unsigned long long>();
          pathStopSequence->sequence                             = c[1].as<unsigned long long>();
          pathStopSequence->stopId                               = c[2].as<unsigned long long>();
          pathStopSequence->routePathId                          = c[3].as<unsigned long long>();
          pathStopSequence->routeId                              = c[4].as<unsigned long long>();
          pathStopSequence->routeTypeId                          = c[5].as<unsigned long long>();
          pathStopSequence->agencyId                             = c[6].as<unsigned long long>();
          
          // add the path stop sequence to the map:
          pathStopSequencesById[pathStopSequence->id] = *pathStopSequence;
          
          // show loading progress in percentage:
          i++;
          if (i % 1000 == 0)
          {
            std::cout << ((((double) i) / resultCount) * 100) << "%\r"; // \r is used to stay on the same line
          }
        }
        std::cout << std::endl;
        
        CalculationTime::algorithmCalculationTime.stopStep();
        std::cout << "-- Put path stop sequences data into a map --" << CalculationTime::algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
        
        // save path stop sequences to binary cache file:
        saveToCacheFile(applicationShortname, pathStopSequencesById, "path_stop_sequences");
      
      } else {
        std::cout << "Can't open database" << std::endl;
      }
    
    }
    else if(dataFetcher == "csv")
    {
      
      CalculationTime::algorithmCalculationTime.startStep();
      std::cout << "Fetching path stop sequences from csv files..." << std::endl;
      
      std::string csvFilePath{applicationShortname + "_path_stop_sequences.csv"};
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
        
        // create a new PathStopSequence for each row:
        PathStopSequence * pathStopSequence = new PathStopSequence();
        Tokenizer::iterator it = tok.begin();
        
        // set PathStopSequence attributes from row:
        pathStopSequence->id          = boost::lexical_cast<unsigned long long> (*it); std::advance(it,1);
        pathStopSequence->sequence    = boost::lexical_cast<unsigned long long> (*it); std::advance(it,1);
        pathStopSequence->stopId      = boost::lexical_cast<unsigned long long> (*it); std::advance(it,1);
        pathStopSequence->routePathId = boost::lexical_cast<unsigned long long> (*it); std::advance(it,1);
        pathStopSequence->routeId     = boost::lexical_cast<unsigned long long> (*it); std::advance(it,1);
        pathStopSequence->routeTypeId = boost::lexical_cast<unsigned long long> (*it); std::advance(it,1);
        pathStopSequence->agencyId    = boost::lexical_cast<unsigned long long> (*it);
        
        // add the path stop sequence to the map:
        pathStopSequencesById[pathStopSequence->id] = *pathStopSequence;
        
        // show loading progress:
        i++;
        if (i % 100 == 0)
        {
          std::cout << i << " path stop sequences\r"; // \r is used to stay on the same line
        }
        
      }
      std::cout << std::endl;
      
      csvFile.close();
      
      CalculationTime::algorithmCalculationTime.stopStep();
      std::cout << "-- Put path stop sequences data into a map --" << CalculationTime::algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
      
      // save connections to binary cache file:
      saveToCacheFile(applicationShortname, pathStopSequencesById, "path_stop_sequences");
       
    }
    
    return pathStopSequencesById;
    
  }
  
  const std::map<unsigned long long,std::map<unsigned long long, int> > DbFetcher::getTransferDurationsByStopId(std::string applicationShortname, std::string dataFetcher, int maxTransferWalkingTravelTimeMinutes, std::string transfersSqlWhereClause)
  {
    
    std::map<unsigned long long,std::map<unsigned long long, int> > transferDurationsByStopId;
    
    // if cache file is not empty, load transfers from cache:
    if (isCacheFileNotEmpty(applicationShortname, "transfers"))
    {
      transferDurationsByStopId = loadFromCacheFile(transferDurationsByStopId, applicationShortname, "transfers");;
      std::cout << "-- Input from transfers cache file --" << CalculationTime::algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
      return transferDurationsByStopId;
    }
    
    if (dataFetcher == "database")
    {
    
      CalculationTime::algorithmCalculationTime.startStep();
      std::cout << "Fetching transfer walking durations from database..." << std::endl;
      std::cout << "max transfer time minutes: " << std::to_string(maxTransferWalkingTravelTimeMinutes) << std::endl;
      std::string sqlQuery = "SELECT "
        "stop_1_id as s1, "
        "stop_2_id as s2, "
        "MAX(CEIL(COALESCE(network_walking_duration_seconds::float/60, network_distance::float/1.38/60, distance::float/1.38/60))) as ttm "
        "FROM " + applicationShortname + ".tr_matrix_stop_distances "
        "WHERE " + transfersSqlWhereClause + " AND CEIL(COALESCE(network_walking_duration_seconds::float/60, network_distance::float/1.38/60, distance::float/1.38/60)) <= " + std::to_string(maxTransferWalkingTravelTimeMinutes) + "::float "
        "GROUP BY stop_1_id, stop_2_id "
        "ORDER BY stop_1_id, MAX(CEIL(COALESCE(network_walking_duration_seconds::float, network_distance::float/1.38, distance::float/1.38))) ";
      
      unsigned long long stopId1;
      unsigned long long stopId2;
      int walkingDurationMinutes;
      
      if (isConnectionOpen())
      {
        
        CalculationTime::algorithmCalculationTime.startStep();
        
        pqxx::nontransaction pgNonTransaction(*(getConnectionPtr()));
        pqxx::result pgResult( pgNonTransaction.exec( sqlQuery ));
        unsigned long long resultCount = pgResult.size();
        unsigned long long i = 1;
        
        std::cout << std::fixed;
        std::cout << std::setprecision(2);
        
        CalculationTime::algorithmCalculationTime.stopStep();
        std::cout << "-- Fetching transfers data from database --" << CalculationTime::algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
        CalculationTime::algorithmCalculationTime.startStep();
        
        for (pqxx::result::const_iterator c = pgResult.begin(); c != pgResult.end(); ++c) {
          stopId1                = c[0].as<unsigned long long>();
          stopId2                = c[1].as<unsigned long long>();
          walkingDurationMinutes = c[2].as<int>();
          transferDurationsByStopId[stopId1][stopId2] = walkingDurationMinutes;
          i++;
          
          if (i % 1000 == 0)
          {
            std::cout << ((((double) i) / resultCount) * 100) << "%\r";
          }
        }
        std::cout << std::endl;
        
        CalculationTime::algorithmCalculationTime.stopStep();
        std::cout << "-- Put transfers data into a map --" << CalculationTime::algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
        
        saveToCacheFile(applicationShortname, transferDurationsByStopId, "transfers");
        
      
      } else {
        std::cout << "Can't open database" << std::endl;
      }
    
    }
    else if(dataFetcher == "csv")
    {
      
      CalculationTime::algorithmCalculationTime.startStep();
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
      
      CalculationTime::algorithmCalculationTime.stopStep();
      std::cout << "-- Put transfers data into a map --" << CalculationTime::algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
      
      // save connections to binary cache file:
      saveToCacheFile(applicationShortname, transferDurationsByStopId, "transfers");
       
    }
    
    return transferDurationsByStopId;
    
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
    
    std::cout << "mode = " << mode << " speed = " << speedMetersPerSecond << " maxTravelTime = " << maxTravelTimeMinutes << " port = " << routingPort << std::endl;
    
    std::cout << std::fixed;
    std::cout << std::setprecision(6);
    
    std::map<unsigned long long, int> nearestStopsIds;
      
    if (dataFetcher == "database")
    {
    
      std::string sqlQuery = "SELECT tr_stops.id, CEIL((ST_DISTANCE(ST_GEOMFROMTEXT('" + point.asWKT() + "'), geography))/(" + std::to_string(speedMetersPerSecond) + "*60)) as ttm FROM " + applicationShortname + ".tr_stops WHERE CEIL((ST_DISTANCE(ST_GEOMFROMTEXT('" + point.asWKT() + "'), geography))/(" + std::to_string(speedMetersPerSecond) + "*60)) <= " + std::to_string(maxTravelTimeMinutes);
      
      if (isConnectionOpen())
      {
        pqxx::nontransaction pgNonTransaction(*(getConnectionPtr()));
        pqxx::result pgResult( pgNonTransaction.exec( sqlQuery ));
        unsigned long long resultCount = pgResult.size();
        unsigned long long i = 1;
        
        for (pqxx::result::const_iterator c = pgResult.begin(); c != pgResult.end(); ++c) {
          
          nearestStopsIds[c[0].as<unsigned long long>()] = c[1].as<int>();
          
        }
      
      } else {
        std::cout << "Can't open database" << std::endl;
      }
      
    }
    else if(dataFetcher == "csv")
    {
      
      for (auto & stopId : stopsById)
      {
        nearestStopsIds[stopId.first] = 9999;
      }
      
    }
        
    boost::asio::ip::tcp::iostream s;
    s.connect(routingHost, routingPort);
    
    std::string queryString = "GET /table/v1/" + mode + "/" + std::to_string(point.longitude) +  "," + std::to_string(point.latitude);
    for (auto & stopId : nearestStopsIds)
    {
      queryString += ";" + std::to_string(stopsById[stopId.first].point.longitude) +  "," + std::to_string(stopsById[stopId.first].point.latitude);
    }
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
    
    
    std::cout << "duration count = " << pt.count("durations") << std::endl;
    
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
      std::cout << "-- Input from travel_time_by_stops_pair cache file --" << CalculationTime::algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
      return travelTimeByStopsPair;
    }
    
    CalculationTime::algorithmCalculationTime.startStep();
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
      
      CalculationTime::algorithmCalculationTime.startStep();
      
      pqxx::nontransaction pgNonTransaction(*(getConnectionPtr()));
      pqxx::result pgResult( pgNonTransaction.exec( sqlQuery ));
      unsigned long long resultCount = pgResult.size();
      unsigned long long i = 1;
      
      std::cout << std::fixed;
      std::cout << std::setprecision(2);
      
      CalculationTime::algorithmCalculationTime.stopStep();
      std::cout << "-- Fetching travel time data from database --" << CalculationTime::algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
      CalculationTime::algorithmCalculationTime.startStep();
      
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
      
      CalculationTime::algorithmCalculationTime.stopStep();
      std::cout << "-- Put travel time data into a map --" << CalculationTime::algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
      
      saveToCacheFile(applicationShortname, travelTimeByStopsPair, "travel_time_by_stops_pair");
      

    } else {
      std::cout << "Can't open database" << std::endl;
    }
    
    return travelTimeByStopsPair;
    
  }
  
}

