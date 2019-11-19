#include "server_http.hpp"
#include "client_http.hpp"

//Added for the json-example
#define BOOST_SPIRIT_THREADSAFE

#include <vector>
#include <algorithm>
#include <string>
#include <iostream>
#include <iterator>
#include <curses.h>

#include <boost/optional.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem.hpp>

#include "toolbox.hpp"
#include "gtfs_fetcher.hpp"
#include "csv_fetcher.hpp"
#include "cache_fetcher.hpp"
#include "calculation_time.hpp"
#include "parameters.hpp"
#include "scenario.hpp"
#include "calculator.hpp"
#include "program_options.hpp"

using namespace TrRouting;

typedef SimpleWeb::Server<SimpleWeb::HTTP> HttpServer;
typedef SimpleWeb::Client<SimpleWeb::HTTP> HttpClient;

std::string consoleRed        = "";
std::string consoleGreen      = "";
std::string consoleYellow     = "";
std::string consoleCyan       = "";
std::string consoleMagenta    = "";
std::string consoleResetColor = "";

int main(int argc, char** argv) {
  
  boost::uuids::string_generator uuidGeneratorMain;

  // Set params:
  ProgramOptions programOptions;
  programOptions.parseOptions(argc, argv);
  Parameters algorithmParams;
  
  // setup program options:
  
  // setup console colors:
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
    
  std::cout << "Starting transit routing on port " << consoleGreen << programOptions.port << consoleResetColor 
            << " for the project: " << consoleGreen << programOptions.projectShortname << consoleResetColor 
            << std::endl << std::endl;
  
  algorithmParams.projectShortname       = programOptions.projectShortname;
  algorithmParams.cacheDirectoryPath     = programOptions.cachePath;
  algorithmParams.dataFetcherShortname   = programOptions.dataFetcherShortname;
  algorithmParams.osrmWalkingPort        = programOptions.osrmWalkingPort;
  algorithmParams.osrmCyclingPort        = programOptions.osrmCyclingPort;
  algorithmParams.osrmDrivingPort        = programOptions.osrmDrivingPort;
  algorithmParams.osrmWalkingHost        = programOptions.osrmWalkingHost;
  algorithmParams.osrmCyclingHost        = programOptions.osrmCyclingHost;
  algorithmParams.osrmDrivingHost        = programOptions.osrmDrivingHost;
  algorithmParams.osrmWalkingFilePath    = programOptions.osrmWalkingFilePath;
  algorithmParams.osrmCyclingFilePath    = programOptions.osrmCyclingFilePath;
  algorithmParams.osrmDrivingFilePath    = programOptions.osrmDrivingFilePath;
  algorithmParams.osrmWalkingUseLib      = programOptions.osrmWalkingUseLib;
  algorithmParams.osrmCyclingUseLib      = programOptions.osrmCyclingUseLib;
  algorithmParams.osrmDrivingUseLib      = programOptions.osrmDrivingUseLib;
  algorithmParams.serverDebugDisplay     = programOptions.debug;

  GtfsFetcher  gtfsFetcher     = GtfsFetcher();
  algorithmParams.gtfsFetcher  = &gtfsFetcher;
  CsvFetcher   csvFetcher      = CsvFetcher();
  algorithmParams.csvFetcher   = &csvFetcher;
  CacheFetcher cacheFetcher    = CacheFetcher();
  algorithmParams.cacheFetcher = &cacheFetcher;
  
  Calculator calculator(algorithmParams);
  int i = 0;
  
  std::cout << "preparing server..." << std::endl;
  
  //HTTP-server using 1 thread
  //Unless you do more heavy non-threaded processing in the resources,
  //1 thread is usually faster than several threads
  HttpServer server;
  server.config.port = programOptions.port;



  server.resource["^/saveCache[/]?$"]["POST"] = [&server, &calculator](std::shared_ptr<HttpServer::Response> serverResponse, std::shared_ptr<HttpServer::Request> request) {
    
    std::string response {""};


    try {
      boost::property_tree::ptree pt;
      boost::property_tree::read_json(request->content, pt);

      //auto name = pt.get<string>("firstName") + " " + pt.get<string>("lastName");

      //response = "{\"status\": \"failed\", \"error\": \"missing or wrong cache name\"}";
    }
    catch(const std::exception &e) {
      std::string error(e.what());
      response = "{\"status\": \"failed\", \"error\": \"" + error + "\"}";
    }


    response = "{\"status\": \"failed\", \"error\": \"missing or wrong cache name\"}";

    *serverResponse << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;

  };


  // updateCache:
  server.resource["^/updateCache[/]?$"]["GET"]=[&server, &calculator](std::shared_ptr<HttpServer::Response> serverResponse, std::shared_ptr<HttpServer::Request> request) {
    
    std::string              response {""};
    std::vector<std::string> parametersWithValues;
    std::vector<std::string> parameterWithValueVector;
    std::string              queryString;
    std::string              customCacheDirectoryPath {""};
    std::string              cacheNamesStr {""};
    std::vector<std::string> cacheNames;
    std::vector<std::string> cacheNamesVector;

    // prepare parameters:
    auto queryFields = request->parse_query_string();
    for(auto &field : queryFields)
    {
      parametersWithValues.push_back(field.first + "=" + field.second);
    }

    for(auto & parameterWithValue : parametersWithValues)
    {
      boost::split(parameterWithValueVector, parameterWithValue, boost::is_any_of("="));

      if (parameterWithValueVector[0] == "names" || parameterWithValueVector[0] == "caches" || parameterWithValueVector[0] == "cache_names" || parameterWithValueVector[0] == "name" || parameterWithValueVector[0] == "cache" || parameterWithValueVector[0] == "cache_name")
      {

        boost::split(cacheNamesVector, parameterWithValueVector[1], boost::is_any_of(","));
        for(std::string cacheName : cacheNamesVector)
        {
          cacheNames.push_back(cacheName);
        }
        continue;
      }
      if (parameterWithValueVector[0] == "path" || parameterWithValueVector[0] == "custom_path" || parameterWithValueVector[0] == "custom_cache_path")
      {
        customCacheDirectoryPath = parameterWithValueVector[1];
        continue;
      }
    }
  
    int i {0};
    bool correctCacheName {false};
    for(std::string cacheName : cacheNames)
    {
      if (cacheName == "data_sources" || cacheName == "all")
      {
        correctCacheName = true;
        calculator.updateDataSourcesFromCache(calculator.params, customCacheDirectoryPath);
      }
      if (cacheName == "households" || cacheName == "all")
      {
        correctCacheName = true;
        calculator.updateHouseholdsFromCache(calculator.params, customCacheDirectoryPath);
      }
      if (cacheName == "persons" || cacheName == "all")
      {
        correctCacheName = true;
        calculator.updatePersonsFromCache(calculator.params, customCacheDirectoryPath);
      }
      if (cacheName == "od_trips" || cacheName == "all")
      {
        correctCacheName = true;
        calculator.updateOdTripsFromCache(calculator.params, customCacheDirectoryPath);
      }
      if (cacheName == "places" || cacheName == "all")
      {
        correctCacheName = true;
        calculator.updatePlacesFromCache(calculator.params, customCacheDirectoryPath);
      }
      if (cacheName == "agencies" || cacheName == "all")
      {
        correctCacheName = true;
        calculator.updateAgenciesFromCache(calculator.params, customCacheDirectoryPath);
      }
      if (cacheName == "services" || cacheName == "all")
      {
        correctCacheName = true;
        calculator.updateServicesFromCache(calculator.params, customCacheDirectoryPath);
      }
      if (cacheName == "nodes" || cacheName == "all")
      {
        correctCacheName = true;
        calculator.updateNodesFromCache(calculator.params, customCacheDirectoryPath);
      }
      if (cacheName == "lines" || cacheName == "all")
      {
        correctCacheName = true;
        calculator.updateLinesFromCache(calculator.params, customCacheDirectoryPath);
      }
      if (cacheName == "paths" || cacheName == "all")
      {
        correctCacheName = true;
        calculator.updatePathsFromCache(calculator.params, customCacheDirectoryPath);
      }
      if (cacheName == "scenarios" || cacheName == "all")
      {
        correctCacheName = true;
        calculator.updateScenariosFromCache(calculator.params, customCacheDirectoryPath);
      }
      if (cacheName == "schedules" || cacheName == "all")
      {
        correctCacheName = true;
        calculator.updateSchedulesFromCache(calculator.params, customCacheDirectoryPath);
      }

      if (correctCacheName)
      {
        cacheNamesStr += cacheName;
        if (i < cacheNames.size() - 1)
        {
          cacheNamesStr += ",";
        }
        i++;
      }
    }
    if (cacheNames.size() > 0)
    {
      response = "{\"status\": \"success\", \"cache_names\": \"" + cacheNamesStr + "\", \"custom_cache_path\": \"" + customCacheDirectoryPath + "\"}";
    }
    else
    {
      response = "{\"status\": \"failed\", \"error\": \"missing or wrong cache name\"}";
    }

    *serverResponse << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;

  };






  // closeServer and exit app:
  server.resource["^/exit[/]?\\?([0-9a-zA-Z&=_,:/.-]+)$"]["GET"]=[&server, &calculator](std::shared_ptr<HttpServer::Response> serverResponse, std::shared_ptr<HttpServer::Request> request) {
    
    std::string response {""};
    *serverResponse << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;

    // todo

  };






  // routing request
  server.resource["^/route/v1/transit[/]?$"]["GET"]=[&server, &calculator](std::shared_ptr<HttpServer::Response> serverResponse, std::shared_ptr<HttpServer::Request> request) {
    
    // prepare benchmarking and timer:
    calculator.algorithmCalculationTime.start();
    calculator.benchmarking.clear();
    
    // prepare parameters:
    std::vector<std::string> parametersWithValues;
    auto queryFields = request->parse_query_string();
    for(auto &field : queryFields)
    {
      parametersWithValues.push_back(field.first + "=" + field.second);
    }
  
    // clear and initialize benchmarking:
    if (calculator.params.debugDisplay)
    {
      calculator.benchmarking["reset"]               = 0;
      calculator.benchmarking["forward_calculation"] = 0;
      //calculator.benchmarking["forward_journey"]     = 0;
      calculator.benchmarking["reverse_calculation"] = 0;
      //calculator.benchmarking["reverse_journey"]     = 0;
      //calculator.benchmarking["generating_results"]  = 0;
    }
    
    std::cout << "calculating request: " << request->path << std::endl;

    int countOdTripsCalculated {0};
    std::string response {""};
    
    // update params:
    calculator.params.setDefaultValues();
    calculator.params.update(parametersWithValues, calculator.scenarioIndexesByUuid, calculator.scenarios, calculator.nodeIndexesByUuid, calculator.agencyIndexesByUuid, calculator.lineIndexesByUuid, calculator.serviceIndexesByUuid, calculator.modeIndexesByShortname, calculator.dataSourceIndexesByUuid);

    if (calculator.params.isCompleteForCalculation())
    {
      
      // find OdTrip if provided:
      bool   foundOdTrip{false};

      calculator.origin      = &calculator.params.origin;
      calculator.destination = &calculator.params.destination;
      calculator.odTrip      = nullptr;

      if (calculator.params.odTripUuid.is_initialized() && calculator.odTripIndexesByUuid.count(calculator.params.odTripUuid.get()))
      {
        calculator.odTrip = calculator.odTrips[calculator.odTripIndexesByUuid[calculator.params.odTripUuid.get()]].get();
        foundOdTrip = true;
        std::cout << "od trip uuid " << calculator.odTrip->uuid << std::endl;
        calculator.origin      = calculator.odTrip->origin.get();
        calculator.destination = calculator.odTrip->destination.get();
        response = calculator.calculate().json.dump(2);
      }
      else if (calculator.params.alternatives)
      {
        response = calculator.alternativesRouting();
      }
      else if (!calculator.params.alternatives && (calculator.params.calculateAllOdTrips || foundOdTrip ))
      {
        response = calculator.odTripsRouting();
      }
      else
      {
        response = calculator.calculate().json.dump(2);
      }
      
      if (calculator.params.saveResultToFile)
      {
        std::cerr << "writing file" << std::endl;
        std::ofstream file;
        //file.imbue(std::locale("en_US.UTF8"));
        file.open(calculator.params.calculationName + "." + calculator.params.responseFormat, std::ios_base::trunc);
        file << response;
        file.close();
      }
      
    }
    else
    {
      response = "{\"status\": \"failed\", \"error\": \"Wrong or malformed query\"}";
    }
    
    std::cerr << "-- total -- " << calculator.algorithmCalculationTime.getDurationMicrosecondsNoStop() << " microseconds\n";
    
    if (calculator.params.debugDisplay)
    {
      for (auto & benchmark : calculator.benchmarking)
      {
        std::cerr << "  -- " << benchmark.first << " -- " << benchmark.second / 1000 << " ms ";
        if (countOdTripsCalculated > 0)
        {
          std::cerr << " (" << (benchmark.second / countOdTripsCalculated / 1000) << " per odTrip)";
        }
        std::cerr << "\n";
      }
      if (countOdTripsCalculated > 0)
      {
        std::cerr << "  -- number of od trips calculated -- " << countOdTripsCalculated << "\n";
      }
    }
    
    *serverResponse << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/" << calculator.params.responseFormat << "; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;
    
  };
  


  server.default_resource["GET"] = [](std::shared_ptr<HttpServer::Response> serverResponse, std::shared_ptr<HttpServer::Request> request) {
    std::cout << "calculating request: " << request->content.string() << std::endl;
    
    std::string response = "{\"status\": \"failed\", \"error\": \"missing params\"}";

    *serverResponse << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;
  };

  server.on_error = [](std::shared_ptr<HttpServer::Request> /*request*/, const SimpleWeb::error_code & /*ec*/) {
    // Handle errors here
    // Note that connection timeouts will also call this handle with ec set to SimpleWeb::errc::operation_canceled
  };

  std::cout << "starting server..." << std::endl;
  std::thread server_thread([&server](){
    server.start();
  });

  // Wait for server to start so that the client can connect:
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  
  std::cout << "ready." << std::endl;
  
  server_thread.join();
    
  return 0;
}









