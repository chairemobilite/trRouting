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

#include "calculation_time.hpp"
#include "parameters.hpp"
#include "connection_scan_algorithm.hpp"

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
  std::string dataFetcher {"database"}; // csv, database
  
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
    dataFetcher = variablesMap["dataFetcher"].as<std::string>();
  }
  else if (variablesMap.count("data") == 1)
  {
    dataFetcher = variablesMap["data"].as<std::string>();
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
  
  
  std::cout << "Using http port "      << serverPort << std::endl;
  std::cout << "Using osrm walk port"  << algorithmParams.osrmRoutingWalkingPort << std::endl;
  std::cout << "Using data fetcher "   << dataFetcher << std::endl;
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
  
  ConnectionScanAlgorithm calculator;
  algorithmParams.applicationShortname = dataShortname;
  algorithmParams.dataFetcher          = dataFetcher;
  
  calculator = ConnectionScanAlgorithm(algorithmParams);
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
    
    std::cout << request->path << std::endl;
    
    if (request->path_match.size() >= 1)
    {
      std::vector<std::string> parametersWithValues;
      
      std::string queryString = request->path_match[1];
      
      boost::split(parametersWithValues, queryString, boost::is_any_of("&"));
      
      float originLatitude, originLongitude, destinationLatitude, destinationLongitude;
      long long startingStopId{-1}, endingStopId{-1};
      std::map<int, bool> onlyServiceIds;
      std::map<int, bool> exceptServiceIds;
      std::map<int, bool> onlyRouteIds;
      std::map<int, bool> exceptRouteIds;
      std::map<int, bool> onlyRouteTypeIds;
      std::map<int, bool> exceptRouteTypeIds;
      std::map<int, bool> onlyAgencyIds;
      std::map<int, bool> exceptAgencyIds;
      
      calculator.params.onlyServiceIds     = onlyServiceIds;
      calculator.params.exceptServiceIds   = exceptServiceIds;
      calculator.params.onlyRouteIds       = onlyRouteIds;
      calculator.params.exceptRouteIds     = exceptRouteIds;
      calculator.params.onlyRouteTypeIds   = onlyRouteTypeIds;
      calculator.params.exceptRouteTypeIds = exceptRouteTypeIds;
      calculator.params.onlyAgencyIds      = onlyAgencyIds;
      calculator.params.exceptAgencyIds    = exceptAgencyIds;
      
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
      
      int timeHour;
      int timeMinute;
      
      calculator.params.forwardCalculation        = true;
      calculator.params.detailedResults           = false;
      calculator.params.returnAllStopsResult      = false;
      calculator.params.transferOnlyAtSameStation = false;
      calculator.params.transferBetweenSameRoute  = true;
      calculator.params.startingPoint             = Point();
      calculator.params.endingPoint               = Point();
      calculator.params.routingDateYear           = 0;
      calculator.params.routingDateMonth          = 0;
      calculator.params.routingDateDay            = 0;
      calculator.params.startingStopId            = -1;
      calculator.params.endingStopId              = -1;
      calculator.params.maxNumberOfTransfers      = -1;
      calculator.params.minWaitingTimeMinutes     = 5;
      calculator.params.departureTimeHour         = -1;
      calculator.params.departureTimeMinutes      = -1;
      calculator.params.arrivalTimeHour           = -1;
      calculator.params.arrivalTimeMinutes        = -1;
      calculator.params.maxTotalTravelTimeMinutes = -1;
      calculator.params.maxAccessWalkingTravelTimeFromOriginToFirstStopMinutes = 20;
      calculator.params.maxAccessWalkingTravelTimeFromLastStopToDestinationMinutes = 20;
      calculator.params.maxTransferWalkingTravelTimeMinutes = 20;
      calculator.params.maxTotalWalkingTravelTimeMinutes = 60;
      calculator.params.accessMode = "walking";
      calculator.params.egressMode = "walking";
      calculator.params.noResultSecondMode = "driving";
      calculator.params.tryNextModeIfRoutingFails = false;
      calculator.params.noResultNextAccessTimeMinutesIncrement = 5;
      calculator.params.maxNoResultNextAccessTimeMinutes = 40;
      
      for(auto & parameterWithValue : parametersWithValues)
      {
        boost::split(parameterWithValueVector, parameterWithValue, boost::is_any_of("="));
        
        if (parameterWithValueVector[0] == "origin")
        {
          boost::split(latitudeLongitudeVector, parameterWithValueVector[1], boost::is_any_of(","));
          originLatitude  = std::stof(latitudeLongitudeVector[0]);
          originLongitude = std::stof(latitudeLongitudeVector[1]);
          calculator.params.startingPoint = Point(originLatitude, originLongitude);
        }
        else if (parameterWithValueVector[0] == "destination")
        {
          boost::split(latitudeLongitudeVector, parameterWithValueVector[1], boost::is_any_of(","));
          destinationLatitude  = std::stof(latitudeLongitudeVector[0]);
          destinationLongitude = std::stof(latitudeLongitudeVector[1]);
          calculator.params.endingPoint = Point(destinationLatitude, destinationLongitude);
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
        else if (parameterWithValueVector[0] == "only_service_ids")
        {
          boost::split(onlyServiceIdsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string onlyServiceId : onlyServiceIdsVector)
          {
            onlyServiceIds[std::stoi(onlyServiceId)] = true;
          }
          calculator.params.onlyServiceIds = onlyServiceIds;
        }
        else if (parameterWithValueVector[0] == "except_service_ids")
        {
          boost::split(exceptServiceIdsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string exceptServiceId : exceptServiceIdsVector)
          {
            exceptServiceIds[std::stoi(exceptServiceId)] = true;
          }
          calculator.params.exceptServiceIds = exceptServiceIds;
        }
        else if (parameterWithValueVector[0] == "only_route_ids")
        {
          boost::split(onlyRouteIdsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string onlyRouteId : onlyRouteIdsVector)
          {
            onlyRouteIds[std::stoi(onlyRouteId)] = true;
          }
          calculator.params.onlyRouteIds = onlyRouteIds;
        }
        else if (parameterWithValueVector[0] == "except_route_ids")
        {
          boost::split(exceptRouteIdsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string exceptRouteId : exceptRouteIdsVector)
          {
            exceptRouteIds[std::stoi(exceptRouteId)] = true;
          }
          calculator.params.exceptRouteIds = exceptRouteIds;
        }
        else if (parameterWithValueVector[0] == "only_route_type_ids")
        {
          boost::split(onlyRouteTypeIdsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string onlyRouteTypeId : onlyRouteTypeIdsVector)
          {
            onlyRouteTypeIds[std::stoi(onlyRouteTypeId)] = true;
          }
          calculator.params.onlyRouteTypeIds = onlyRouteTypeIds;
        }
        else if (parameterWithValueVector[0] == "except_route_type_ids")
        {
          boost::split(exceptRouteTypeIdsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string exceptRouteTypeId : exceptRouteTypeIdsVector)
          {
            exceptRouteTypeIds[std::stoi(exceptRouteTypeId)] = true;
          }
          calculator.params.exceptRouteTypeIds = exceptRouteTypeIds;
        }
        else if (parameterWithValueVector[0] == "only_agency_ids")
        {
          boost::split(onlyAgencyIdsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string onlyAgencyId : onlyAgencyIdsVector)
          {
            onlyAgencyIds[std::stoi(onlyAgencyId)] = true;
          }
          calculator.params.onlyAgencyIds = onlyAgencyIds;
        }
        else if (parameterWithValueVector[0] == "except_agency_ids")
        {
          boost::split(exceptAgencyIdsVector, parameterWithValueVector[1], boost::is_any_of(","));
          for(std::string exceptAgencyId : exceptAgencyIdsVector)
          {
            exceptAgencyIds[std::stoi(exceptAgencyId)] = true;
          }
          calculator.params.exceptAgencyIds = exceptAgencyIds;
        }
        else if (parameterWithValueVector[0] == "starting_stop_id"
                 || parameterWithValueVector[0] == "start_stop_id"
                 || parameterWithValueVector[0] == "origin_stop_id"
                )
        {
          calculator.params.startingStopId = std::stoi(parameterWithValueVector[1]);
        }
        else if (parameterWithValueVector[0] == "ending_stop_id"
                 || parameterWithValueVector[0] == "end_stop_id"
                 || parameterWithValueVector[0] == "destination_stop_id"
                )
        {
          calculator.params.endingStopId = std::stoi(parameterWithValueVector[1]);
        }
        else if (parameterWithValueVector[0] == "max_number_of_transfers" || parameterWithValueVector[0] == "max_transfers")
        {
          calculator.params.maxNumberOfTransfers = std::stoi(parameterWithValueVector[1]);
        }
        else if (parameterWithValueVector[0] == "min_waiting_time" || parameterWithValueVector[0] == "min_waiting_time_minutes")
        {
          calculator.params.minWaitingTimeMinutes = std::stoi(parameterWithValueVector[1]);
        }
        else if (parameterWithValueVector[0] == "max_travel_time" || parameterWithValueVector[0] == "max_travel_time_minutes")
        {
          calculator.params.maxTotalTravelTimeMinutes = std::stoi(parameterWithValueVector[1]);
        }
        else if (parameterWithValueVector[0] == "max_access_travel_time" || parameterWithValueVector[0] == "max_access_travel_time_minutes")
        {
          calculator.params.maxAccessWalkingTravelTimeFromOriginToFirstStopMinutes = std::stoi(parameterWithValueVector[1]);
        }
        else if (parameterWithValueVector[0] == "max_egress_travel_time" || parameterWithValueVector[0] == "max_egress_travel_time_minutes")
        {
          calculator.params.maxAccessWalkingTravelTimeFromLastStopToDestinationMinutes = std::stoi(parameterWithValueVector[1]);
        }
        else if (parameterWithValueVector[0] == "max_transfer_travel_time" || parameterWithValueVector[0] == "max_transfer_travel_time_minutes")
        {
          calculator.params.maxTransferWalkingTravelTimeMinutes = std::stoi(parameterWithValueVector[1]);
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
      
      //calculator.algorithmCalculationTime.stopStep();
      
      //std::cout << "-- parsing request -- " << calculator.algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
      
      calculator.refresh();
      
      //calculator.algorithmCalculationTime.stopStep();
      //calculator.algorithmCalculationTime.startStep();
      
      calculator.resetAccessEgressModes();
      
      //calculator.algorithmCalculationTime.stopStep();
      
      //std::cout << "-- reset access egress modes -- " << calculator.algorithmCalculationTime.getStepDurationMilliseconds() << " ms\n";
      
      resultStr = calculator.calculate("1");
      
      //calculator.algorithmCalculationTime.startStep();

    }
    else
    {
      resultStr = "{\"status\": \"failed\", \"error\": \"Wrong or malformed query\"}";
    }
    
    //calculator.algorithmCalculationTime.stop();
      
    //std::cerr << "-- calculation time -- " << calculator.algorithmCalculationTime.getDurationMilliseconds() << " ms\n";
    
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << resultStr.length() << "\r\n\r\n" << resultStr;
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
