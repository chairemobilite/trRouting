#include "server_http.hpp"
#include "client_http.hpp"

//Added for the json-example
#define BOOST_SPIRIT_THREADSAFE

#include <vector>
#include <algorithm>
#include <string>
#include <iterator>
#include "spdlog/spdlog.h"

#include <boost/uuid/uuid.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string.hpp>

#include "cache_fetcher.hpp"
#include "calculation_time.hpp"
#include "parameters.hpp"
#include "scenario.hpp"
#include "calculator.hpp"
#include "program_options.hpp"
#include "result_to_v2.hpp"
#include "result_to_v2_summary.hpp"
#include "result_to_v2_accessibility.hpp"
#include "routing_result.hpp"
#include "transit_data.hpp"
#include "osrmgeofilter.hpp"
#include "euclideangeofilter.hpp"

using namespace TrRouting;

typedef SimpleWeb::Server<SimpleWeb::HTTP> HttpServer;
typedef SimpleWeb::Client<SimpleWeb::HTTP> HttpClient;

std::string intializeResponse(DataStatus status)
{
  switch(status)
  {
    case DataStatus::READY: return "";
    case DataStatus::DATA_READ_ERROR: return "{\"status\": \"data_error\"}";
    case DataStatus::NO_AGENCIES:
      return "{\"status\": \"error\", \"error\": {\"error\": \"No agencies found\", \"code\": \"MISSING_DATA_AGENCIES\"}}";
    case DataStatus::NO_LINES:
      return "{\"status\": \"error\", \"error\": {\"error\": \"No lines found\", \"code\": \"MISSING_DATA_LINES\"}}";
    case DataStatus::NO_NODES:
      return "{\"status\": \"error\", \"error\": {\"error\": \"No nodes found\", \"code\": \"MISSING_DATA_NODES\"}}";
    case DataStatus::NO_PATHS:
      return "{\"status\": \"error\", \"error\": {\"error\": \"No paths found\", \"code\": \"MISSING_DATA_PATHS\"}}";
    case DataStatus::NO_SCENARIOS:
      return "{\"status\": \"error\", \"error\": {\"error\": \"No scenarios found\", \"code\": \"MISSING_DATA_SCENARIOS\"}}";
    case DataStatus::NO_SCHEDULES:
      return "{\"status\": \"error\", \"error\": {\"error\": \"No schedules found\", \"code\": \"MISSING_DATA_SCHEDULES\"}}";
    case DataStatus::NO_SERVICES:
      return "{\"status\": \"error\", \"error\": {\"error\": \"No services found\", \"code\": \"MISSING_DATA_SERVICES\"}}";
    default: return "PARAM_ERROR_UNKNOWN";
  }
}

std::string getFastErrorResponse(DataStatus status)
{
  switch(status)
  {
    case DataStatus::READY: return "";
    case DataStatus::DATA_READ_ERROR: return "{\"status\": \"data_error\", \"errorCode\": \"DATA_ERROR\"}";
    case DataStatus::NO_AGENCIES:
      return "{\"status\": \"data_error\", \"errorCode\": \"MISSING_DATA_AGENCIES\"}";
    case DataStatus::NO_LINES:
      return "{\"status\": \"data_error\", \"errorCode\": \"MISSING_DATA_LINES\"}";
    case DataStatus::NO_NODES:
      return "{\"status\": \"data_error\", \"errorCode\": \"MISSING_DATA_NODES\"}";
    case DataStatus::NO_PATHS:
      return "{\"status\": \"data_error\", \"errorCode\": \"MISSING_DATA_PATHS\"}";
    case DataStatus::NO_SCENARIOS:
      return "{\"status\": \"data_error\", \"errorCode\": \"MISSING_DATA_SCENARIOS\"}";
    case DataStatus::NO_SCHEDULES:
      return "{\"status\": \"data_error\", \"errorCode\": \"MISSING_DATA_SCHEDULES\"}";
    case DataStatus::NO_SERVICES:
      return "{\"status\": \"data_error\", \"errorCode\": \"MISSING_DATA_SERVICES\"}";
    default: return "PARAM_ERROR_UNKNOWN";
  }
}

