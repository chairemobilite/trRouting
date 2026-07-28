#include <drogon/drogon.h>
#include <trantor/utils/ConcurrentTaskQueue.h>

#include <vector>
#include <algorithm>
#include <string>
#include <iterator>
#include <atomic>
#include <functional>
#include "spdlog/spdlog.h"

#include <boost/uuid/uuid.hpp>
#include <boost/program_options.hpp>
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
#ifdef HAVE_MEMCACHED
  #include "memcachedgeofilter.hpp"
#endif

using namespace TrRouting;

typedef std::function<void(const drogon::HttpResponsePtr &)> HandlerCallback;
typedef std::function<void(const drogon::HttpRequestPtr &, HandlerCallback &&)> Handler;

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

// Build a JSON response with the same headers the previous implementation
// streamed by hand (CORS + content type). Content-Length is handled by Drogon.
static drogon::HttpResponsePtr makeJsonResponse(const std::string &body,
                                                drogon::HttpStatusCode code = drogon::k200OK)
{
  auto resp = drogon::HttpResponse::newHttpResponse();
  resp->setStatusCode(code);
  resp->setContentTypeString("application/json; charset=utf-8");
  resp->addHeader("Access-Control-Allow-Origin", "*");
  resp->setBody(body);
  return resp;
}

// Query parameters as the vector of pairs expected by the parameter parsers
static std::vector<std::pair<std::string, std::string>> extractParameters(const drogon::HttpRequestPtr &request)
{
  std::vector<std::pair<std::string, std::string>> parametersWithValues;
  for (const auto &field : request->getParameters())
  {
    parametersWithValues.push_back(std::make_pair(field.first, field.second));
  }
  return parametersWithValues;
}

// Run a calculation on the compute pool and reply through the Drogon callback.
// The compute function returns the JSON response body; ParameterException and
// unknown exceptions are mapped to the same 400 responses as before.
static void runCalculation(trantor::ConcurrentTaskQueue &computePool,
                           const std::string &name,
                           HandlerCallback &&callback,
                           std::function<std::string()> compute)
{
  computePool.runTaskInQueue(
    [name, callback = std::move(callback), compute = std::move(compute)]() {
      try
      {
        callback(makeJsonResponse(compute()));
      } catch (ParameterException &exp) {
        auto responseCode = getResponseCode(exp.getType());
        spdlog::info("-- parameter exception in {} calculation -- {}", name, responseCode);
        std::string response = "{\"status\": \"query_error\", \"errorCode\": \"" + responseCode + "\"}";
        callback(makeJsonResponse(response, drogon::k400BadRequest));
      } catch (const std::exception &e) {
        spdlog::error("-- unknown exception in {} calculation -- {}", name, e.what());
        std::string response = "{\"status\": \"query_error\", \"errorCode\": \"PARAM_ERROR_UNKNOWN\"}";
        callback(makeJsonResponse(response, drogon::k400BadRequest));
      }
    });
}

// Return the Calculator owned by the calling compute thread, creating it on
// first use. Constructing a Calculator per request allocated (and immediately
// freed) network-sized containers, which dominated system time under load.
// The compute pool threads live for the whole process, and a task always runs
// to completion on a single thread, so one instance per thread is safe and
// lets reset() reuse warm allocations instead of faulting in fresh pages.
// Note: this must not be called from the Drogon IO loop threads, only from
// tasks queued on the compute pool.
static Calculator & getThreadCalculator(const TransitData &transitData, GeoFilter &geoFilter)
{
  thread_local Calculator calculator(transitData, geoFilter);
  // Start the timer at the beginning of each request.
  // TODO Maybe we should move the timer out of the calculator
  calculator.startRequestTimer();
  return calculator;
}

