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
#include "result_to_v1.hpp"
#include "result_to_v2.hpp"
#include "result_to_v2_summary.hpp"
#include "routing_result.hpp"
#include "osrm_fetcher.hpp"
#include "transit_data.hpp"

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
  
  OsrmFetcher::osrmWalkingPort        = programOptions.osrmWalkingPort;
  OsrmFetcher::osrmCyclingPort        = programOptions.osrmCyclingPort;
  OsrmFetcher::osrmDrivingPort        = programOptions.osrmDrivingPort;
  OsrmFetcher::osrmWalkingHost        = programOptions.osrmWalkingHost;
  OsrmFetcher::osrmCyclingHost        = programOptions.osrmCyclingHost;
  OsrmFetcher::osrmDrivingHost        = programOptions.osrmDrivingHost;

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
  TransitData transitData(*fetcher);
  //TODO We wanted to handle error in the constructor, but later part of this code expect a dataStatus
  // leaving as a todo
  DataStatus dataStatus = transitData.getDataStatus();
  
  Calculator calculator(transitData);
  //TODO, should this be in the constructor?
  calculator.initializeCalculationData();
  spdlog::info("preparing server...");

  //HTTP-server using 1 thread
  //Unless you do more heavy non-threaded processing in the resources,
  //1 thread is usually faster than several threads
  HttpServer server;
  server.config.port = programOptions.port;

  // FIXME: Now, all endpoints, including v2 needs this param object to be initialized, so let's do it here, but it should not be necessary (see https://github.com/chairemobilite/trRouting/issues/58)
  calculator.params.setDefaultValues();


  server.resource["^/saveCache[/]?$"]["POST"] = [&server, &calculator](std::shared_ptr<HttpServer::Response> serverResponse, std::shared_ptr<HttpServer::Request> request) {

    std::string response {""};


    try {
      boost::property_tree::ptree pt;
      boost::property_tree::read_json(request->content, pt);

      //auto name = pt.get<string>("firstName") + " " + pt.get<string>("lastName");

      //response = "{\"status\": \"error\", \"error\": \"missing or wrong cache name\"}";
    }
    catch(const std::exception &e) {
      std::string error(e.what());
      response = "{\"status\": \"error\", \"error\": \"" + error + "\"}";
    }


    response = "{\"status\": \"error\", \"error\": \"missing or wrong cache name\"}";

    *serverResponse << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;

  };


  // updateCache:
  server.resource["^/updateCache[/]?$"]["GET"]=[&server, &calculator, &transitData](std::shared_ptr<HttpServer::Response> serverResponse, std::shared_ptr<HttpServer::Request> request) {

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
    calculator.initializeCalculationData();
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
  server.resource["^/exit[/]?\\?([0-9a-zA-Z&=_,:/.-]+)$"]["GET"]=[&server, &calculator](std::shared_ptr<HttpServer::Response> serverResponse, std::shared_ptr<HttpServer::Request> ) {

    std::string response {""};
    *serverResponse << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;

    // todo

  };






  // routing request
  server.resource["^/route/v1/transit[/]?$"]["GET"]=[&server, &calculator, &dataStatus, &transitData](std::shared_ptr<HttpServer::Response> serverResponse, std::shared_ptr<HttpServer::Request> request) {

    std::string response = intializeResponse(dataStatus);

    if (!response.empty())
    {
      *serverResponse << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;
      return;
    }

    try {
      // prepare benchmarking and timer:
      calculator.algorithmCalculationTime.start();
      calculator.benchmarking.clear();

      std::unique_ptr<TrRouting::RoutingResult> routingResult;
      TrRouting::AlternativesResult alternativeResult;

      // prepare parameters:
      std::vector<std::string> parametersWithValues;
      auto queryFields = request->parse_query_string();
      for(auto &field : queryFields)
      {
        parametersWithValues.push_back(field.first + "=" + field.second);
      }

      // clear and initialize benchmarking:
      // TODO create flags to control benchmarking
      {
        calculator.benchmarking["reset"]               = 0;
        calculator.benchmarking["forward_calculation"] = 0;
        //calculator.benchmarking["forward_journey"]     = 0;
        calculator.benchmarking["reverse_calculation"] = 0;
        //calculator.benchmarking["reverse_journey"]     = 0;
        //calculator.benchmarking["generating_results"]  = 0;
      }

      spdlog::info("calculating request: {}", request->path);

      int countOdTripsCalculated {0};

      // update params:
      calculator.params.setDefaultValues();
      RouteParameters routeParams = calculator.params.update(parametersWithValues,
                                                             transitData.getScenarios(),
                                                             transitData.getOdTrips(),
                                                             transitData.getNodes(),
                                                             transitData.getDataSources());

      // find OdTrip if provided:
      bool   foundOdTrip{false};

      calculator.odTripGlob = std::nullopt;

      try {

        if (calculator.params.odTripUuid.has_value() && transitData.getOdTrips().count(calculator.params.odTripUuid.value()))
        {
          calculator.odTripGlob = transitData.getOdTrips().at(calculator.params.odTripUuid.value());
          foundOdTrip = true;
          spdlog::info("od trip uuid {}", to_string(calculator.odTripGlob.value().get().uuid));
          spdlog::info("dts {} ", calculator.odTripGlob.value().get().departureTimeSeconds);
          if (routeParams.isWithAlternatives())
          {
            alternativeResult = calculator.alternativesRouting(routeParams);
            response = ResultToV1Response::resultToJsonString(alternativeResult, routeParams).dump(2);
          }
          else
          {
            routingResult = calculator.calculate(routeParams);
          }
        }
        else if (routeParams.isWithAlternatives())
        {
          alternativeResult = calculator.alternativesRouting(routeParams);
          response = ResultToV1Response::resultToJsonString(alternativeResult, routeParams).dump(2);
        }
        else if (!routeParams.isWithAlternatives() && (calculator.params.calculateAllOdTrips || foundOdTrip ))
        {
          response = calculator.odTripsRouting(routeParams);
        }
        else
        {
          routingResult = calculator.calculate(routeParams);
        }

        if (routingResult.get() != nullptr) {
          response = ResultToV1Response::resultToJsonString(*routingResult.get(), routeParams).dump(2);
        }

        spdlog::debug("-- total -- {} microseconds", calculator.algorithmCalculationTime.getDurationMicrosecondsNoStop());

        for (auto & benchmark : calculator.benchmarking) 
        {
          if (countOdTripsCalculated > 0) {
            //TODO Check units here, ms should me second * 1000 not second/1000
            spdlog::debug("  -- {} -- {} ms ({} per odTrip) -- number od trips: {}",
                          benchmark.first,
                          benchmark.second / 1000,
                          (benchmark.second / countOdTripsCalculated / 1000),
                          countOdTripsCalculated);
          } else {
            //TODO Check units here, ms should me second * 1000 not second/1000
            spdlog::debug("  -- {} -- {} ms", benchmark.first, benchmark.second / 1000);
          }
        }

      } catch (NoRoutingFoundException &e) {
          response = ResultToV1Response::noRoutingFoundResponse(routeParams, e.getReason()).dump(2);
      }

    }
    catch (...)
    {
      response = "{\"status\": \"error\", \"error\": \"Wrong or malformed query\"}";
    }

    *serverResponse << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;

  };

  // Routing request for a single origin destination
  // TODO Copy-pasted and adapted from /route/v1/transit. There's still a lot of common code. Application code should be extracted to common functions outside the web server
  server.resource["^/v2/route[/]?$"]["GET"]=[&server, &calculator, &dataStatus, &transitData](std::shared_ptr<HttpServer::Response> serverResponse, std::shared_ptr<HttpServer::Request> request) {

    std::string response = getFastErrorResponse(dataStatus);

    if (!response.empty()) {
      *serverResponse << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;
      return;
    }

    // prepare benchmarking and timer:
    // TODO Shouldn't have to do this, a query is not a benchmark
    calculator.algorithmCalculationTime.start();
    calculator.benchmarking.clear();

    // prepare parameters:
    std::vector<std::pair<std::string, std::string>> parametersWithValues;
    auto queryFields = request->parse_query_string();
    for(auto &field : queryFields)
    {
      parametersWithValues.push_back(std::make_pair(field.first, field.second));
    }

    spdlog::debug("-- calculating request -- {}", request->path);

    try
    {
      std::unique_ptr<TrRouting::RoutingResult> routingResult;
      TrRouting::AlternativesResult alternativeResult;

      RouteParameters queryParams = RouteParameters::createRouteODParameter(parametersWithValues, transitData.getScenarios());

      try {
        if (queryParams.isWithAlternatives())
        {
          alternativeResult = calculator.alternativesRouting(queryParams);
          response = ResultToV2Response::resultToJsonString(alternativeResult, queryParams).dump(2);
        }
        else
        {
          routingResult = calculator.calculate(queryParams);
          if (routingResult.get() != nullptr) {
            response = ResultToV2Response::resultToJsonString(*routingResult.get(), queryParams).dump(2);
          }
        }

        spdlog::debug("-- total -- {} microseconds", calculator.algorithmCalculationTime.getDurationMicrosecondsNoStop());

      } catch (NoRoutingFoundException &e) {
        response = ResultToV2Response::noRoutingFoundResponse(queryParams, e.getReason()).dump(2);
      }

      *serverResponse << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;

    } catch (ParameterException &exp) {
      response = "{\"status\": \"query_error\", \"errorCode\": \"" + getResponseCode(exp.getType()) + "\"}";
      *serverResponse << "HTTP/1.1 400 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;
    } catch (...) {
      response = "{\"status\": \"query_error\", \"errorCode\": \"PARAM_ERROR_UNKNOWN\"}";
      *serverResponse << "HTTP/1.1 400 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;
    }

  };

  // Request a summary of lines data for a route
  // TODO Copy pasted from v2/route. There's a lot in common, it should be extracted to common class, just the response parser is different
  server.resource["^/v2/summary[/]?$"]["GET"]=[&server, &calculator, &dataStatus, &transitData](std::shared_ptr<HttpServer::Response> serverResponse, std::shared_ptr<HttpServer::Request> request) {

    std::string response = getFastErrorResponse(dataStatus);

    if (!response.empty()) {
      *serverResponse << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;
      return;
    }

    // prepare benchmarking and timer:
    // TODO Shouldn't have to do this, a query is not a benchmark
    calculator.algorithmCalculationTime.start();
    calculator.benchmarking.clear();

    // prepare parameters:
    std::vector<std::pair<std::string, std::string>> parametersWithValues;
    auto queryFields = request->parse_query_string();
    for(auto &field : queryFields)
    {
      parametersWithValues.push_back(std::make_pair(field.first, field.second));
    }

    spdlog::debug("-- calculating request -- {}", request->path);

    try
    {
      std::unique_ptr<TrRouting::RoutingResult> routingResult;
      TrRouting::AlternativesResult alternativeResult;

      RouteParameters queryParams = RouteParameters::createRouteODParameter(parametersWithValues, transitData.getScenarios());

      try {
        if (queryParams.isWithAlternatives())
        {
          alternativeResult = calculator.alternativesRouting(queryParams);
          response = ResultToV2SummaryResponse::resultToJsonString(alternativeResult, queryParams).dump(2);
        }
        else
        {
          routingResult = calculator.calculate(queryParams);
          if (routingResult.get() != nullptr) {
            response = ResultToV2SummaryResponse::resultToJsonString(*routingResult.get(), queryParams).dump(2);
          }
        }

        spdlog::debug("-- total -- {} microseconds", calculator.algorithmCalculationTime.getDurationMicrosecondsNoStop());

      } catch (NoRoutingFoundException &e) {
        response = ResultToV2SummaryResponse::noRoutingFoundResponse(queryParams, e.getReason()).dump(2);
      }

      *serverResponse << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;

    } catch (ParameterException &exp) {
      response = "{\"status\": \"query_error\", \"errorCode\": \"" + getResponseCode(exp.getType()) + "\"}";
      *serverResponse << "HTTP/1.1 400 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << response.length() << "\r\n\r\n" << response;
    } catch (...) {
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