std::string getResponseCode(ParameterException::Type type)
{
  switch(type)
  {
    case ParameterException::Type::EMPTY_SCENARIO: return "EMPTY_SCENARIO";
    case ParameterException::Type::MISSING_SCENARIO: return "MISSING_PARAM_SCENARIO";
    case ParameterException::Type::MISSING_ORIGIN: return "MISSING_PARAM_ORIGIN";
    case ParameterException::Type::MISSING_DESTINATION: return "MISSING_PARAM_DESTINATION";
    case ParameterException::Type::MISSING_TIME_OF_TRIP: return "MISSING_PARAM_TIME_OF_TRIP";
    case ParameterException::Type::INVALID_SCENARIO: return "INVALID_SCENARIO";
    case ParameterException::Type::INVALID_ORIGIN: return "INVALID_ORIGIN";
    case ParameterException::Type::INVALID_DESTINATION: return "INVALID_DESTINATION";
    case ParameterException::Type::INVALID_NUMERICAL_DATA: return "INVALID_NUMERICAL_DATA";
    default: return "PARAM_ERROR_UNKNOWN";
  }
}

int main(int argc, char** argv) {

  // Set params:
  ProgramOptions programOptions;
  programOptions.parseOptions(argc, argv);

  // setup program options:
  spdlog::info("Starting transit routing on port {} for the data: {}", programOptions.port, programOptions.cachePath);
  
  if (programOptions.debug) {
    spdlog::set_level(spdlog::level::debug);
  }

  DataFetcher *fetcher = 0;
  if (programOptions.dataFetcherShortname == "cache") {
    fetcher = new CacheFetcher(programOptions.cachePath);
  } else {
    spdlog::error("Using invalid DataFetcher {}", programOptions.dataFetcherShortname);
    exit(-2);
  }

  spdlog::info("preparing calculator...");
  TransitData transitData(*fetcher, programOptions.cacheAllConnectionSets);
  //TODO We wanted to handle error in the constructor, but later part of this code expect a dataStatus
  // leaving as a todo
  DataStatus dataStatus = transitData.getDataStatus();

  // Selection which geofilter to use. OSRM is the default one. Euclidean mostly used for debugging and testing
  GeoFilter *geoFilter = 0;
  if (programOptions.useEuclideanDistance) {
    geoFilter = new EuclideanGeoFilter();
    spdlog::info("Using Euclidean distance for access/egress node time/distance");
  } else {
    geoFilter = new OsrmGeoFilter("walking", programOptions.osrmWalkingHost, programOptions.osrmWalkingPort);
    spdlog::info("Using OSRM for access/egress node time/distance");
  }

  spdlog::info("preparing server with {} threads...", programOptions.numberOfThreads);

  HttpServer server;
  server.config.port = programOptions.port;
  server.config.thread_pool_size = programOptions.numberOfThreads;

  // updateCache:
  server.resource["^/updateCache[/]?$"]["GET"]=[&server, &transitData](std::shared_ptr<HttpServer::Response> serverResponse, std::shared_ptr<HttpServer::Request> request) {

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

    bool correctCacheName {false};
    //TODO Merge this and the preparations.cpp code
    for(std::string cacheName : cacheNames)
    {
      if (cacheName == "data_sources" || cacheName == "all")
      {
        correctCacheName = true;
        transitData.updateDataSources(customCacheDirectoryPath);
      }
      /* TODO #167
      if (cacheName == "households" || cacheName == "all")
      {
        correctCacheName = true;
        transitData.updateHouseholds(customCacheDirectoryPath);
      }
      */
      if (cacheName == "persons" || cacheName == "all")
      {
        correctCacheName = true;
        transitData.updatePersons(customCacheDirectoryPath);
      }
      if (cacheName == "od_trips" || cacheName == "all")
      {
        correctCacheName = true;
        transitData.updateOdTrips(customCacheDirectoryPath);
      }
      /* TODO #167
      if (cacheName == "places" || cacheName == "all")
      {
        correctCacheName = true;
        transitData.updatePlaces(customCacheDirectoryPath);
      }
      */
      if (cacheName == "agencies" || cacheName == "all")
      {
        correctCacheName = true;
        transitData.updateAgencies(customCacheDirectoryPath);
      }
      if (cacheName == "services" || cacheName == "all")
      {
        correctCacheName = true;
        transitData.updateServices(customCacheDirectoryPath);
      }
      if (cacheName == "nodes" || cacheName == "all")
      {
        correctCacheName = true;
        transitData.updateNodes(customCacheDirectoryPath);
      }
      if (cacheName == "lines" || cacheName == "all")
      {
        correctCacheName = true;
        transitData.updateLines(customCacheDirectoryPath);
      }
      if (cacheName == "paths" || cacheName == "all")
      {
        correctCacheName = true;
        transitData.updatePaths(customCacheDirectoryPath);
      }
      if (cacheName == "scenarios" || cacheName == "all")
      {
        correctCacheName = true;
        transitData.updateScenarios(customCacheDirectoryPath);
      }
      if (cacheName == "schedules" || cacheName == "all")
      {
        correctCacheName = true;
        transitData.updateSchedules(customCacheDirectoryPath);
      }

      //TODO This is incorrect if we have multiple name and the second one is wrong, correctCacheName is always true
      if (correctCacheName)
      {
        cacheNamesStr += cacheName;
        cacheNamesStr += ",";
      }
    }

    //TODO do this only if we had at least one correct name
    //Reinit some data after the update
    // TODO Just the schedules???
    if (cacheNames.size() > 0)
    {
      // Remove last ","
      cacheNamesStr.pop_back();
      response = "{\"status\": \"success\", \"cache_names\": \"" + cacheNamesStr + "\", \"custom_cache_path\": \"" + customCacheDirectoryPath + "\"}";
    }
    else
    {
      response = "{\"status\": \"error\", \"error\": \"missing or wrong cache name\"}";
    }

    *serverResponse << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;

  };






  // closeServer and exit app:
  server.resource["^/exit[/]?\\?([0-9a-zA-Z&=_,:/.-]+)$"]["GET"]=[&server](std::shared_ptr<HttpServer::Response> serverResponse, std::shared_ptr<HttpServer::Request> ) {

    std::string response {""};
    *serverResponse << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;

    // todo

  };






  // Routing request for a single origin destination
  // TODO Copy-pasted and adapted from /route/v1/transit. There's still a lot of common code. Application code should be extracted to common functions outside the web server
  server.resource["^/v2/route[/]?$"]["GET"]=[&server, &dataStatus, &transitData, &geoFilter](std::shared_ptr<HttpServer::Response> serverResponse, std::shared_ptr<HttpServer::Request> request) {
    // Have a global id to match the requests in the logs
    static int routeRequestId = 0;
    std::string response = getFastErrorResponse(dataStatus);

    if (!response.empty()) {
      *serverResponse << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;
      return;
    }
    Calculator calculator(transitData, *geoFilter);

    // prepare parameters:
    std::vector<std::pair<std::string, std::string>> parametersWithValues;
    auto queryFields = request->parse_query_string();
    for(auto &field : queryFields)
    {
      parametersWithValues.push_back(std::make_pair(field.first, field.second));
    }
    int currentRequestId = routeRequestId++;
    spdlog::info("-- calculating route request -- {}", currentRequestId);

    try
    {
      RouteParameters queryParams = RouteParameters::createRouteODParameter(parametersWithValues, transitData.getScenarios());

      try {
        if (queryParams.isWithAlternatives())
        {
          TrRouting::AlternativesResult alternativeResult = calculator.alternativesRouting(queryParams);
          response = ResultToV2Response::resultToJsonString(alternativeResult, queryParams).dump(2);
        }
        else
        {
          std::unique_ptr<TrRouting::SingleCalculationResult> routingResult = calculator.calculateSingle(queryParams);
          if (routingResult.get() != nullptr) {
            response = ResultToV2Response::resultToJsonString(*routingResult.get(), queryParams).dump(2);
          }
        }

        spdlog::info("-- route request complete -- {}", currentRequestId);

      } catch (NoRoutingFoundException &e) {
        response = ResultToV2Response::noRoutingFoundResponse(queryParams, e.getReason()).dump(2);
        spdlog::info("-- route request not found -- {}", currentRequestId);

      }

      *serverResponse << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;

    } catch (ParameterException &exp) {
      auto responseCode = getResponseCode(exp.getType());
      spdlog::info("-- parameter exception in route calculation -- {}", responseCode);
      response = "{\"status\": \"query_error\", \"errorCode\": \"" + responseCode + "\"}";
      *serverResponse << "HTTP/1.1 400 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;
    } catch (...) {
      std::exception_ptr eptr = std::current_exception(); // capture
      try {
          std::rethrow_exception(eptr);
      } catch(const std::exception& e) {
          spdlog::error("-- unknown exception in route calculation -- {}", e.what());
          std::cout << "Caught exception \"" << e.what() << "\"\n";
      }
      response = "{\"status\": \"query_error\", \"errorCode\": \"PARAM_ERROR_UNKNOWN\"}";
      *serverResponse << "HTTP/1.1 400 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;
    }

  };

  // Request a summary of lines data for a route
  // TODO Copy pasted from v2/route. There's a lot in common, it should be extracted to common class, just the response parser is different
  server.resource["^/v2/summary[/]?$"]["GET"]=[&server, &dataStatus, &transitData, &geoFilter](std::shared_ptr<HttpServer::Response> serverResponse, std::shared_ptr<HttpServer::Request> request) {
    // Have a global id to match the requests in the logs
    static int summaryRequestId = 0;

    std::string response = getFastErrorResponse(dataStatus);

    if (!response.empty()) {
      *serverResponse << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;
      return;
    }
    Calculator calculator(transitData, *geoFilter);

    // prepare parameters:
    std::vector<std::pair<std::string, std::string>> parametersWithValues;
    auto queryFields = request->parse_query_string();
    for(auto &field : queryFields)
    {
      parametersWithValues.push_back(std::make_pair(field.first, field.second));
    }
    int currentRequestId = summaryRequestId++;

    spdlog::info("-- calculating summary request -- {}", currentRequestId);

    try
    {
      RouteParameters queryParams = RouteParameters::createRouteODParameter(parametersWithValues, transitData.getScenarios());

      try {
        if (queryParams.isWithAlternatives())
        {
          TrRouting::AlternativesResult alternativeResult = calculator.alternativesRouting(queryParams);
          response = ResultToV2SummaryResponse::resultToJsonString(alternativeResult, queryParams).dump(2);
        }
        else
        {
          std::unique_ptr<TrRouting::SingleCalculationResult> routingResult = calculator.calculateSingle(queryParams);
          if (routingResult.get() != nullptr) {
            response = ResultToV2SummaryResponse::resultToJsonString(*routingResult.get(), queryParams).dump(2);
          }
        }

        spdlog::info("-- summary request complete -- {}", currentRequestId);

      } catch (NoRoutingFoundException &e) {
        response = ResultToV2SummaryResponse::noRoutingFoundResponse(queryParams, e.getReason()).dump(2);
        spdlog::info("-- summary request not found -- {}", currentRequestId);
      }

      *serverResponse << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;

    } catch (ParameterException &exp) {
      auto responseCode = getResponseCode(exp.getType());
      spdlog::info("-- parameter exception in summary calculation -- {}", responseCode);
      response = "{\"status\": \"query_error\", \"errorCode\": \"" + responseCode + "\"}";
      *serverResponse << "HTTP/1.1 400 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;
    } catch (...) {
      std::exception_ptr eptr = std::current_exception(); // capture
      try {
          std::rethrow_exception(eptr);
      } catch(const std::exception& e) {
          spdlog::error("-- unknown exception in summary calculation -- {}", e.what());
          std::cout << "Caught exception \"" << e.what() << "\"\n";
      }
      response = "{\"status\": \"query_error\", \"errorCode\": \"PARAM_ERROR_UNKNOWN\"}";
      *serverResponse << "HTTP/1.1 400 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;
    }

  };

  // Routing request for a single origin destination
  // TODO Copy-pasted and adapted from /route/v1/transit. There's still a lot of common code. Application code should be extracted to common functions outside the web server
  server.resource["^/v2/accessibility[/]?$"]["GET"]=[&server, &dataStatus, &transitData, &geoFilter](std::shared_ptr<HttpServer::Response> serverResponse, std::shared_ptr<HttpServer::Request> request) {
    // Have a global id to match the requests in the logs
    static int accessibilityRequestId = 0;

    std::string response = getFastErrorResponse(dataStatus);

    if (!response.empty()) {
      *serverResponse << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;
      return;
    }
    Calculator calculator(transitData, *geoFilter);

    // prepare parameters:
    std::vector<std::pair<std::string, std::string>> parametersWithValues;
    auto queryFields = request->parse_query_string();
    for(auto &field : queryFields)
    {
      parametersWithValues.push_back(std::make_pair(field.first, field.second));
    }

    int currentRequestId = accessibilityRequestId++;
    spdlog::info("-- calculating accessibility request -- {}", currentRequestId);

    try
    {
      AccessibilityParameters queryParams = AccessibilityParameters::createAccessibilityParameter(parametersWithValues, transitData.getScenarios());

      try {
        std::unique_ptr<AllNodesResult> accessibilityResult = calculator.calculateAllNodes(queryParams);
        if (accessibilityResult.get() != nullptr) {
          response = ResultToV2AccessibilityResponse::resultToJsonString(*accessibilityResult.get(), queryParams).dump(2);
        }

        spdlog::info("-- accessibility request complete -- {}", currentRequestId);

      } catch (NoRoutingFoundException &e) {
        response = ResultToV2AccessibilityResponse::noRoutingFoundResponse(queryParams, e.getReason()).dump(2);
        spdlog::info("-- accessibility request not found -- {}", currentRequestId);
      }

      *serverResponse << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;

    } catch (ParameterException &exp) {
      auto responseCode = getResponseCode(exp.getType());
      spdlog::info("-- parameter exception in accessibility map calculation -- {}", responseCode);
      response = "{\"status\": \"query_error\", \"errorCode\": \"" + responseCode + "\"}";
      *serverResponse << "HTTP/1.1 400 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;
    } catch (...) {
      std::exception_ptr eptr = std::current_exception(); // capture
      try {
          std::rethrow_exception(eptr);
      } catch(const std::exception& e) {
          spdlog::error("-- unknown exception in accessibility calculation -- {}", e.what());
          std::cout << "Caught exception \"" << e.what() << "\"\n";
      }
      response = "{\"status\": \"query_error\", \"errorCode\": \"PARAM_ERROR_UNKNOWN\"}";
      *serverResponse << "HTTP/1.1 400 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;
    }

  };

  server.default_resource["GET"] = [](std::shared_ptr<HttpServer::Response> serverResponse, std::shared_ptr<HttpServer::Request> request) {
    spdlog::info("calculating request: {}", request->content.string());

    std::string response = "{\"status\": \"error\", \"error\": \"missing params\"}";

    *serverResponse << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;
  };

  server.on_error = [](std::shared_ptr<HttpServer::Request> /*request*/, const SimpleWeb::error_code & /*ec*/) {
    // Handle errors here
    // Note that connection timeouts will also call this handle with ec set to SimpleWeb::errc::operation_canceled
  };

  spdlog::info("starting server...");
  std::thread server_thread([&server](){
    server.start();
  });

  // Wait for server to start so that the client can connect:
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  spdlog::info("ready.");

  server_thread.join();

  // Cleanup
  delete fetcher;

  return 0;
}









