#include "server_http.hpp"
#include "client_http.hpp"

//Added for the json-example
#define BOOST_SPIRIT_THREADSAFE
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>


//Added for the default_resource example
#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include <iostream>
#include <iterator>
#include <curses.h>
#include <locale>

#include "toolbox.hpp"
#include "database_fetcher.hpp"
#include "gtfs_fetcher.hpp"
#include "csv_fetcher.hpp"
#include "cache_fetcher.hpp"
#include "calculation_time.hpp"
#include "parameters.hpp"
#include "calculator.hpp"
#include "combinations.hpp"

//Added for the json-example:
using namespace boost::property_tree;
using namespace TrRouting;

typedef SimpleWeb::Server<SimpleWeb::HTTP> HttpServer;
typedef SimpleWeb::Client<SimpleWeb::HTTP> HttpClient;

int stepCount = 1;

std::string consoleRed        = "";
std::string consoleGreen      = "";
std::string consoleYellow     = "";
std::string consoleCyan       = "";
std::string consoleMagenta    = "";
std::string consoleResetColor = "";

//namespace 
//{ 
//  const size_t ERROR_IN_COMMAND_LINE = 1;
//}

//Added for the default_resource example
void default_resource_send(const HttpServer &server, const std::shared_ptr<HttpServer::Response> &response,
                           const std::shared_ptr<std::ifstream> &ifs);

