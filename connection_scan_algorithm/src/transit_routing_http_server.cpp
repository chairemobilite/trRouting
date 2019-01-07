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
#include <boost/algorithm/string/replace.hpp>

#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include <iostream>
#include <iterator>
#include <curses.h>
#include <locale>

#include <osrm/osrm.hpp>
#include <osrm/engine_config.hpp>
#include <osrm/table_parameters.hpp>

#include "toolbox.hpp"
//#include "gtfs_fetcher.hpp"
//#include "csv_fetcher.hpp"
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
      ("dataFetcher,data", boost::program_options::value<std::string>(), "data fetcher (csv, gtfs or cache)"); // only cache implemented for now
  optionsDesc.add_options() 
      ("dataShortname,sn", boost::program_options::value<std::string>(), "data shortname (shortname of the application to use or data to use)");
  optionsDesc.add_options() 
      ("osrmWalkPort",     boost::program_options::value<std::string>(), "osrm walking port");
  optionsDesc.add_options() 
      ("osrmFilePath",     boost::program_options::value<std::string>(), "osrm file path (PATH_TO_ROUTING_FILE.osrm)");
  optionsDesc.add_options() 
      ("osrmUseLib",       boost::program_options::value<std::string>(), "osrm use libosrm instead of server (1 or 0)");
  optionsDesc.add_options() 
      ("odTripsFootpathsMaxTravelTimeMinutes", boost::program_options::value<int>(), "max travel time to use when fetching od_trips footpaths to access and egress nodes");
  optionsDesc.add_options() 
      ("updateOdTrips", boost::program_options::value<std::string>(), "update od trips access and egress nodes or not (1 or 0)");
  
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
  if(variablesMap.count("osrmFilePath") == 1)
  {
    algorithmParams.osrmFilePath = variablesMap["osrmFilePath"].as<std::string>();
  }
  if(variablesMap.count("osrmUseLib") == 1)
  {
    algorithmParams.osrmUseLib = (variablesMap["osrmUseLib"].as<std::string>() == "1") ? true : false;
  }
  if(variablesMap.count("odTripsFootpathsMaxTravelTimeMinutes") == 1)
  {
    algorithmParams.maxAccessWalkingTravelTimeSeconds = variablesMap["odTripsFootpathsMaxTravelTimeMinutes"].as<int>() * 60;
    algorithmParams.maxEgressWalkingTravelTimeSeconds = algorithmParams.maxAccessWalkingTravelTimeSeconds;
    std::cerr << "Max access/egress travel time seconds: " << algorithmParams.maxAccessWalkingTravelTimeSeconds << std::endl;
  }
  if(variablesMap.count("updateOdTrips") == 1)
  {
    algorithmParams.updateOdTrips = (variablesMap["updateOdTrips"].as<std::string>() == "1") ? true : false;
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
  
  algorithmParams.applicationShortname = dataShortname;
  algorithmParams.dataFetcherShortname = dataFetcherStr;
  
  /*GtfsFetcher gtfsFetcher         = GtfsFetcher();
  algorithmParams.gtfsFetcher     = &gtfsFetcher;
  CsvFetcher csvFetcher           = CsvFetcher();
  algorithmParams.csvFetcher      = &csvFetcher;*/
  CacheFetcher cacheFetcher       = CacheFetcher();
  algorithmParams.cacheFetcher    = &cacheFetcher;
  
  Calculator calculator(algorithmParams);
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
      long long startingNodeUuid{-1}, endingNodeUuid{-1};
      std::vector<int> onlyServiceUuids;
      std::vector<unsigned long long> exceptServiceUuids;
      std::vector<unsigned long long> onlyLineUuids;
      std::vector<unsigned long long> exceptLineUuids;
      std::vector<unsigned long long> onlyModes;
      std::vector<unsigned long long> exceptModes;
      std::vector<unsigned long long> onlyAgencyUuids;
      std::vector<unsigned long long> exceptAgencyUuids;
      std::vector<unsigned long long> accessNodeUuids;
      std::vector<unsigned long long> egressNodeUUids;
      std::vector<int> accessNodeTravelTimesSeconds;
      std::vector<int> egressNodeTravelTimesSeconds;
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
      int odTripUuid {""}; // when calculating for only one trip
      bool alternatives {false}; // calculate alternatives or not
      
      calculator.params.onlyServiceUuids     = onlyServiceUuids;
      calculator.params.exceptServiceUuids   = exceptServiceUuids;
      calculator.params.onlyLineUuids       = onlyLineUuids;
      calculator.params.exceptLineUuids     = exceptLineUuids;
      calculator.params.onlyLineTypeUuids   = onlyModes;
      calculator.params.exceptLineTypeUuids = exceptModes;
      calculator.params.onlyAgencyUuids      = onlyAgencyUuids;
      calculator.params.exceptAgencyUuids    = exceptAgencyUuids;
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
      std::vector<std::string> onlyServiceUuidsVector;
      std::vector<std::string> exceptServiceUuidsVector;
      std::vector<std::string> onlyLineUuidsVector;
      std::vector<std::string> exceptLineUuidsVector;
      std::vector<std::string> onlyModesVector;
      std::vector<std::string> exceptModesVector;
      std::vector<std::string> onlyAgencyUuidsVector;
      std::vector<std::string> exceptAgencyUuidsVector;
      std::vector<std::string> accessNodeUuidsVector;
      std::vector<std::string> accessNodeTravelTimesSecondsVector;
      std::vector<std::string> egressNodeUuidsVector;
      std::vector<std::string> egressNodeTravelTimesSecondsVector;
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
      calculator.params.returnAllNodesResult                   = false;
      calculator.params.transferOnlyAtSameStation              = false;
      calculator.params.transferBetweenSameLine               = true;
      calculator.params.origin                                 = Point();
      calculator.params.destination                            = Point();
      calculator.params.routingDateYear                        = 0;
      calculator.params.routingDateMonth                       = 0;
      calculator.params.routingDateDay                         = 0;
      calculator.params.originNodeUuid                         = -1;
      calculator.params.destinationNodeUuid                    = -1;
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
      calculator.params.accessNodeUuids.clear();
      calculator.params.egressNodeUuids.clear();
      calculator.params.accessNodeTravelTimesSeconds.clear();
      calculator.params.egressNodeTravelTimesSeconds.clear();

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
        else if (parameterWithValueVector[0] == "access_node_uuids")
        {
          boost::split(accessNodeUuidsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string accessNodeUuid : accessNodeUuidsVector)
          {
            accessNodeUuids.push_back(accessNodeUuid);
          }
          calculator.params.accessNodeUuids = accessNodeUuids;
        }
        else if (parameterWithValueVector[0] == "egress_node_uuids")
        {
          boost::split(egressNodeUuidsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string egressNodeuid : egressNodeUuidsVector)
          {
            egressNodeUuids.push_back(egressNodeUuid);
          }
          calculator.params.egressNodeuids = egressNodeUuids;
        }
        else if (parameterWithValueVector[0] == "od_trip_uuid")
        {
          odTripUuid = parameterWithValueVector[1];
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
            if (odTripsAgeGroup.find("plus") != std::string::npos) // replace plus by + (+ cannot be used in query string)
            {
              odTripsAgeGroups.push_back(boost::replace_all_copy(odTripsAgeGroup, "plus","+"));
            }
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
        
        else if (parameterWithValueVector[0] == "access_node_travel_times_seconds" || parameterWithValueVector[0] == "access_node_travel_times")
        {
          boost::split(accessNodeTravelTimesSecondsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string accessNodeTravelTimeSeconds : accessNodeTravelTimesSecondsVector)
          {
            accessNodeTravelTimesSeconds.push_back(std::stoi(accessNodeTravelTimeSeconds));
          }
          calculator.params.accessNodeTravelTimesSeconds = accessNodeTravelTimesSeconds;
        }
        else if (parameterWithValueVector[0] == "egress_node_travel_times_seconds" || parameterWithValueVector[0] == "egress_node_travel_times")
        {
          boost::split(egressNodeTravelTimesSecondsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string egressNodeTravelTimeSeconds : egressNodeTravelTimesSecondsVector)
          {
            egressNodeTravelTimesSeconds.push_back(std::stoi(egressNodeTravelTimeSeconds));
          }
          calculator.params.egressNodeTravelTimesSeconds = egressNodeTravelTimesSeconds;
        }
        else if (parameterWithValueVector[0] == "only_service_uuids")
        {
          boost::split(onlyServiceUuidsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string onlyServiceUuid : onlyServiceUuidsVector)
          {
            onlyServiceUuids.push_back(onlyServiceUuid);
          }
          calculator.params.onlyServiceUuids = onlyServiceUuids;
        }
        else if (parameterWithValueVector[0] == "except_service_uuids")
        {
          boost::split(exceptServiceUuidsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string exceptServiceUuid : exceptServiceUuidsVector)
          {
            exceptServiceUuids.push_back(exceptServiceUuid);
          }
          calculator.params.exceptServiceUuids = exceptServiceUuids;
        }
        else if (parameterWithValueVector[0] == "only_line_uuids")
        {
          boost::split(onlyLineUuidsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string onlyLineUuid : onlyLineUuidsVector)
          {
            onlyLineUuids.push_back(std::stoll(onlyLineUuid));
          }
          calculator.params.onlyLineUuids = onlyLineUuids;
        }
        else if (parameterWithValueVector[0] == "except_line_uuids")
        {
          boost::split(exceptLineUuidsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string exceptLineUuid : exceptLineUuidsVector)
          {
            exceptLineUuids.push_back(std::stoll(exceptLineUuid));
          }
          calculator.params.exceptLineUuids = exceptLineUuids;
        }
        else if (parameterWithValueVector[0] == "only_modes")
        {
          boost::split(onlyModesVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string onlyMode : onlyModesVector)
          {
            onlyModes.push_back(onlyModes);
          }
          calculator.params.onlyModes = onlyModes;
        }
        else if (parameterWithValueVector[0] == "except_modes")
        {
          boost::split(exceptModesVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string exceptMode : exceptModesVector)
          {
            exceptModes.push_back(exceptModes);
          }
          calculator.params.exceptModes = exceptModes;
        }
        else if (parameterWithValueVector[0] == "only_agency_uuids")
        {
          boost::split(onlyAgencyUuidsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string onlyAgencyUuid : onlyAgencyUuidsVector)
          {
            onlyAgencyUuids.push_back(onlyAgencyUuid);
          }
          calculator.params.onlyAgencyUuids = onlyAgencyUuids;
        }
        else if (parameterWithValueVector[0] == "except_agency_uuids")
        {
          boost::split(exceptAgencyUuidsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string exceptAgencyUuid : exceptAgencyUuidsVector)
          {
            exceptAgencyUuids.push_back(exceptAgencyUuid);
          }
          calculator.params.exceptAgencyUuids = exceptAgencyUuids;
        }
        else if (parameterWithValueVector[0] == "starting_node_uuid"
                 || parameterWithValueVector[0] == "start_node_uuid"
                 || parameterWithValueVector[0] == "origin_node_uuid"
                )
        {
          calculator.params.originNodeUuid = std::stoll(parameterWithValueVector[1]);
        }
        else if (parameterWithValueVector[0] == "ending_node_uuid"
                 || parameterWithValueVector[0] == "end_node_uuid"
                 || parameterWithValueVector[0] == "destination_node_uuid"
                )
        {
          calculator.params.destinationNodeUuid = std::stoll(parameterWithValueVector[1]);
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
        else if (parameterWithValueVector[0] == "return_all_nodes_results"
                 || parameterWithValueVector[0] == "return_all_nodes_result"
                 || parameterWithValueVector[0] == "all_nodes"
                )
        {
          if (parameterWithValueVector[1] == "true" || parameterWithValueVector[1] == "1") { calculator.params.returnAllNodesResult = true; }
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
        else if (parameterWithValueVector[0] == "transfer_between_same_line"
                 || parameterWithValueVector[0] == "allow_same_line_transfer"
                 || parameterWithValueVector[0] == "transfers_between_same_line"
                 || parameterWithValueVector[0] == "allow_same_line_transfers"
                )
        {
          if (parameterWithValueVector[1] == "false" || parameterWithValueVector[1] == "0") { calculator.params.transferBetweenSameLine = false; }
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
        if (odTripUuid >= 0)
        {
          for (auto & _odTrip : calculator.odTrips)
          {
            if (_odTrip.uuid == odTripUuid)
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
          std::cout << "od trip uuid " << odTrip.uuid << std::endl;
          calculator.params.origin      = odTrip.origin;
          calculator.params.destination = odTrip.destination;
          calculator.params.odTrip      = &odTrip;
          json["odTripUuid"] = odTrip.uuid;
        }
        
        json["alternatives"] = nlohmann::json::array();
        std::vector<unsigned long long>                 foundLineUuids;
        std::vector< std::vector<unsigned long long> >  allCombinations;
        std::map<std::vector<unsigned long long>, bool> alreadyCalculatedCombinations;
        std::map<std::vector<unsigned long long>, bool> alreadyFoundLineUuids;
        std::map<std::vector<unsigned long long>, int>  foundLineUuidsTravelTimeSeconds;
        std::vector<int> combinationsKs;
        int maxTravelTime;
        int numAlternatives = 1;
        int maxAlternatives = calculator.params.maxAlternatives;
        int lastFoundedAtNum = 0;
        int departureTimeSeconds = -1;
        std::vector<std::string> lineShortnames;
        
        std:: cout << numAlternatives << "." << std::endl;
        std::cout << "initialMaxTotalTravelTimeSeconds: " << calculator.params.maxTotalTravelTimeSeconds << std::endl;
        
        routingResult = calculator.calculate();
        
        if (routingResult.status == "success")
        {
          lineShortnames.clear();
          for(auto lineUuid : routingResult.lineUuids)
          {
            lineShortnames.push_back(calculator.lines[calculator.lineIndexesByUuid[lineUuid]].shortname);
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
          alternativeJson["lineUuids"]                    = routingResult.lineUuids;
          alternativeJson["lineShortnames"]               = lineShortnames;
          alternativeJson["lineTypeUuids"]                = routingResult.modes;
          alternativeJson["agencyUuids"]                  = routingResult.agencyUuids;
          alternativeJson["boardingNodeUuids"]            = routingResult.boardingNodeUuids;
          alternativeJson["unboardingNodeUuids"]          = routingResult.unboardingNodeUuids;
          alternativeJson["tripUuids"]                    = routingResult.tripUuids;
          alternativeJson["alternativeSequence"]          = numAlternatives;
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
          
          foundLineUuids = routingResult.lineUuids;
          std::stable_sort(foundLineUuids.begin(),foundLineUuids.end());
          alreadyFoundLineUuids[foundLineUuids] = true;
          foundLineUuidsTravelTimeSeconds[foundLineUuids] = routingResult.travelTimeSeconds;
          lastFoundedAtNum = 1;
          //std::cout << "fastest line ids: ";
          //for (auto lineUuid : foundLineUuids)
          //{
          //  std::cout << calculator.lines[calculator.lineIndexesByUuid[lineUuid]].shortname << " ";
          //}
          //std::cout << std::endl;
          combinationsKs.clear();
          for (int i = 1; i <= foundLineUuids.size(); i++) { combinationsKs.push_back(i); }
          for (auto k : combinationsKs)
          {
            Combinations<unsigned long long> combinations(foundLineUuids, k);
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
              calculator.params.exceptLineUuids = combination;
              //std::cout << "except line Uuids: ";
              //for (auto lineUuid : combination)
              //{
              //  std::cout << calculator.lines[calculator.lineIndexesByUuid[lineUuid]].shortname << " ";
              //}
              //std::cout << std::endl;
              routingResult = calculator.calculate(false);
              
              if (routingResult.status == "success")
              {
              
                foundLineUuids = routingResult.lineUuids;
                std::stable_sort(foundLineUuids.begin(),foundLineUuids.end());
                
                if (foundLineUuids.size() > 0 && alreadyFoundLineUuids.count(foundLineUuids) == 0)
                {
                  lineShortnames.clear();
                  for(auto lineUuid : routingResult.lineUuids)
                  {
                    lineShortnames.push_back(calculator.lines[calculator.lineIndexesByUuid[lineUuid]].shortname);
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
                  alternativeJson["lineUuids"]                    = routingResult.lineUuids;
                  alternativeJson["lineTypeUuids"]                = routingResult.lineTypeUuids;
                  alternativeJson["lineShortnames"]               = lineShortnames;
                  alternativeJson["agencyUuids"]                  = routingResult.agencyUuids;
                  alternativeJson["boardingNodeUuids"]            = routingResult.boardingNodeUuids;
                  alternativeJson["unboardingNodeUuids"]          = routingResult.unboardingNodeUuids;
                  alternativeJson["tripUuids"]                    = routingResult.tripUuids;
                  alternativeJson["alternativeSequence"]          = numAlternatives;
                  json["alternatives"].push_back(alternativeJson);
                
                  //std::cout << "travelTimeSeconds: " << routingResult.travelTimeSeconds << " line Uuids: ";
                  //for (auto lineUuid : foundLineUuids)
                  //{
                  //  std::cout << calculator.lines[calculator.lineIndexesByUuid[lineUuid]].shortname << " ";
                  //}
                  //std::cout << std::endl;
                  combinationsKs.clear();
                  
                  lastFoundedAtNum = numAlternatives;
                  alreadyFoundLineUuids[foundLineUuids] = true;
                  foundLineUuidsTravelTimeSeconds[foundLineUuid] = routingResult.travelTimeSeconds;
                  for (int i = 1; i <= foundLineUuids.size(); i++) { combinationsKs.push_back(i); }
                  for (auto k : combinationsKs)
                  {
                    Combinations<unsigned long long> combinations(foundLineUuids, k);
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
        //for (auto foundLineUuids : alreadyFoundLineUuids)
        //{
        //  std::cout << i << ". ";
        //  for(auto lineUuid : foundLineUuids.first)
        //  {
        //    std::cout << calculator.lines[calculator.lineIndexesByUuid[lineUuid]].shortname << " ";
        //  }
        //  std::cout << " tt: " << (foundLineUuidsTravelTimeSeconds[foundLineUuids.first] / 60);
        //  i++;
        //  std::cout << std::endl;
        //}
        
        std::cout << "last alternative found at: " << lastFoundedAtNum << " on a total of " << maxAlternatives << " calculations" << std::endl;
        
        //return 0;
        
        resultStr = json.dump(2);
      }
      
      
      if (!calculator.params.alternatives && (calculateAllOdTrips || odTripUuid !== "" ))
      {
        RoutingResult routingResult;
        std::map<unsigned long long, std::map<int, float>> tripsLegsProfile; // parent map key: trip id, nested map key: connection sequence, value: number of trips using this connection
        std::map<unsigned long long, std::map<int, std::pair<float, std::vector<unsigned long long>>>> pathsLegsProfile; // parent map key: trip id, nested map key: connection sequence, value: number of trips using this connection
        std::map<unsigned long long, float> linesOdTripsCount; // key: line id, value: count od trips using this line
        unsigned long long legTripUuid;
        unsigned long long legLineUuid;
        unsigned long long legPathUuid;
        bool atLeastOneOdTrip {false};
        bool atLeastOneCompatiblePeriod {false};
        bool attributesMatches {true};
        int odTripsCount = calculator.odTrips.size();
        std::string ageGroup;
        
        int legBoardingSequence;
        int legUnboardingSequence;
        
        nlohmann::json json;
        nlohmann::json odTripJson;
        nlohmann::json linesOdTripsCountJson;
        nlohmann::json pathsOdTripsProfilesJson;
        nlohmann::json pathsOdTripsProfilesSequenceJson;
        //std::vector<unsigned long long> pathsOdTripsProfilesOdTripUuids;
        
        if (fileFormat == "csv" && batchNumber == 1) // write header only on first batch, so we can easily append subsequent batches to the same csv file
        {
          // write csv header:
          csv += "uuid,status,ageGroup,gender,occupation,activity,mode,expansionFactor,travelTimeSeconds,onlyWalkingTravelTimeSeconds,"
                 "declaredDepartureTimeSeconds,departureTimeSeconds,arrivalTimeSeconds,numberOfTransfers,inVehicleTravelTimeSeconds,"
                 "transferTravelTimeSeconds,waitingTimeSeconds,accessTravelTimeSeconds,egressTravelTimeSeconds,transferWaitingTimeSeconds,"
                 "firstWaitingTimeSeconds,nonTransitTravelTimeSeconds,lineUuids,lineTypeUuids,agencyUuids,boardingNodeUuids,unboardingNodeUuids,tripUuids\n";
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
          if ( (odTripsAgeGroups.size()   > 0 && std::find(odTripsAgeGroups.begin(), odTripsAgeGroups.end(), odTrip.ageGroup)              == odTripsAgeGroups.end()) 
            || (odTripsGenders.size()     > 0 && std::find(odTripsGenders.begin(), odTripsGenders.end(), odTrip.gender)                    == odTripsGenders.end())
            || (odTripsOccupations.size() > 0 && std::find(odTripsOccupations.begin(), odTripsOccupations.end(), odTrip.occupation)        == odTripsOccupations.end())
            || (odTripsActivities.size()  > 0 && std::find(odTripsActivities.begin(), odTripsActivities.end(), odTrip.destinationActivity) == odTripsActivities.end())
            || (odTripsModes.size()       > 0 && std::find(odTripsModes.begin(), odTripsModes.end(), odTrip.mode)                          == odTripsModes.end())
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
          
          if (attributesMatches && (atLeastOneCompatiblePeriod || odTripsPeriods.size() == 0) && (odTripUuid == "" || odTripUuid == odTrip.uuid) )
          {
            std::cout << "od trip uuid " << odTrip.uuid << " (" << (i+1) << "/" << odTripsCount << ")" << std::endl;// << " dts: " << odTrip.departureTimeSeconds << " atLeastOneCompatiblePeriod: " << (atLeastOneCompatiblePeriod ? "true " : "false ") << "attributesMatches: " << (attributesMatches ? "true " : "false ") << std::endl;
            
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
                    legTripUuid           = std::get<0>(leg);
                    legLineUuid           = std::get<1>(leg);
                    legPathUuid           = std::get<2>(leg);
                    legBoardingSequence   = std::get<3>(leg);
                    legUnboardingSequence = std::get<4>(leg);
                    if (linesOdTripsCount.find(legLineUuid) == linesOdTripsCount.end())
                    {
                      linesOdTripsCount[legLineUuid] = odTrip.expansionFactor;
                    }
                    else
                    {
                      linesOdTripsCount[legLineUuid] += odTrip.expansionFactor;
                    }
                    if (tripsLegsProfile.find(legTripUuid) == tripsLegsProfile.end()) // initialize legs for this trip if not already set
                    {
                      tripsLegsProfile[legTripUuid] = std::map<int, float>();
                    }
                    if (pathsLegsProfile.find(legPathUuid) == pathsLegsProfile.end()) // initialize legs for this trip if not already set
                    {
                      pathsLegsProfile[legPathUuid] = std::map<int, std::pair<float, std::vector<unsigned long long>>>();
                    }
                    for (int sequence = legBoardingSequence; sequence <= legUnboardingSequence; sequence++) // loop each connection sequence between boarding and unboarding sequences
                    {
                      // increment in trip profile:
                      if (tripsLegsProfile[legTripUuid].find(sequence) == tripsLegsProfile[legTripUuid].end())
                      {
                        tripsLegsProfile[legTripUuid][sequence] = odTrip.expansionFactor; // create the first od_trip for this connection
                      }
                      else
                      {
                        tripsLegsProfile[legTripUuid][sequence] += odTrip.expansionFactor; // increment od_trips for this connection
                      }
                      // increment in line path profile:
                      if (pathsLegsProfile[legPathUuid].find(sequence) == pathsLegsProfile[legPathUuid].end())
                      {
                        std::vector<unsigned long long> odTripUuids;
                        odTripUuids.push_back(odTrip.uuid);
                        pathsLegsProfile[legPathUuid][sequence] = std::make_pair(odTrip.expansionFactor, odTripUuids); // create the first od_trip for this connection
                      }
                      else
                      {
                        std::get<0>(pathsLegsProfile[legPathUuid][sequence]) += odTrip.expansionFactor; // increment od_trips for this connection
                        std::get<1>(pathsLegsProfile[legPathUuid][sequence]).push_back(odTrip.uuid);
                      }
                    }
                  }
                }
              }
              
              if (fileFormat == "csv")
              {
                ageGroup = odTrip.ageGroup;
                std::replace( ageGroup.begin(), ageGroup.end(), '-', '_' ); // remove dash so Excel does not convert to age groups to numbers...
                csv += std::to_string(odTrip.uuid) + ",\"" + routingResult.status + "\",\"" + ageGroup + "\",\"" + odTrip.gender + "\",\"" + odTrip.occupation + "\",\"";
                csv += odTrip.destinationActivity + "\",\"" + odTrip.mode + "\"," + std::to_string(odTrip.expansionFactor) + "," + std::to_string(routingResult.travelTimeSeconds) + ",";
                csv += std::to_string(odTrip.walkingTravelTimeSeconds) + "," + std::to_string(odTrip.departureTimeSeconds) + "," + std::to_string(routingResult.departureTimeSeconds) + ",";
                csv += std::to_string(routingResult.arrivalTimeSeconds) + "," + std::to_string(routingResult.numberOfTransfers) + "," + std::to_string(routingResult.inVehicleTravelTimeSeconds) + ",";
                csv += std::to_string(routingResult.transferTravelTimeSeconds) + "," + std::to_string(routingResult.waitingTimeSeconds) + "," + std::to_string(routingResult.accessTravelTimeSeconds) + ",";
                csv += std::to_string(routingResult.egressTravelTimeSeconds) + "," + std::to_string(routingResult.transferWaitingTimeSeconds) + "," + std::to_string(routingResult.firstWaitingTimeSeconds) + ",";
                csv += std::to_string(routingResult.nonTransitTravelTimeSeconds) + ",";
                
                int countLineUuids = routingResult.lineUuids.size();
                j = 0;
                for (auto & lineUuid : routingResult.lineUuids)
                {
                  csv += std::to_string(lineUuid);
                  if (j < countLineUuids - 1)
                  {
                    csv += "|";
                  }
                  j++;
                }
                csv += ",";
                j = 0;
                for (auto & lineTypeUuid : routingResult.lineTypeUuids)
                {
                  csv += std::to_string(lineTypeUuid);
                  if (j < countLineUuids - 1)
                  {
                    csv += "|";
                  }
                  j++;
                }
                csv += ",";
                j = 0;
                for (auto & agencyUuid : routingResult.agencyUuids)
                {
                  csv += std::to_string(agencyUuid);
                  if (j < countLineUuids - 1)
                  {
                    csv += "|";
                  }
                  j++;
                }
                csv += ",";
                j = 0;
                for (auto & boardingNodeUuid : routingResult.boardingNodeUuids)
                {
                  csv += std::to_string(boardingNodeUuid);
                  if (j < countLineUuids - 1)
                  {
                    csv += "|";
                  }
                  j++;
                }
                csv += ",";
                j = 0;
                for (auto & unboardingNodeUuid : routingResult.unboardingNodeUuids)
                {
                  csv += std::to_string(unboardingNodeUuid);
                  if (j < countLineUuids - 1)
                  {
                    csv += "|";
                  }
                  j++;
                }
                csv += ",";
                j = 0;
                for (auto & tripUuid : routingResult.tripUuids)
                {
                  csv += std::to_string(tripUuid);
                  if (j < countLineUuids - 1)
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
                odTripJson["uuid"]                         = odTrip.uuid;
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
                odTripJson["lineUuids"]                    = routingResult.lineUuids;
                odTripJson["Modes"]                        = routingResult.modes;
                odTripJson["agencyUuids"]                  = routingResult.agencyUuids;
                odTripJson["boardingNodeUuids"]            = routingResult.boardingNodeUuids;
                odTripJson["unboardingNodeUuids"]          = routingResult.unboardingNodeUuids;
                odTripJson["tripUuids"]                    = routingResult.tripUuids;
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
          linesOdTripsCountJson = {};
          for (auto & lineCount : linesOdTripsCount)
          {
            linesOdTripsCountJson[std::to_string(lineCount.first)] = lineCount.second;
          }
          json["linesOdTripsCount"] = linesOdTripsCountJson;
          
          pathsOdTripsProfilesJson = {};
          for (auto & pathProfile : pathsLegsProfile)
          {
            pathsOdTripsProfilesSequenceJson = {};
            for (auto & sequenceProfile : pathProfile.second)
            {
              //pathsOdTripsProfilesOdTripUuids.clear();
              //for (auto & odTripUuid : std::get<1>(sequenceProfile.second))
              //{
              //  pathsOdTripsProfilesOdTripUuids.push_back()
              //}
              pathsOdTripsProfilesSequenceJson[std::to_string(sequenceProfile.first)] = {{"demand", std::get<0>(sequenceProfile.second)}, {"odTripUuids", std::get<1>(sequenceProfile.second)}};
            }
            pathsOdTripsProfilesJson[std::to_string(pathProfile.first)] = pathsOdTripsProfilesSequenceJson;
          }
          json["pathsOdTripsProfiles"] = pathsOdTripsProfilesJson;
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