// Register a GET handler for both /path and /path/ to keep the behavior of
// the previous "[/]?" route regexes
static void registerGet(const std::string &path, Handler handler)
{
  drogon::app().registerHandler(path, Handler(handler), {drogon::Get});
  drogon::app().registerHandler(path + "/", std::move(handler), {drogon::Get});
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

  // Wrap the geoFilter with memcached if requested and available
  if (programOptions.useMemcached) {
  #ifdef HAVE_MEMCACHED
    geoFilter = new TrRouting::MemcachedGeoFilter(geoFilter, programOptions.memcachedServers, 3600, programOptions.numberOfThreads);
    spdlog::info("Using memcached for caching GeoFilter results with server(s): {}", programOptions.memcachedServers);
    // Don't delete the original filter, as it's now managed by the cached filter
  #else
    spdlog::warn("Memcached support was requested but is not available (not compiled in). Continuing without caching.");
  #endif
  }

  spdlog::info("preparing server with {} compute threads...", programOptions.numberOfThreads);

  // Compute pool for the CSA calculations. This is where --threads goes now:
  // the Drogon IO loops only accept connections and parse HTTP, all heavy
  // work is dispatched here so IO is never blocked by a calculation.
  trantor::ConcurrentTaskQueue computePool(programOptions.numberOfThreads, "csa-compute");

  // updateCache:
  registerGet("/updateCache",
    [&transitData](const drogon::HttpRequestPtr &request, HandlerCallback &&callback) {

    std::string              response {""};
    std::string              customCacheDirectoryPath {""};
    std::string              cacheNamesStr {""};
    std::vector<std::string> cacheNames;
    std::vector<std::string> cacheNamesVector;

    // prepare parameters:
    for (const auto &field : request->getParameters())
    {
      const std::string &parameterName = field.first;

      if (parameterName == "names" || parameterName == "caches" || parameterName == "cache_names" || parameterName == "name" || parameterName == "cache" || parameterName == "cache_name")
      {
        boost::split(cacheNamesVector, field.second, boost::is_any_of(","));
        for(std::string cacheName : cacheNamesVector)
        {
          cacheNames.push_back(cacheName);
        }
        continue;
      }
      if (parameterName == "path" || parameterName == "custom_path" || parameterName == "custom_cache_path")
      {
        customCacheDirectoryPath = field.second;
        continue;
      }
    }

    bool correctCacheName {false};
    //TODO Merge this and the preparations.cpp code
    for(std::string cacheName : cacheNames)
    {
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

    callback(makeJsonResponse(response));

  });

  // closeServer and exit app:
  registerGet("/exit",
    [](const drogon::HttpRequestPtr &, HandlerCallback &&callback) {

    callback(makeJsonResponse(""));

    // todo (drogon::app().quit() will stop the event loops and return from run())

  });

  // Routing request for a single origin destination
  registerGet("/v2/route",
    [&dataStatus, &transitData, &geoFilter, &computePool](const drogon::HttpRequestPtr &request, HandlerCallback &&callback) {
    // Have a global id to match the requests in the logs
    static std::atomic<int> routeRequestId {0};

    std::string fastError = getFastErrorResponse(dataStatus);
    if (!fastError.empty()) {
      callback(makeJsonResponse(fastError));
      return;
    }

    auto parametersWithValues = extractParameters(request);
    int currentRequestId = routeRequestId++;

    runCalculation(computePool, "route", std::move(callback),
      [parametersWithValues = std::move(parametersWithValues), currentRequestId, &transitData, &geoFilter]() -> std::string {

      spdlog::info("-- calculating route request -- {}", currentRequestId);

      Calculator &calculator = getThreadCalculator(transitData, *geoFilter);
      RouteParameters queryParams = RouteParameters::createRouteODParameter(parametersWithValues, transitData.getScenarios());

      try {
        std::string response;
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
        return response;

      } catch (NoRoutingFoundException &e) {
        spdlog::info("-- route request not found -- {}", currentRequestId);
        return ResultToV2Response::noRoutingFoundResponse(queryParams, e.getReason()).dump(2);
      }
    });
  });

  // Request a summary of lines data for a route
  registerGet("/v2/summary",
    [&dataStatus, &transitData, &geoFilter, &computePool](const drogon::HttpRequestPtr &request, HandlerCallback &&callback) {
    // Have a global id to match the requests in the logs
    static std::atomic<int> summaryRequestId {0};

    std::string fastError = getFastErrorResponse(dataStatus);
    if (!fastError.empty()) {
      callback(makeJsonResponse(fastError));
      return;
    }

    auto parametersWithValues = extractParameters(request);
    int currentRequestId = summaryRequestId++;

    runCalculation(computePool, "summary", std::move(callback),
      [parametersWithValues = std::move(parametersWithValues), currentRequestId, &transitData, &geoFilter]() -> std::string {

      spdlog::info("-- calculating summary request -- {}", currentRequestId);

      Calculator &calculator = getThreadCalculator(transitData, *geoFilter);
      RouteParameters queryParams = RouteParameters::createRouteODParameter(parametersWithValues, transitData.getScenarios());

      try {
        std::string response;
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
        return response;

      } catch (NoRoutingFoundException &e) {
        spdlog::info("-- summary request not found -- {}", currentRequestId);
        return ResultToV2SummaryResponse::noRoutingFoundResponse(queryParams, e.getReason()).dump(2);
      }
    });
  });

  // Accessibility map from/to a single point
  registerGet("/v2/accessibility",
    [&dataStatus, &transitData, &geoFilter, &computePool](const drogon::HttpRequestPtr &request, HandlerCallback &&callback) {
    // Have a global id to match the requests in the logs
    static std::atomic<int> accessibilityRequestId {0};

    std::string fastError = getFastErrorResponse(dataStatus);
    if (!fastError.empty()) {
      callback(makeJsonResponse(fastError));
      return;
    }

    auto parametersWithValues = extractParameters(request);
    int currentRequestId = accessibilityRequestId++;

    runCalculation(computePool, "accessibility", std::move(callback),
      [parametersWithValues = std::move(parametersWithValues), currentRequestId, &transitData, &geoFilter]() -> std::string {

      spdlog::info("-- calculating accessibility request -- {}", currentRequestId);

      Calculator &calculator = getThreadCalculator(transitData, *geoFilter);
      AccessibilityParameters queryParams = AccessibilityParameters::createAccessibilityParameter(parametersWithValues, transitData.getScenarios());

      try {
        std::string response;
        std::unique_ptr<AllNodesResult> accessibilityResult = calculator.calculateAllNodes(queryParams);
        if (accessibilityResult.get() != nullptr) {
          response = ResultToV2AccessibilityResponse::resultToJsonString(*accessibilityResult.get(), queryParams).dump(2);
        }

        spdlog::info("-- accessibility request complete -- {}", currentRequestId);
        return response;

      } catch (NoRoutingFoundException &e) {
        spdlog::info("-- accessibility request not found -- {}", currentRequestId);
        return ResultToV2AccessibilityResponse::noRoutingFoundResponse(queryParams, e.getReason()).dump(2);
      }
    });
  });

  // Default handler for unmatched paths (replaces default_resource)
  drogon::app().setDefaultHandler(
    [](const drogon::HttpRequestPtr &request, HandlerCallback &&callback) {
    spdlog::info("unmatched request: {}", request->path());

    callback(makeJsonResponse("{\"status\": \"error\", \"error\": \"missing params\"}"));
  });

  spdlog::info("starting server...");

  drogon::app().enableReusePort(programOptions.enableReusePort);
  drogon::app()
    .addListener("0.0.0.0", programOptions.port)
    .setThreadNum(4) // IO event loops only; calculations run on the compute pool
    .setIdleConnectionTimeout(1200)  // by default drogon kick an idle connection after 60 seconds
                                     // but we have calculation that can run longer
                                     // For now set a really long time to still clean up stale connections
    .registerBeginningAdvice([]() {
      spdlog::info("ready.");
    })
    .run(); // blocks until drogon::app().quit()

  // Cleanup
  delete fetcher;

  return 0;
}