int main(int argc, char** argv) {
  
  int serverPort {4000};
  std::string dataFetcherStr {"database"}; // csv, database
  
  // Get application shortname from config file:
  std::string applicationShortname;
  std::ifstream applicationShortnameFile;
  applicationShortnameFile.open("application_shortname.txt");
  std::getline(applicationShortnameFile, applicationShortname);
  applicationShortnameFile.close();
  std::string dataShortname {applicationShortname};
  
  // Set params:
  Parameters algorithmParams;
  algorithmParams.setDefaultValues();
  
  // setup program options:
  
  boost::program_options::options_description optionsDesc("Options"); 
  boost::program_options::variables_map variablesMap;
  optionsDesc.add_options() 
      ("port,p", boost::program_options::value<int>(), "http server port");
  optionsDesc.add_options() 
      ("dataFetcher,data", boost::program_options::value<std::string>(), "data fetcher (csv or database or cache)");
  optionsDesc.add_options() 
      ("dataShortname,sn", boost::program_options::value<std::string>(), "data shortname (shortname of the application to use or data to use)");
  optionsDesc.add_options() 
      ("osrmWalkPort",     boost::program_options::value<std::string>(), "osrm walking port");
  optionsDesc.add_options() 
      ("databaseUser",     boost::program_options::value<std::string>(), "database user");
  optionsDesc.add_options() 
      ("databaseName",     boost::program_options::value<std::string>(), "database name");
  optionsDesc.add_options() 
      ("databasePort",     boost::program_options::value<std::string>(), "database port");
  optionsDesc.add_options() 
      ("databaseHost",     boost::program_options::value<std::string>(), "database host");
  optionsDesc.add_options() 
      ("databasePassword", boost::program_options::value<std::string>(), "database password");
  optionsDesc.add_options() 
      ("updateOdTrips", boost::program_options::value<std::string>(), "update od trips access and egress stops or not (1 or 0)");
  
  boost::program_options::store(boost::program_options::parse_command_line(argc, argv, optionsDesc), variablesMap);
  
  if(variablesMap.count("port") == 1)
  {
    serverPort = variablesMap["port"].as<int>();
  }
  else if (variablesMap.count("p") == 1)
  {
    serverPort = variablesMap["p"].as<int>();
  }
  if(variablesMap.count("dataFetcher") == 1)
  {
    dataFetcherStr = variablesMap["dataFetcher"].as<std::string>();
  }
  else if (variablesMap.count("data") == 1)
  {
    dataFetcherStr = variablesMap["data"].as<std::string>();
  }
  if(variablesMap.count("dataShortname") == 1)
  {
    dataShortname = variablesMap["dataShortname"].as<std::string>();
  }
  if(variablesMap.count("sn") == 1)
  {
    dataShortname = variablesMap["sn"].as<std::string>();
  }
  if(variablesMap.count("osrmWalkPort") == 1)
  {
    algorithmParams.osrmRoutingWalkingPort = variablesMap["osrmWalkPort"].as<std::string>();
  }
  if(variablesMap.count("databasePort") == 1)
  {
    algorithmParams.databasePort = variablesMap["databasePort"].as<std::string>();
  }
  if(variablesMap.count("databaseHost") == 1)
  {
    algorithmParams.databaseHost = variablesMap["databaseHost"].as<std::string>();
  }
  if(variablesMap.count("databaseUser") == 1)
  {
    algorithmParams.databaseUser = variablesMap["databaseUser"].as<std::string>();
  }
  if(variablesMap.count("databaseName") == 1)
  {
    algorithmParams.databaseName = variablesMap["databaseName"].as<std::string>();
  }
  if(variablesMap.count("databasePassword") == 1)
  {
    algorithmParams.databasePassword = variablesMap["databasePassword"].as<std::string>();
  }
  if(variablesMap.count("updateOdTrips") == 1)
  {
    algorithmParams.updateOdTrips = variablesMap["updateOdTrips"].as<int>();
  }
  
  
  std::cout << "Using http port "      << serverPort << std::endl;
  std::cout << "Using osrm walk port "  << algorithmParams.osrmRoutingWalkingPort << std::endl;
  std::cout << "Using data fetcher "   << dataFetcherStr << std::endl;
  std::cout << "Using data shortname " << dataShortname << std::endl;
  
  // setup console colors 
  // (this will create a new terminal window, check if the terminal is color-capable and then it will close the terminal window with endwin()):
  initscr();
  start_color();
  if (has_colors())
  {
    consoleRed        = "\033[0;31m";
    consoleGreen      = "\033[1;32m";
    consoleYellow     = "\033[1;33m";
    consoleCyan       = "\033[0;36m";
    consoleMagenta    = "\033[0;35m";
    consoleResetColor = "\033[0m";
  }
  else
  {
    consoleRed        = "";
    consoleGreen      = "";
    consoleYellow     = "";
    consoleCyan       = "";
    consoleMagenta    = "";
    consoleResetColor = "";
  }
  endwin();
  
  std::cout << "Starting transit routing for the application: ";
  std::cout << consoleGreen + dataShortname + consoleResetColor << std::endl << std::endl;
  
  Calculator  calculator;
  algorithmParams.applicationShortname = dataShortname;
  algorithmParams.dataFetcherShortname = dataFetcherStr;
  
  DatabaseFetcher databaseFetcher;
  if (dataFetcherStr == "database")
  {
    databaseFetcher = DatabaseFetcher("dbname=" + algorithmParams.databaseName + " user=" + algorithmParams.databaseUser + " hostaddr=" + algorithmParams.databaseHost + " port=" + algorithmParams.databasePort + "");
    algorithmParams.databaseFetcher = &databaseFetcher;
  }
  else
  {
    algorithmParams.databaseFetcher = &databaseFetcher;
  }
  GtfsFetcher gtfsFetcher         = GtfsFetcher();
  algorithmParams.gtfsFetcher     = &gtfsFetcher;
  CsvFetcher csvFetcher           = CsvFetcher();
  algorithmParams.csvFetcher      = &csvFetcher;
  CacheFetcher cacheFetcher       = CacheFetcher();
  algorithmParams.cacheFetcher    = &cacheFetcher;
  
  calculator = Calculator(algorithmParams);
  int i = 0;
  
  /////////
  
  std::cout << "preparing server..." << std::endl;
  
  //HTTP-server using 1 thread
  //Unless you do more heavy non-threaded processing in the resources,
  //1 thread is usually faster than several threads
  HttpServer server;
  server.config.port = serverPort;
  
  server.resource["^/route/v1/transit[/]?\\?([0-9a-zA-Z&=_,:/.-]+)$"]["GET"]=[&server, &calculator](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
    
    calculator.algorithmCalculationTime.start();
    
    //calculator.algorithmCalculationTime.startStep();
    
    std::cout << "calculating request..." << std::endl;
    
    std::string resultStr;
    std::string csv;
    
    std::cout << request->path << std::endl;
    
    bool saveToFile {false}; // save result to file instead of returning it in response
    std::string calculationName {"trRoutingResult"};
    std::string fileFormat {"json"}; // csv can be used, but only with calculateAllTrips = true
    
    if (request->path_match.size() >= 1)
    {
      std::vector<std::string> parametersWithValues;
      
      std::string queryString = request->path_match[1];
      
      boost::split(parametersWithValues, queryString, boost::is_any_of("&"));
      
      float originLatitude, originLongitude, destinationLatitude, destinationLongitude;
      long long startingStopId{-1}, endingStopId{-1};
      std::vector<int> onlyServiceIds;
      std::vector<unsigned long long> exceptServiceIds;
      std::vector<unsigned long long> onlyRouteIds;
      std::vector<unsigned long long> exceptRouteIds;
      std::vector<unsigned long long> onlyRouteTypeIds;
      std::vector<unsigned long long> exceptRouteTypeIds;
      std::vector<unsigned long long> onlyAgencyIds;
      std::vector<unsigned long long> exceptAgencyIds;
      std::vector<unsigned long long> accessStopIds;
      std::vector<unsigned long long> egressStopIds;
      std::vector<int> accessStopTravelTimesSeconds;
      std::vector<int> egressStopTravelTimesSeconds;
      std::vector<std::pair<int,int>> odTripsPeriods; // pair: start_at_seconds, end_at_seconds
      std::vector<std::string> odTripsGenders;
      std::vector<std::string> odTripsAgeGroups;
      std::vector<std::string> odTripsOccupations;
      std::vector<std::string> odTripsActivities;
      std::vector<std::string> odTripsModes;
      
      int odTripsSampleSize {-1}; // for testing only
      bool calculateAllOdTrips {false}; // fetch all od trips from cache or database and calculate for all these trips
      int batchNumber  {1}; // when using multiple batches (parallele calculations)
      int batchesCount {1};
      int odTripId {-1}; // when calculating for only one trip
      bool alternatives {false}; // calculate alternatives or not
      
      calculator.params.onlyServiceIds     = onlyServiceIds;
      calculator.params.exceptServiceIds   = exceptServiceIds;
      calculator.params.onlyRouteIds       = onlyRouteIds;
      calculator.params.exceptRouteIds     = exceptRouteIds;
      calculator.params.onlyRouteTypeIds   = onlyRouteTypeIds;
      calculator.params.exceptRouteTypeIds = exceptRouteTypeIds;
      calculator.params.onlyAgencyIds      = onlyAgencyIds;
      calculator.params.exceptAgencyIds    = exceptAgencyIds;
      //calculator.params.odTripsPeriods     = odTripsPeriods;
      //calculator.params.odTripsGenders     = odTripsGenders;
      //calculator.params.odTripsAgeGroups   = odTripsAgeGroups;
      //calculator.params.odTripsOccupations = odTripsOccupations;
      //calculator.params.odTripsActivities  = odTripsActivities;
      //calculator.params.odTripsModes       = odTripsModes;
      
      std::vector<std::string> parameterWithValueVector;
      std::vector<std::string> latitudeLongitudeVector;
      std::vector<std::string> dateVector;
      std::vector<std::string> timeVector;
      std::vector<std::string> onlyServiceIdsVector;
      std::vector<std::string> exceptServiceIdsVector;
      std::vector<std::string> onlyRouteIdsVector;
      std::vector<std::string> exceptRouteIdsVector;
      std::vector<std::string> onlyRouteTypeIdsVector;
      std::vector<std::string> exceptRouteTypeIdsVector;
      std::vector<std::string> onlyAgencyIdsVector;
      std::vector<std::string> exceptAgencyIdsVector;
      std::vector<std::string> accessStopIdsVector;
      std::vector<std::string> accessStopTravelTimesSecondsVector;
      std::vector<std::string> egressStopIdsVector;
      std::vector<std::string> egressStopTravelTimesSecondsVector;
      std::vector<std::string> odTripsPeriodsVector;
      std::vector<std::string> odTripsGendersVector;
      std::vector<std::string> odTripsAgeGroupsVector;
      std::vector<std::string> odTripsOccupationsVector;
      std::vector<std::string> odTripsActivitiesVector;
      std::vector<std::string> odTripsModesVector;

      int timeHour;
      int timeMinute;
      
      calculator.params.forwardCalculation                     = true;
      calculator.params.detailedResults                        = false;
      calculator.params.returnAllStopsResult                   = false;
      calculator.params.transferOnlyAtSameStation              = false;
      calculator.params.transferBetweenSameRoute               = true;
      calculator.params.origin                                 = Point();
      calculator.params.destination                            = Point();
      calculator.params.routingDateYear                        = 0;
      calculator.params.routingDateMonth                       = 0;
      calculator.params.routingDateDay                         = 0;
      calculator.params.originStopId                           = -1;
      calculator.params.destinationStopId                      = -1;
      calculator.params.odTrip                                 = NULL;
      calculator.params.maxNumberOfTransfers                   = -1;
      calculator.params.minWaitingTimeSeconds                  = 5 * 60;
      calculator.params.departureTimeHour                      = -1;
      calculator.params.departureTimeMinutes                   = -1;
      calculator.params.departureTimeSeconds                   = -1;
      calculator.params.arrivalTimeHour                        = -1;
      calculator.params.arrivalTimeMinutes                     = -1;
      calculator.params.arrivalTimeSeconds                     = -1;
      calculator.params.maxTotalTravelTimeSeconds              = MAX_INT;
      calculator.params.maxAccessWalkingTravelTimeSeconds      = 20 * 60;
      calculator.params.maxEgressWalkingTravelTimeSeconds      = 20 * 60;
      calculator.params.maxTransferWalkingTravelTimeSeconds    = 20 * 60;
      calculator.params.maxTotalWalkingTravelTimeSeconds       = 60 * 60;
      calculator.params.maxOnlyWalkingAccessTravelTimeRatio    = 1.5;
      calculator.params.transferPenaltySeconds                 = 0;
      calculator.params.accessMode                             = "walking";
      calculator.params.egressMode                             = "walking";
      calculator.params.noResultSecondMode                     = "driving";
      calculator.params.tryNextModeIfRoutingFails              = false;
      calculator.params.noResultNextAccessTimeSecondsIncrement = 5 * 60;
      calculator.params.maxNoResultNextAccessTimeSeconds       = 40 * 60;
      calculator.params.calculateByNumberOfTransfers           = false;
      calculator.params.alternatives                           = false;
      calculator.params.accessStopIds.clear();
      calculator.params.egressStopIds.clear();
      calculator.params.accessStopTravelTimesSeconds.clear();
      calculator.params.egressStopTravelTimesSeconds.clear();

      for(auto & parameterWithValue : parametersWithValues)
      {
        boost::split(parameterWithValueVector, parameterWithValue, boost::is_any_of("="));
        
        if (parameterWithValueVector[0] == "origin")
        {
          boost::split(latitudeLongitudeVector, parameterWithValueVector[1], boost::is_any_of(","));
          originLatitude           = std::stof(latitudeLongitudeVector[0]);
          originLongitude          = std::stof(latitudeLongitudeVector[1]);
          calculator.params.origin = Point(originLatitude, originLongitude);
        }
        else if (parameterWithValueVector[0] == "destination")
        {
          boost::split(latitudeLongitudeVector, parameterWithValueVector[1], boost::is_any_of(","));
          destinationLatitude           = std::stof(latitudeLongitudeVector[0]);
          destinationLongitude          = std::stof(latitudeLongitudeVector[1]);
          calculator.params.destination = Point(destinationLatitude, destinationLongitude);
        }
        else if (parameterWithValueVector[0] == "calculation_name")
        {
          calculationName = parameterWithValueVector[1];
        }
        else if (parameterWithValueVector[0] == "file_format")
        {
          fileFormat = parameterWithValueVector[1];
        }
        else if (parameterWithValueVector[0] == "batch")
        {
          batchNumber = std::stoi(parameterWithValueVector[1]);
        }
        else if (parameterWithValueVector[0] == "num_batches")
        {
          batchesCount = std::stoi(parameterWithValueVector[1]);
        }
        else if (parameterWithValueVector[0] == "date")
        {
          boost::split(dateVector, parameterWithValueVector[1], boost::is_any_of("/"));
          calculator.params.routingDateYear      = std::stoi(dateVector[0]);
          calculator.params.routingDateMonth     = std::stoi(dateVector[1]);
          calculator.params.routingDateDay       = std::stoi(dateVector[2]);
        }
        else if (parameterWithValueVector[0] == "time"
                 || parameterWithValueVector[0] == "departure"      || parameterWithValueVector[0] == "arrival"
                 || parameterWithValueVector[0] == "departure_time" || parameterWithValueVector[0] == "arrival_time"
                 || parameterWithValueVector[0] == "start_time"     || parameterWithValueVector[0] == "end_time"
                )
        {
          boost::split(timeVector, parameterWithValueVector[1], boost::is_any_of(":"));
          timeHour      = std::stoi(timeVector[0]);
          timeMinute    = std::stoi(timeVector[1]);
        }
        else if (parameterWithValueVector[0] == "access_stop_ids")
        {
          boost::split(accessStopIdsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string accessStopId : accessStopIdsVector)
          {
            accessStopIds.push_back(std::stoll(accessStopId));
          }
          calculator.params.accessStopIds = accessStopIds;
        }
        else if (parameterWithValueVector[0] == "egress_stop_ids")
        {
          boost::split(egressStopIdsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string egressStopId : egressStopIdsVector)
          {
            egressStopIds.push_back(std::stoll(egressStopId));
          }
          calculator.params.egressStopIds = egressStopIds;
        }
        else if (parameterWithValueVector[0] == "od_trip_id")
        {
          odTripId = std::stoi(parameterWithValueVector[1]);
        }
        else if (parameterWithValueVector[0] == "od_trips")
        {
          if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1") { calculateAllOdTrips = true; }
        }
        else if (parameterWithValueVector[0] == "od_trips_sample_size")
        {
          odTripsSampleSize = std::stoi(parameterWithValueVector[1]);
        }
        else if (parameterWithValueVector[0] == "od_trips_periods")
        {
          boost::split(odTripsPeriodsVector, parameterWithValueVector[1], boost::is_any_of(","));
          int periodIndex {0};
          int startAtSeconds;
          int endAtSeconds;
          for(std::string odTripsTime : odTripsPeriodsVector)
          {
            if (periodIndex % 2 == 0)
            {
              startAtSeconds = std::stoi(odTripsTime);
            }
            else
            {
              endAtSeconds = std::stoi(odTripsTime);
              odTripsPeriods.push_back(std::make_pair(startAtSeconds, endAtSeconds));
            }
            periodIndex++;
          }
          //calculator.params.odTripsPeriods = odTripsPeriods;
        }
        else if (parameterWithValueVector[0] == "od_trips_age_groups")
        {
          boost::split(odTripsAgeGroupsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string odTripsAgeGroup : odTripsAgeGroupsVector)
          {
            odTripsAgeGroups.push_back(odTripsAgeGroup);
          }
          //calculator.params.odTripsAgeGroups = odTripsAgeGroups;
        }
        else if (parameterWithValueVector[0] == "od_trips_genders")
        {
          boost::split(odTripsGendersVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string odTripsGender : odTripsGendersVector)
          {
            odTripsGenders.push_back(odTripsGender);
          }
          //calculator.params.odTripsGenders = odTripsGenders;
        }
        else if (parameterWithValueVector[0] == "od_trips_occupations")
        {
          boost::split(odTripsOccupationsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string odTripsOccupation : odTripsOccupationsVector)
          {
            odTripsOccupations.push_back(odTripsOccupation);
          }
          //calculator.params.odTripsOccupations = odTripsOccupations;
        }
        else if (parameterWithValueVector[0] == "od_trips_activities")
        {
          boost::split(odTripsActivitiesVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string odTripsActivity : odTripsActivitiesVector)
          {
            odTripsActivities.push_back(odTripsActivity);
          }
          //calculator.params.odTripsActivities = odTripsActivities;
        }
        else if (parameterWithValueVector[0] == "od_trips_modes")
        {
          boost::split(odTripsModesVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string odTripsMode : odTripsModesVector)
          {
            odTripsModes.push_back(odTripsMode);
          }
          //calculator.params.odTripsModes = odTripsModes;
        }
        
        else if (parameterWithValueVector[0] == "access_stop_travel_times_seconds" || parameterWithValueVector[0] == "access_stop_travel_times")
        {
          boost::split(accessStopTravelTimesSecondsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string accessStopTravelTimeSeconds : accessStopTravelTimesSecondsVector)
          {
            accessStopTravelTimesSeconds.push_back(std::stoi(accessStopTravelTimeSeconds));
          }
          calculator.params.accessStopTravelTimesSeconds = accessStopTravelTimesSeconds;
        }
        else if (parameterWithValueVector[0] == "egress_stop_travel_times_seconds" || parameterWithValueVector[0] == "egress_stop_travel_times")
        {
          boost::split(egressStopTravelTimesSecondsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string egressStopTravelTimeSeconds : egressStopTravelTimesSecondsVector)
          {
            egressStopTravelTimesSeconds.push_back(std::stoi(egressStopTravelTimeSeconds));
          }
          calculator.params.egressStopTravelTimesSeconds = egressStopTravelTimesSeconds;
        }
        else if (parameterWithValueVector[0] == "only_service_ids")
        {
          boost::split(onlyServiceIdsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string onlyServiceId : onlyServiceIdsVector)
          {
            onlyServiceIds.push_back(std::stoll(onlyServiceId));
          }
          calculator.params.onlyServiceIds = onlyServiceIds;
        }
        else if (parameterWithValueVector[0] == "except_service_ids")
        {
          boost::split(exceptServiceIdsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string exceptServiceId : exceptServiceIdsVector)
          {
            exceptServiceIds.push_back(std::stoll(exceptServiceId));
          }
          calculator.params.exceptServiceIds = exceptServiceIds;
        }
        else if (parameterWithValueVector[0] == "only_route_ids")
        {
          boost::split(onlyRouteIdsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string onlyRouteId : onlyRouteIdsVector)
          {
            onlyRouteIds.push_back(std::stoll(onlyRouteId));
          }
          calculator.params.onlyRouteIds = onlyRouteIds;
        }
        else if (parameterWithValueVector[0] == "except_route_ids")
        {
          boost::split(exceptRouteIdsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string exceptRouteId : exceptRouteIdsVector)
          {
            exceptRouteIds.push_back(std::stoll(exceptRouteId));
          }
          calculator.params.exceptRouteIds = exceptRouteIds;
        }
        else if (parameterWithValueVector[0] == "only_route_type_ids")
        {
          boost::split(onlyRouteTypeIdsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string onlyRouteTypeId : onlyRouteTypeIdsVector)
          {
            onlyRouteTypeIds.push_back(std::stoll(onlyRouteTypeId));
          }
          calculator.params.onlyRouteTypeIds = onlyRouteTypeIds;
        }
        else if (parameterWithValueVector[0] == "except_route_type_ids")
        {
          boost::split(exceptRouteTypeIdsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string exceptRouteTypeId : exceptRouteTypeIdsVector)
          {
            exceptRouteTypeIds.push_back(std::stoll(exceptRouteTypeId));
          }
          calculator.params.exceptRouteTypeIds = exceptRouteTypeIds;
        }
        else if (parameterWithValueVector[0] == "only_agency_ids")
        {
          boost::split(onlyAgencyIdsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string onlyAgencyId : onlyAgencyIdsVector)
          {
            onlyAgencyIds.push_back(std::stoll(onlyAgencyId));
          }
          calculator.params.onlyAgencyIds = onlyAgencyIds;
        }
        else if (parameterWithValueVector[0] == "except_agency_ids")
        {
          boost::split(exceptAgencyIdsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string exceptAgencyId : exceptAgencyIdsVector)
          {
            exceptAgencyIds.push_back(std::stoll(exceptAgencyId));
          }
          calculator.params.exceptAgencyIds = exceptAgencyIds;
        }
        else if (parameterWithValueVector[0] == "starting_stop_id"
                 || parameterWithValueVector[0] == "start_stop_id"
                 || parameterWithValueVector[0] == "origin_stop_id"
                )
        {
          calculator.params.originStopId = std::stoll(parameterWithValueVector[1]);
        }
        else if (parameterWithValueVector[0] == "ending_stop_id"
                 || parameterWithValueVector[0] == "end_stop_id"
                 || parameterWithValueVector[0] == "destination_stop_id"
                )
        {
          calculator.params.destinationStopId = std::stoll(parameterWithValueVector[1]);
        }
        else if (parameterWithValueVector[0] == "max_number_of_transfers" || parameterWithValueVector[0] == "max_transfers")
        {
          calculator.params.maxNumberOfTransfers = std::stoi(parameterWithValueVector[1]);
        }
        else if (parameterWithValueVector[0] == "calculate_by_number_of_transfers" || parameterWithValueVector[0] == "by_num_transfers")
        {
          if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1") { calculator.params.calculateByNumberOfTransfers = true; }
        }
        else if (parameterWithValueVector[0] == "save_to_file")
        {
          if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1") { saveToFile = true; }
        }
        else if (parameterWithValueVector[0] == "min_waiting_time" || parameterWithValueVector[0] == "min_waiting_time_minutes")
        {
          calculator.params.minWaitingTimeSeconds = std::stoi(parameterWithValueVector[1]) * 60;
          if (calculator.params.minWaitingTimeSeconds < 0)
          {
            calculator.params.minWaitingTimeSeconds = 0;
          }
        }
        else if (parameterWithValueVector[0] == "max_travel_time" || parameterWithValueVector[0] == "max_travel_time_minutes")
        {
          calculator.params.maxTotalTravelTimeSeconds = std::stoi(parameterWithValueVector[1]) * 60;
          if (calculator.params.maxTotalTravelTimeSeconds == 0)
          {
            calculator.params.maxTotalTravelTimeSeconds = MAX_INT;
          }
        }
        else if (parameterWithValueVector[0] == "max_access_travel_time" || parameterWithValueVector[0] == "max_access_travel_time_minutes")
        {
          calculator.params.maxAccessWalkingTravelTimeSeconds = std::stoi(parameterWithValueVector[1]) * 60;
        }
        else if (parameterWithValueVector[0] == "alternatives_max_added_travel_time_minutes" || parameterWithValueVector[0] == "alt_max_added_travel_time")
        {
          calculator.params.alternativesMaxAddedTravelTimeSeconds = std::stoi(parameterWithValueVector[1]) * 60;
        }
        else if (parameterWithValueVector[0] == "max_only_walking_access_travel_time_ratio")
        {
          calculator.params.maxOnlyWalkingAccessTravelTimeRatio = std::stof(parameterWithValueVector[1]);
        }
        else if (parameterWithValueVector[0] == "alternatives_max_travel_time_ratio" || parameterWithValueVector[0] == "alt_max_ratio")
        {
          calculator.params.alternativesMaxTravelTimeRatio = std::stof(parameterWithValueVector[1]);
        }
        else if (parameterWithValueVector[0] == "alternatives_min_max_travel_time_minutes" || parameterWithValueVector[0] == "alt_min_max_travel_time")
        {
          calculator.params.minAlternativeMaxTravelTimeSeconds = std::stoi(parameterWithValueVector[1]) * 60;
        }
        else if (parameterWithValueVector[0] == "max_egress_travel_time" || parameterWithValueVector[0] == "max_egress_travel_time_minutes")
        {
          calculator.params.maxEgressWalkingTravelTimeSeconds = std::stoi(parameterWithValueVector[1]) * 60;
        }
        else if (parameterWithValueVector[0] == "max_transfer_travel_time" || parameterWithValueVector[0] == "max_transfer_travel_time_minutes")
        {
          calculator.params.maxTransferWalkingTravelTimeSeconds = std::stoi(parameterWithValueVector[1]) * 60;
        }
        else if (parameterWithValueVector[0] == "transfer_penalty" || parameterWithValueVector[0] == "transfer_penalty_minutes")
        {
          calculator.params.transferPenaltySeconds = std::stoi(parameterWithValueVector[1]) * 60;
        }
        else if (parameterWithValueVector[0] == "max_alternatives" || parameterWithValueVector[0] == "max_alt")
        {
          calculator.params.maxAlternatives = std::stoi(parameterWithValueVector[1]);
        }
        else if (parameterWithValueVector[0] == "alternatives" || parameterWithValueVector[0] == "alt")
        {
          if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1") { calculator.params.alternatives = true; }
        }
        else if (parameterWithValueVector[0] == "return_all_stops_results"
                 || parameterWithValueVector[0] == "return_all_stops_result"
                 || parameterWithValueVector[0] == "all_stops"
                )
        {
          if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1") { calculator.params.returnAllStopsResult = true; }
        }
        else if (parameterWithValueVector[0] == "transfer_only_at_same_station"
                 || parameterWithValueVector[0] == "transfer_only_at_station"
                )
        {
          if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1") { calculator.params.transferOnlyAtSameStation = true; }
        }
        else if (parameterWithValueVector[0] == "detailed"
                 || parameterWithValueVector[0] == "detailed_results"
                 || parameterWithValueVector[0] == "detailed_result"
                )
        {
          if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1") { calculator.params.detailedResults = true; }
        }
        else if (parameterWithValueVector[0] == "transfer_between_same_route"
                 || parameterWithValueVector[0] == "allow_same_route_transfer"
                 || parameterWithValueVector[0] == "transfers_between_same_route"
                 || parameterWithValueVector[0] == "allow_same_route_transfers"
                )
        {
          if (parameterWithValueVector[1] == "false" || parameterWithValueVector[1] == "0") { calculator.params.transferBetweenSameRoute = false; }
        }
        else if (parameterWithValueVector[0] == "reverse")
        {
          if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1") { calculator.params.forwardCalculation = false; }
        }
        
      }
            
      if (calculator.params.forwardCalculation)
      {
        calculator.params.departureTimeHour    = timeHour;
        calculator.params.departureTimeMinutes = timeMinute;
      }
      else
      {
        calculator.params.arrivalTimeHour    = timeHour;
        calculator.params.arrivalTimeMinutes = timeMinute;
      }
      
      //for (auto & ageGroup : odTripsAgeGroups)
      //{
      //  std::cerr << ageGroup << std::endl;
      //}
      
            
      if (calculator.params.alternatives)
      {
        RoutingResult routingResult;
        
        OdTrip odTrip;
        bool foundOdTrip{false};
        if (odTripId >= 0)
        {
          for (auto & _odTrip : calculator.odTrips)
          {
            if (_odTrip.id == odTripId)
            {
              odTrip      = _odTrip;
              foundOdTrip = true;
              break;
            }
          }
        }
        
        nlohmann::json json;
        nlohmann::json alternativeJson;
        
        if (foundOdTrip)
        {
          std::cout << "od trip id " << odTrip.id << std::endl;
          calculator.params.origin      = odTrip.origin;
          calculator.params.destination = odTrip.destination;
          calculator.params.odTrip      = &odTrip;
          json["odTripId"] = odTrip.id;
        }
        
        json["alternatives"] = nlohmann::json::array();
        std::vector<unsigned long long>                 foundRouteIds;
        std::vector< std::vector<unsigned long long> >  allCombinations;
        std::map<std::vector<unsigned long long>, bool> alreadyCalculatedCombinations;
        std::map<std::vector<unsigned long long>, bool> alreadyFoundRouteIds;
        std::map<std::vector<unsigned long long>, int>  foundRouteIdsTravelTimeSeconds;
        std::vector<int> combinationsKs;
        int maxTravelTime;
        int numAlternatives = 1;
        int maxAlternatives = calculator.params.maxAlternatives;
        int lastFoundedAtNum = 0;
        int departureTimeSeconds = -1;
        std::vector<std::string> routeShortnames;
        
        std:: cout << numAlternatives << "." << std::endl;
        std::cout << "initialMaxTotalTravelTimeSeconds: " << calculator.params.maxTotalTravelTimeSeconds << std::endl;
        
        routingResult = calculator.calculate();
        numAlternatives += 1;
        
        if (routingResult.status == "success")
        {
          routeShortnames.clear();
          for(auto routeId : routingResult.routeIds)
          {
            routeShortnames.push_back(calculator.routes[calculator.routeIndexesById[routeId]].shortname);
          }
          
          alternativeJson = {};
          alternativeJson["status"]                       = routingResult.status;
          alternativeJson["travelTimeSeconds"]            = routingResult.travelTimeSeconds;
          alternativeJson["minimizedTravelTimeSeconds"]   = routingResult.travelTimeSeconds - routingResult.firstWaitingTimeSeconds + calculator.params.minWaitingTimeSeconds;
          alternativeJson["departureTimeSeconds"]         = routingResult.departureTimeSeconds;
          alternativeJson["arrivalTimeSeconds"]           = routingResult.arrivalTimeSeconds;
          alternativeJson["numberOfTransfers"]            = routingResult.numberOfTransfers;
          alternativeJson["inVehicleTravelTimeSeconds"]   = routingResult.inVehicleTravelTimeSeconds;
          alternativeJson["transferTravelTimeSeconds"]    = routingResult.transferTravelTimeSeconds;
          alternativeJson["waitingTimeSeconds"]           = routingResult.waitingTimeSeconds;
          alternativeJson["accessTravelTimeSeconds"]      = routingResult.accessTravelTimeSeconds;
          alternativeJson["egressTravelTimeSeconds"]      = routingResult.egressTravelTimeSeconds;
          alternativeJson["transferWaitingTimeSeconds"]   = routingResult.transferWaitingTimeSeconds;
          alternativeJson["firstWaitingTimeSeconds"]      = routingResult.firstWaitingTimeSeconds;
          alternativeJson["nonTransitTravelTimeSeconds"]  = routingResult.nonTransitTravelTimeSeconds;
          alternativeJson["inVehicleTravelTimesSeconds"]  = routingResult.inVehicleTravelTimesSeconds;
          alternativeJson["routeIds"]                     = routingResult.routeIds;
          alternativeJson["routeShortnames"]              = routeShortnames;
          alternativeJson["routeTypeIds"]                 = routingResult.routeTypeIds;
          alternativeJson["agencyIds"]                    = routingResult.agencyIds;
          alternativeJson["boardingStopIds"]              = routingResult.boardingStopIds;
          alternativeJson["unboardingStopIds"]            = routingResult.unboardingStopIds;
          alternativeJson["tripIds"]                      = routingResult.tripIds;
          alternativeJson["alternativeSequence"]          = 1;
          json["alternatives"].push_back(alternativeJson);
          json["status"] = "success";
          departureTimeSeconds = routingResult.departureTimeSeconds + routingResult.firstWaitingTimeSeconds - calculator.params.minWaitingTimeSeconds;
                
          maxTravelTime = calculator.params.alternativesMaxTravelTimeRatio * routingResult.travelTimeSeconds;
          if (maxTravelTime < calculator.params.minAlternativeMaxTravelTimeSeconds)
          {
            calculator.params.maxTotalTravelTimeSeconds = calculator.params.minAlternativeMaxTravelTimeSeconds;
          }
          else if (maxTravelTime > routingResult.travelTimeSeconds + calculator.params.alternativesMaxAddedTravelTimeSeconds)
          {
            calculator.params.maxTotalTravelTimeSeconds = routingResult.travelTimeSeconds + calculator.params.alternativesMaxAddedTravelTimeSeconds;
          }
          else
          {
            calculator.params.maxTotalTravelTimeSeconds = maxTravelTime;
          }
          
          calculator.params.departureTimeSeconds = departureTimeSeconds;
          
          std::cout << "minAlternativeMaxTravelTimeSeconds: " << calculator.params.minAlternativeMaxTravelTimeSeconds << std::endl;
          std::cout << "alternativesMaxAddedTravelTimeSeconds: " << calculator.params.alternativesMaxAddedTravelTimeSeconds << std::endl;
          std::cout << "alternativesMaxTravelTimeRatio: " << calculator.params.alternativesMaxTravelTimeRatio << std::endl;
          std::cout << "fastestTravelTimeSeconds: " << routingResult.travelTimeSeconds << std::endl;
          std::cout << "maxTotalTravelTimeSeconds: " << calculator.params.maxTotalTravelTimeSeconds << std::endl;
          
          foundRouteIds = routingResult.routeIds;
          std::stable_sort(foundRouteIds.begin(),foundRouteIds.end());
          alreadyFoundRouteIds[foundRouteIds] = true;
          foundRouteIdsTravelTimeSeconds[foundRouteIds] = routingResult.travelTimeSeconds;
          lastFoundedAtNum = 1;
          //std::cout << "fastest route ids: ";
          //for (auto routeId : foundRouteIds)
          //{
          //  std::cout << calculator.routes[calculator.routeIndexesById[routeId]].shortname << " ";
          //}
          //std::cout << std::endl;
          combinationsKs.clear();
          for (int i = 1; i <= foundRouteIds.size(); i++) { combinationsKs.push_back(i); }
          for (auto k : combinationsKs)
          {
            Combinations<unsigned long long> combinations(foundRouteIds, k);
            //std::cout << "\nk = " << k << std::endl;
            for (auto newCombination : combinations)
            {
              std::stable_sort(newCombination.begin(), newCombination.end());
              allCombinations.push_back(newCombination);
              alreadyCalculatedCombinations[newCombination] = true;
            }
          }
          
          std::vector<unsigned long long> combination;
          for (int i = 0; i < allCombinations.size(); i++)
          {
            if (numAlternatives <= maxAlternatives)
            {
              //std:: cout << std::endl << numAlternatives << "." << std::endl;
              combination = allCombinations.at(i);
              calculator.params.exceptRouteIds = combination;
              //std::cout << "except route Ids: ";
              //for (auto routeId : combination)
              //{
              //  std::cout << calculator.routes[calculator.routeIndexesById[routeId]].shortname << " ";
              //}
              //std::cout << std::endl;
              routingResult = calculator.calculate(false);
              
              if (routingResult.status == "success")
              {
              
                foundRouteIds = routingResult.routeIds;
                std::stable_sort(foundRouteIds.begin(),foundRouteIds.end());
                
                if (foundRouteIds.size() > 0 && alreadyFoundRouteIds.count(foundRouteIds) == 0)
                {
                  routeShortnames.clear();
                  for(auto routeId : routingResult.routeIds)
                  {
                    routeShortnames.push_back(calculator.routes[calculator.routeIndexesById[routeId]].shortname);
                  }
                  
                  numAlternatives += 1;
                  
                  alternativeJson = {};
                  alternativeJson["status"]                       = routingResult.status;
                  alternativeJson["travelTimeSeconds"]            = routingResult.travelTimeSeconds;
                  alternativeJson["minimizedTravelTimeSeconds"]   = routingResult.travelTimeSeconds - routingResult.firstWaitingTimeSeconds + calculator.params.minWaitingTimeSeconds;
                  alternativeJson["departureTimeSeconds"]         = routingResult.departureTimeSeconds;
                  alternativeJson["arrivalTimeSeconds"]           = routingResult.arrivalTimeSeconds;
                  alternativeJson["numberOfTransfers"]            = routingResult.numberOfTransfers;
                  alternativeJson["inVehicleTravelTimeSeconds"]   = routingResult.inVehicleTravelTimeSeconds;
                  alternativeJson["transferTravelTimeSeconds"]    = routingResult.transferTravelTimeSeconds;
                  alternativeJson["waitingTimeSeconds"]           = routingResult.waitingTimeSeconds;
                  alternativeJson["accessTravelTimeSeconds"]      = routingResult.accessTravelTimeSeconds;
                  alternativeJson["egressTravelTimeSeconds"]      = routingResult.egressTravelTimeSeconds;
                  alternativeJson["transferWaitingTimeSeconds"]   = routingResult.transferWaitingTimeSeconds;
                  alternativeJson["firstWaitingTimeSeconds"]      = routingResult.firstWaitingTimeSeconds;
                  alternativeJson["nonTransitTravelTimeSeconds"]  = routingResult.nonTransitTravelTimeSeconds;
                  alternativeJson["inVehicleTravelTimesSeconds"]  = routingResult.inVehicleTravelTimesSeconds;
                  alternativeJson["routeIds"]                     = routingResult.routeIds;
                  alternativeJson["routeTypeIds"]                 = routingResult.routeTypeIds;
                  alternativeJson["routeShortnames"]              = routeShortnames;
                  alternativeJson["agencyIds"]                    = routingResult.agencyIds;
                  alternativeJson["boardingStopIds"]              = routingResult.boardingStopIds;
                  alternativeJson["unboardingStopIds"]            = routingResult.unboardingStopIds;
                  alternativeJson["tripIds"]                      = routingResult.tripIds;
                  alternativeJson["alternativeSequence"]          = numAlternatives;
                  json["alternatives"].push_back(alternativeJson);
                
                  //std::cout << "travelTimeSeconds: " << routingResult.travelTimeSeconds << " route Ids: ";
                  //for (auto routeId : foundRouteIds)
                  //{
                  //  std::cout << calculator.routes[calculator.routeIndexesById[routeId]].shortname << " ";
                  //}
                  //std::cout << std::endl;
                  combinationsKs.clear();
                  
                  lastFoundedAtNum = numAlternatives;
                  alreadyFoundRouteIds[foundRouteIds] = true;
                  foundRouteIdsTravelTimeSeconds[foundRouteIds] = routingResult.travelTimeSeconds;
                  for (int i = 1; i <= foundRouteIds.size(); i++) { combinationsKs.push_back(i); }
                  for (auto k : combinationsKs)
                  {
                    Combinations<unsigned long long> combinations(foundRouteIds, k);
                    //std::cout << "\nk = " << k << std::endl;
                    for (auto newCombination : combinations)
                    {
                      newCombination.insert( newCombination.end(), combination.begin(), combination.end() );
                      std::stable_sort(newCombination.begin(), newCombination.end());
                      if (alreadyCalculatedCombinations.count(newCombination) == 0)
                      {
                        allCombinations.push_back(newCombination);
                        alreadyCalculatedCombinations[newCombination] = true;
                      }
                    }
                  }
                  
                  combination.clear();
                }
              }
            }
          }
        }
        else
        {
          json["status"] = "failed";
        }
        
        std::cout << std::endl;
        int i {1};
        //for (auto foundRouteIds : alreadyFoundRouteIds)
        //{
        //  std::cout << i << ". ";
        //  for(auto routeId : foundRouteIds.first)
        //  {
        //    std::cout << calculator.routes[calculator.routeIndexesById[routeId]].shortname << " ";
        //  }
        //  std::cout << " tt: " << (foundRouteIdsTravelTimeSeconds[foundRouteIds.first] / 60);
        //  i++;
        //  std::cout << std::endl;
        //}
        
        std::cout << "last alternative found at: " << lastFoundedAtNum << " on a total of " << maxAlternatives << " calculations" << std::endl;
        
        //return 0;
        
        resultStr = json.dump(2);
      }
      
      
      if (!calculator.params.alternatives && (calculateAllOdTrips || odTripId >= 0))
      {
        RoutingResult routingResult;
        std::map<unsigned long long, std::map<int, float>> tripsLegsProfile; // parent map key: trip id, nested map key: connection sequence, value: number of trips using this connection
        std::map<unsigned long long, std::map<int, std::pair<float, std::vector<unsigned long long>>>> routePathsLegsProfile; // parent map key: trip id, nested map key: connection sequence, value: number of trips using this connection
        std::map<unsigned long long, float> routesOdTripsCount; // key: route id, value: count od trips using this route
        unsigned long long legTripId;
        unsigned long long legRouteId;
        unsigned long long legRoutePathId;
        bool atLeastOneOdTrip {false};
        bool atLeastOneCompatiblePeriod {false};
        bool attributesMatches {true};
        int odTripsCount = calculator.odTrips.size();
        std::string ageGroup;
        
        int legBoardingSequence;
        int legUnboardingSequence;
        
        nlohmann::json json;
        nlohmann::json odTripJson;
        nlohmann::json routesOdTripsCountJson;
        nlohmann::json routePathsOdTripsProfilesJson;
        nlohmann::json routePathsOdTripsProfilesSequenceJson;
        //std::vector<unsigned long long> routePathsOdTripsProfilesOdTripIds;
        
        if (fileFormat == "csv" && batchNumber == 1) // write header only on first batch, so we can easily append subsequent batches to the same csv file
        {
          // write csv header:
          csv += "id,status,ageGroup,gender,occupation,activity,mode,expansionFactor,travelTimeSeconds,onlyWalkingTravelTimeSeconds,"
                 "declaredDepartureTimeSeconds,departureTimeSeconds,arrivalTimeSeconds,numberOfTransfers,inVehicleTravelTimeSeconds,"
                 "transferTravelTimeSeconds,waitingTimeSeconds,accessTravelTimeSeconds,egressTravelTimeSeconds,transferWaitingTimeSeconds,"
                 "firstWaitingTimeSeconds,nonTransitTravelTimeSeconds,routeIds,routeTypeIds,agencyIds,boardingStopIds,unboardingStopIds,tripIds\n";
        }
        else
        {
          json["odTrips"] = nlohmann::json::array();
        }
                
        int i = 0;
        int j = 0;
        for (auto & odTrip : calculator.odTrips)
        {
          
          if ( i % batchesCount != batchNumber - 1) // when using multiple parallel calculators
          {
            i++;
            continue;
          }
          
          attributesMatches          = true;
          atLeastOneCompatiblePeriod = false;
          
          // verify that od trip matches selected attributes:
          if ( (odTripsAgeGroups.size()   > 0 && std::find(odTripsAgeGroups.begin(), odTripsAgeGroups.end(), odTrip.ageGroup)       == odTripsAgeGroups.end()) 
            || (odTripsGenders.size()     > 0 && std::find(odTripsGenders.begin(), odTripsGenders.end(), odTrip.gender)             == odTripsGenders.end())
            || (odTripsOccupations.size() > 0 && std::find(odTripsOccupations.begin(), odTripsOccupations.end(), odTrip.occupation) == odTripsOccupations.end())
            || (odTripsActivities.size()  > 0 && std::find(odTripsActivities.begin(), odTripsActivities.end(), odTrip.destinationActivity)     == odTripsActivities.end())
            || (odTripsModes.size()       > 0 && std::find(odTripsModes.begin(), odTripsModes.end(), odTrip.mode)                   == odTripsModes.end())
          )
          {
            attributesMatches = false;
          }

          // verify that od trip matches at least one selected period:
          for (auto & period : odTripsPeriods)
          {
            if (odTrip.departureTimeSeconds >= period.first && odTrip.departureTimeSeconds < period.second)
            {
              atLeastOneCompatiblePeriod = true;
            }
          }
          
          if (attributesMatches && (atLeastOneCompatiblePeriod || odTripsPeriods.size() == 0) && (odTripId == -1 || odTripId == odTrip.id) )
          {
            std::cout << "od trip id " << odTrip.id << " (" << (i+1) << "/" << odTripsCount << ")" << std::endl;// << " dts: " << odTrip.departureTimeSeconds << " atLeastOneCompatiblePeriod: " << (atLeastOneCompatiblePeriod ? "true " : "false ") << "attributesMatches: " << (attributesMatches ? "true " : "false ") << std::endl;
            
            calculator.params.origin = odTrip.origin;
            calculator.params.destination = odTrip.destination;
            calculator.params.odTrip = &odTrip;
            routingResult = calculator.calculate();
            if (true/*routingResult.status == "success"*/)
            {
              atLeastOneOdTrip = true;
              if (routingResult.legs.size() > 0)
              {
                if (fileFormat != "csv")
                {
                  for (auto & leg : routingResult.legs)
                  {
                    legTripId             = std::get<0>(leg);
                    legRouteId            = std::get<1>(leg);
                    legRoutePathId        = std::get<2>(leg);
                    legBoardingSequence   = std::get<3>(leg);
                    legUnboardingSequence = std::get<4>(leg);
                    if (routesOdTripsCount.find(legRouteId) == routesOdTripsCount.end())
                    {
                      routesOdTripsCount[legRouteId] = odTrip.expansionFactor;
                    }
                    else
                    {
                      routesOdTripsCount[legRouteId] += odTrip.expansionFactor;
                    }
                    if (tripsLegsProfile.find(legTripId) == tripsLegsProfile.end()) // initialize legs for this trip if not already set
                    {
                      tripsLegsProfile[legTripId] = std::map<int, float>();
                    }
                    if (routePathsLegsProfile.find(legRoutePathId) == routePathsLegsProfile.end()) // initialize legs for this trip if not already set
                    {
                      routePathsLegsProfile[legRoutePathId] = std::map<int, std::pair<float, std::vector<unsigned long long>>>();
                    }
                    for (int sequence = legBoardingSequence; sequence <= legUnboardingSequence; sequence++) // loop each connection sequence between boarding and unboarding sequences
                    {
                      // increment in trip profile:
                      if (tripsLegsProfile[legTripId].find(sequence) == tripsLegsProfile[legTripId].end())
                      {
                        tripsLegsProfile[legTripId][sequence] = odTrip.expansionFactor; // create the first od_trip for this connection
                      }
                      else
                      {
                        tripsLegsProfile[legTripId][sequence] += odTrip.expansionFactor; // increment od_trips for this connection
                      }
                      // increment in route path profile:
                      if (routePathsLegsProfile[legRoutePathId].find(sequence) == routePathsLegsProfile[legRoutePathId].end())
                      {
                        std::vector<unsigned long long> odTripIds;
                        odTripIds.push_back(odTrip.id);
                        routePathsLegsProfile[legRoutePathId][sequence] = std::make_pair(odTrip.expansionFactor, odTripIds); // create the first od_trip for this connection
                      }
                      else
                      {
                        std::get<0>(routePathsLegsProfile[legRoutePathId][sequence]) += odTrip.expansionFactor; // increment od_trips for this connection
                        std::get<1>(routePathsLegsProfile[legRoutePathId][sequence]).push_back(odTrip.id);
                      }
                    }
                  }
                }
              }
              
              if (fileFormat == "csv")
              {
                ageGroup = odTrip.ageGroup;
                std::replace( ageGroup.begin(), ageGroup.end(), '-', '_' ); // remove dash so Excel does not convert to age groups to numbers...
                csv += std::to_string(odTrip.id) + ",\"" + routingResult.status + "\",\"" + ageGroup + "\",\"" + odTrip.gender + "\",\"" + odTrip.occupation + "\",\"";
                csv += odTrip.destinationActivity + "\",\"" + odTrip.mode + "\"," + std::to_string(odTrip.expansionFactor) + "," + std::to_string(routingResult.travelTimeSeconds) + ",";
                csv += std::to_string(odTrip.walkingTravelTimeSeconds) + "," + std::to_string(odTrip.departureTimeSeconds) + "," + std::to_string(routingResult.departureTimeSeconds) + ",";
                csv += std::to_string(routingResult.arrivalTimeSeconds) + "," + std::to_string(routingResult.numberOfTransfers) + "," + std::to_string(routingResult.inVehicleTravelTimeSeconds) + ",";
                csv += std::to_string(routingResult.transferTravelTimeSeconds) + "," + std::to_string(routingResult.waitingTimeSeconds) + "," + std::to_string(routingResult.accessTravelTimeSeconds) + ",";
                csv += std::to_string(routingResult.egressTravelTimeSeconds) + "," + std::to_string(routingResult.transferWaitingTimeSeconds) + "," + std::to_string(routingResult.firstWaitingTimeSeconds) + ",";
                csv += std::to_string(routingResult.nonTransitTravelTimeSeconds) + ",";
                
                int countRouteIds = routingResult.routeIds.size();
                j = 0;
                for (auto & routeId : routingResult.routeIds)
                {
                  csv += std::to_string(routeId);
                  if (j < countRouteIds - 1)
                  {
                    csv += "|";
                  }
                  j++;
                }
                csv += ",";
                j = 0;
                for (auto & routeTypeId : routingResult.routeTypeIds)
                {
                  csv += std::to_string(routeTypeId);
                  if (j < countRouteIds - 1)
                  {
                    csv += "|";
                  }
                  j++;
                }
                csv += ",";
                j = 0;
                for (auto & agencyId : routingResult.agencyIds)
                {
                  csv += std::to_string(agencyId);
                  if (j < countRouteIds - 1)
                  {
                    csv += "|";
                  }
                  j++;
                }
                csv += ",";
                j = 0;
                for (auto & boardingStopId : routingResult.boardingStopIds)
                {
                  csv += std::to_string(boardingStopId);
                  if (j < countRouteIds - 1)
                  {
                    csv += "|";
                  }
                  j++;
                }
                csv += ",";
                j = 0;
                for (auto & unboardingStopId : routingResult.unboardingStopIds)
                {
                  csv += std::to_string(unboardingStopId);
                  if (j < countRouteIds - 1)
                  {
                    csv += "|";
                  }
                  j++;
                }
                csv += ",";
                j = 0;
                for (auto & tripId : routingResult.tripIds)
                {
                  csv += std::to_string(tripId);
                  if (j < countRouteIds - 1)
                  {
                    csv += "|";
                  }
                  j++;
                }
                csv += "\n";
              }
              else
              {
                odTripJson = {};
                odTripJson["id"]                           = odTrip.id;
                odTripJson["status"]                       = routingResult.status;
                odTripJson["ageGroup"]                     = odTrip.ageGroup;
                odTripJson["gender"]                       = odTrip.gender;
                odTripJson["occupation"]                   = odTrip.occupation;
                odTripJson["activity"]                     = odTrip.destinationActivity;
                odTripJson["mode"]                         = odTrip.mode;
                odTripJson["expansionFactor"]              = odTrip.expansionFactor;
                odTripJson["travelTimeSeconds"]            = routingResult.travelTimeSeconds;
                odTripJson["minimizedTravelTimeSeconds"]   = routingResult.travelTimeSeconds - routingResult.firstWaitingTimeSeconds + calculator.params.minWaitingTimeSeconds;
                odTripJson["onlyWalkingTravelTimeSeconds"] = odTrip.walkingTravelTimeSeconds;
                odTripJson["declaredDepartureTimeSeconds"] = odTrip.departureTimeSeconds;
                odTripJson["departureTimeSeconds"]         = routingResult.departureTimeSeconds;
                odTripJson["arrivalTimeSeconds"]           = routingResult.arrivalTimeSeconds;
                odTripJson["numberOfTransfers"]            = routingResult.numberOfTransfers;
                odTripJson["inVehicleTravelTimeSeconds"]   = routingResult.inVehicleTravelTimeSeconds;
                odTripJson["transferTravelTimeSeconds"]    = routingResult.transferTravelTimeSeconds;
                odTripJson["waitingTimeSeconds"]           = routingResult.waitingTimeSeconds;
                odTripJson["accessTravelTimeSeconds"]      = routingResult.accessTravelTimeSeconds;
                odTripJson["egressTravelTimeSeconds"]      = routingResult.egressTravelTimeSeconds;
                odTripJson["transferWaitingTimeSeconds"]   = routingResult.transferWaitingTimeSeconds;
                odTripJson["firstWaitingTimeSeconds"]      = routingResult.firstWaitingTimeSeconds;
                odTripJson["nonTransitTravelTimeSeconds"]  = routingResult.nonTransitTravelTimeSeconds;
                odTripJson["routeIds"]                     = routingResult.routeIds;
                odTripJson["routeTypeIds"]                 = routingResult.routeTypeIds;
                odTripJson["agencyIds"]                    = routingResult.agencyIds;
                odTripJson["boardingStopIds"]              = routingResult.boardingStopIds;
                odTripJson["unboardingStopIds"]            = routingResult.unboardingStopIds;
                odTripJson["tripIds"]                      = routingResult.tripIds;
                json["odTrips"].push_back(odTripJson);
              }
            }
          }
          i++;
          if (odTripsSampleSize >= 0 && i >= odTripsSampleSize)
          {
            break;
          }
        }
        if (fileFormat != "csv")
        {
          routesOdTripsCountJson = {};
          for (auto & routeCount : routesOdTripsCount)
          {
            routesOdTripsCountJson[std::to_string(routeCount.first)] = routeCount.second;
          }
          json["routesOdTripsCount"] = routesOdTripsCountJson;
          
          routePathsOdTripsProfilesJson = {};
          for (auto & routePathProfile : routePathsLegsProfile)
          {
            routePathsOdTripsProfilesSequenceJson = {};
            for (auto & sequenceProfile : routePathProfile.second)
            {
              //routePathsOdTripsProfilesOdTripIds.clear();
              //for (auto & odTripId : std::get<1>(sequenceProfile.second))
              //{
              //  routePathsOdTripsProfilesOdTripIds.push_back()
              //}
              routePathsOdTripsProfilesSequenceJson[std::to_string(sequenceProfile.first)] = {{"demand", std::get<0>(sequenceProfile.second)}, {"odTripIds", std::get<1>(sequenceProfile.second)}};
            }
            routePathsOdTripsProfilesJson[std::to_string(routePathProfile.first)] = routePathsOdTripsProfilesSequenceJson;
          }
          json["routePathsOdTripsProfiles"] = routePathsOdTripsProfilesJson;
          resultStr = json.dump(2);
        }
        if (calculateAllOdTrips && fileFormat == "csv")
        {
          
          
          if (saveToFile)
          {
            calculationName += "__batch_" + std::to_string(batchNumber) + "_of_" + std::to_string(batchesCount);
            std::cerr << "writing csv file" << std::endl;
            std::ofstream csvFile;
            //csvFile.imbue(std::locale("en_US.UTF8"));
            csvFile.open(calculationName + ".csv", std::ios_base::trunc);
            csvFile << csv;
            csvFile.close();
            //resultStr = "{\"status\": \"success\", \"format\": \"csv\", \"filename\": \"" + calculationName + ".csv\"}";
          }
        }
      }
      else if (!calculator.params.alternatives)
      {
        resultStr = calculator.calculate().json;
      }

      //calculator.algorithmCalculationTime.stopStep();
      
      //std::cout << "-- parsing request -- " << calculator.algorithmCalculationTime.getStepDurationMicroseconds() << " microseconds\n";
      
      //calculator.reset();
      
      //calculator.algorithmCalculationTime.stopStep();
      //calculator.algorithmCalculationTime.startStep();
      
      //calculator.resetAccessEgressModes();
      
      //calculator.algorithmCalculationTime.stopStep();
      
      //std::cout << "-- reset access egress modes -- " << calculator.algorithmCalculationTime.getStepDurationMicroseconds() << " microseconds\n";
      
      
      //calculator.algorithmCalculationTime.startStep();
      if (saveToFile && fileFormat != "csv")
      {
        std::cerr << "writing json file" << std::endl;
        std::ofstream jsonFile;
        //jsonFile.imbue(std::locale("en_US.UTF8"));
        jsonFile.open(calculationName + ".json", std::ios_base::trunc);
        jsonFile << resultStr;
        jsonFile.close();
      }
      
    }
    else
    {
      resultStr = "{\"status\": \"failed\", \"error\": \"Wrong or malformed query\"}";
    }
    
    std::cerr << "-- total -- " << calculator.algorithmCalculationTime.getDurationMicrosecondsNoStop() << " microseconds\n";
    
    //calculator.algorithmCalculationTime.stop();
      
    //std::cerr << "-- calculation time -- " << calculator.algorithmCalculationTime.getDurationMicroseconds() << " microseconds\n";
    
    if (fileFormat == "csv")
    {
      *response << "HTTP/1.1 200 OK\r\nContent-Type: application/csv; charset=utf-8\r\nContent-Length: " << csv.length() << "\r\n\r\n" << csv;
    }
    else
    {
      *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << resultStr.length() << "\r\n\r\n" << resultStr;
    }
    
  };
  
  
  
  
  std::cout << "starting server..." << std::endl;
  //server.start();
  std::thread server_thread([&server](){
      //Start server
      server.start();
  });
    
  //Wait for server to start so that the client can connect
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  
  std::cout << "ready." << std::endl;
  
  server_thread.join();
  
  //calculator.destroy();

  std::cout << "done..." << std::endl;
  
  return 0;
}

void default_resource_send(const HttpServer &server, const std::shared_ptr<HttpServer::Response> &response,
                           const std::shared_ptr<std::ifstream> &ifs) {
    //read and send 128 KB at a time
    static std::vector<char> buffer(131072); // Safe when server is running on one thread
    std::streamsize read_length;
    if((read_length=ifs->read(&buffer[0], buffer.size()).gcount())>0) {
        response->write(&buffer[0], read_length);
        if(read_length==static_cast<std::streamsize>(buffer.size())) {
            server.send(response, [&server, response, ifs](const boost::system::error_code &ec) {
                if(!ec)
                    default_resource_send(server, response, ifs);
                else
                    std::cerr << "Connection interrupted" << std::endl;
            });
        }
    }
}
