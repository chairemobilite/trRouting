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

#include "toolbox.hpp"
#include "gtfs_fetcher.hpp"
#include "csv_fetcher.hpp"
#include "cache_fetcher.hpp"
#include "calculation_time.hpp"
#include "parameters.hpp"
#include "scenario.hpp"
#include "calculator.hpp"
#include "combinations.hpp"
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

//Added for the default_resource example
void default_resource_send(const HttpServer &server, const std::shared_ptr<HttpServer::Response> &response,
                           const std::shared_ptr<std::ifstream> &ifs);

int main(int argc, char** argv) {
  
  boost::uuids::string_generator uuidGeneratorMain;

  // Set params:
  ProgramOptions programOptions;
  programOptions.parseOptions(argc, argv);
  Parameters algorithmParams;
  algorithmParams.setDefaultValues();
  
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

  // updateCache:
  server.resource["^/updateCache[/]?\\?([0-9a-zA-Z&=_,:/.-]+)$"]["GET"]=[&server, &calculator](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
    
    // todo

    std::string jsonResponse {""};
    *response << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << jsonResponse.length() << "\r\n\r\n" << jsonResponse;

  };

  // closeServer and exit app:
  server.resource["^/exit[/]?\\?([0-9a-zA-Z&=_,:/.-]+)$"]["GET"]=[&server, &calculator](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
    
    std::string jsonResponse {""};
    *response << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << jsonResponse.length() << "\r\n\r\n" << jsonResponse;

    // todo

  };

  // routing request
  server.resource["^/route/v1/transit[/]?\\?([0-9a-zA-Z&=_,:/.-]+)$"]["GET"]=[&server, &calculator](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
    
    // prepare benchmarking and timer:
    calculator.algorithmCalculationTime.start();
    calculator.benchmarking.clear();
    
    // prepare parameters:
    std::vector<std::string> parametersWithValues;
    std::string              queryString;
    if (request->path_match.size() >= 1)
    {
      queryString = request->path_match[1];
    }
    boost::split(parametersWithValues, queryString, boost::is_any_of("&"));
  
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
    std::string jsonResponse {""};
    std::string csvResponse  {""};
    
    // update params:
    calculator.params.update(parametersWithValues, calculator.scenarioIndexesByUuid, calculator.scenarios, calculator.nodeIndexesByUuid);

    if (calculator.params.isCompleteForCalculation())
    {
      
      OdTrip odTrip;
      bool   foundOdTrip{false};
      if (calculator.params.odTripUuid.is_initialized() && calculator.odTripIndexesByUuid.count(calculator.params.odTripUuid))
      {
        odTrip      = calculator.odTrips[calculator.odTripIndexesByUuid[calculator.params.odTripUuid]];
        foundOdTrip = true;
      }

      if (calculator.params.alternatives)
      {
        RoutingResult  routingResult;
        nlohmann::json json;
        nlohmann::json alternativeJson;
        
        if (foundOdTrip)
        {
          std::cout << "od trip uuid " << odTrip.uuid << std::endl;
          calculator.params.origin      = odTrip.origin;
          calculator.params.destination = odTrip.destination;
          calculator.params.odTrip      = &odTrip;
          json["odTripUuid"] = boost::uuids::to_string(odTrip.uuid);
        }
        
        json["alternatives"] = nlohmann::json::array();

        std::vector<int>                 foundLinesIdx;
        std::vector< std::vector<int> >  allCombinations;
        std::map<std::vector<int>, bool> alreadyCalculatedCombinations;
        std::map<std::vector<int>, bool> alreadyFoundLinesIdx;
        std::map<std::vector<int>, int>  foundLinesIdxTravelTimeSeconds;
        std::vector<int>                 combinationsKs;
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
          for(auto lineIdx : routingResult.linesIdx)
          {
            lineShortnames.push_back(calculator.lines[lineIdx].shortname);
          }
          
          alternativeJson = {};
          alternativeJson["status"]                        = routingResult.status;
          alternativeJson["travelTimeSeconds"]             = routingResult.travelTimeSeconds;
          alternativeJson["minimizedTravelTimeSeconds"]    = routingResult.travelTimeSeconds - routingResult.firstWaitingTimeSeconds + calculator.params.minWaitingTimeSeconds;
          alternativeJson["departureTimeSeconds"]          = routingResult.departureTimeSeconds;
          alternativeJson["minimizedDepartureTimeSeconds"] = routingResult.minimizedDepartureTimeSeconds;
          alternativeJson["arrivalTimeSeconds"]            = routingResult.arrivalTimeSeconds;
          alternativeJson["numberOfTransfers"]             = routingResult.numberOfTransfers;
          alternativeJson["inVehicleTravelTimeSeconds"]    = routingResult.inVehicleTravelTimeSeconds;
          alternativeJson["transferTravelTimeSeconds"]     = routingResult.transferTravelTimeSeconds;
          alternativeJson["waitingTimeSeconds"]            = routingResult.waitingTimeSeconds;
          alternativeJson["accessTravelTimeSeconds"]       = routingResult.accessTravelTimeSeconds;
          alternativeJson["egressTravelTimeSeconds"]       = routingResult.egressTravelTimeSeconds;
          alternativeJson["transferWaitingTimeSeconds"]    = routingResult.transferWaitingTimeSeconds;
          alternativeJson["firstWaitingTimeSeconds"]       = routingResult.firstWaitingTimeSeconds;
          alternativeJson["nonTransitTravelTimeSeconds"]   = routingResult.nonTransitTravelTimeSeconds;
          alternativeJson["inVehicleTravelTimesSeconds"]   = routingResult.inVehicleTravelTimesSeconds;
          alternativeJson["lineUuids"]                     = routingResult.lineUuids;
          alternativeJson["lineShortnames"]                = lineShortnames;
          alternativeJson["modeShortnames"]                = routingResult.modeShortnames;
          alternativeJson["agencyUuids"]                   = routingResult.agencyUuids;
          alternativeJson["boardingNodeUuids"]             = routingResult.boardingNodeUuids;
          alternativeJson["unboardingNodeUuids"]           = routingResult.unboardingNodeUuids;
          alternativeJson["tripUuids"]                     = routingResult.tripUuids;
          alternativeJson["alternativeSequence"]           = numAlternatives;
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
          
          foundLinesIdx = routingResult.linesIdx;
          std::stable_sort(foundLinesIdx.begin(),foundLinesIdx.end());
          alreadyFoundLinesIdx[foundLinesIdx]           = true;
          foundLinesIdxTravelTimeSeconds[foundLinesIdx] = routingResult.travelTimeSeconds;
          lastFoundedAtNum = 1;
          //std::cout << "fastest line ids: ";
          //for (auto lineUuid : foundLineUuids)
          //{
          //  std::cout << calculator.lines[calculator.lineIndexesByUuid[lineUuid]].shortname << " ";
          //}
          //std::cout << std::endl;
          combinationsKs.clear();
          for (int i = 1; i <= foundLinesIdx.size(); i++) { combinationsKs.push_back(i); }
          for (auto k : combinationsKs)
          {
            Combinations<int> combinations(foundLinesIdx, k);
            //std::cout << "\nk = " << k << std::endl;
            for (auto newCombination : combinations)
            {
              std::stable_sort(newCombination.begin(), newCombination.end());
              allCombinations.push_back(newCombination);
              alreadyCalculatedCombinations[newCombination] = true;
            }
          }
          
          std::vector<int> combination;
          for (int i = 0; i < allCombinations.size(); i++)
          {
            if (numAlternatives <= maxAlternatives)
            {
              //std:: cout << std::endl << numAlternatives << "." << std::endl;
              combination = allCombinations.at(i);
              calculator.params.exceptLinesIdx = combination;
              //std::cout << "except line Uuids: ";
              //for (auto lineUuid : combination)
              //{
              //  std::cout << calculator.lines[calculator.lineIndexesByUuid[lineUuid]].shortname << " ";
              //}
              //std::cout << std::endl;
              routingResult = calculator.calculate(false, false);
              
              if (routingResult.status == "success")
              {
              
                foundLinesIdx = routingResult.linesIdx;
                std::stable_sort(foundLinesIdx.begin(),foundLinesIdx.end());
                
                if (foundLinesIdx.size() > 0 && alreadyFoundLinesIdx.count(foundLinesIdx) == 0)
                {
                  lineShortnames.clear();
                  for(auto lineIdx : routingResult.linesIdx)
                  {
                    lineShortnames.push_back(calculator.lines[lineIdx].shortname);
                  }
                  
                  numAlternatives += 1;
                  
                  alternativeJson = {};
                  alternativeJson["status"]                        = routingResult.status;
                  alternativeJson["travelTimeSeconds"]             = routingResult.travelTimeSeconds;
                  alternativeJson["minimizedTravelTimeSeconds"]    = routingResult.travelTimeSeconds - routingResult.firstWaitingTimeSeconds + calculator.params.minWaitingTimeSeconds;
                  alternativeJson["departureTimeSeconds"]          = routingResult.departureTimeSeconds;
                  alternativeJson["minimizedDepartureTimeSeconds"] = routingResult.minimizedDepartureTimeSeconds;
                  alternativeJson["arrivalTimeSeconds"]            = routingResult.arrivalTimeSeconds;
                  alternativeJson["numberOfTransfers"]             = routingResult.numberOfTransfers;
                  alternativeJson["inVehicleTravelTimeSeconds"]    = routingResult.inVehicleTravelTimeSeconds;
                  alternativeJson["transferTravelTimeSeconds"]     = routingResult.transferTravelTimeSeconds;
                  alternativeJson["waitingTimeSeconds"]            = routingResult.waitingTimeSeconds;
                  alternativeJson["accessTravelTimeSeconds"]       = routingResult.accessTravelTimeSeconds;
                  alternativeJson["egressTravelTimeSeconds"]       = routingResult.egressTravelTimeSeconds;
                  alternativeJson["transferWaitingTimeSeconds"]    = routingResult.transferWaitingTimeSeconds;
                  alternativeJson["firstWaitingTimeSeconds"]       = routingResult.firstWaitingTimeSeconds;
                  alternativeJson["nonTransitTravelTimeSeconds"]   = routingResult.nonTransitTravelTimeSeconds;
                  alternativeJson["inVehicleTravelTimesSeconds"]   = routingResult.inVehicleTravelTimesSeconds;
                  alternativeJson["lineUuids"]                     = routingResult.lineUuids;
                  alternativeJson["lineShortnames"]                = lineShortnames;
                  alternativeJson["modeShortnames"]                = routingResult.modeShortnames;
                  alternativeJson["agencyUuids"]                   = routingResult.agencyUuids;
                  alternativeJson["boardingNodeUuids"]             = routingResult.boardingNodeUuids;
                  alternativeJson["unboardingNodeUuids"]           = routingResult.unboardingNodeUuids;
                  alternativeJson["tripUuids"]                     = routingResult.tripUuids;
                  alternativeJson["alternativeSequence"]           = numAlternatives;
                  json["alternatives"].push_back(alternativeJson);
                
                  //std::cout << "travelTimeSeconds: " << routingResult.travelTimeSeconds << " line Uuids: ";
                  //for (auto lineUuid : foundLineUuids)
                  //{
                  //  std::cout << calculator.lines[calculator.lineIndexesByUuid[lineUuid]].shortname << " ";
                  //}
                  //std::cout << std::endl;
                  combinationsKs.clear();
                  
                  lastFoundedAtNum = numAlternatives;
                  alreadyFoundLinesIdx[foundLinesIdx] = true;
                  foundLinesIdxTravelTimeSeconds[foundLinesIdx] = routingResult.travelTimeSeconds;
                  for (int i = 1; i <= foundLinesIdx.size(); i++) { combinationsKs.push_back(i); }
                  for (auto k : combinationsKs)
                  {
                    Combinations<int> combinations(foundLinesIdx, k);
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
        
        jsonResponse = json.dump(2);
      }
      
      
      if (!calculator.params.alternatives && (calculator.params.calculateAllOdTrips || foundOdTrip ))
      {
        RoutingResult routingResult;
        //std::map<boost::uuids::uuid, std::map<int, float>> tripsLegsProfile; // parent map key: trip uuid, nested map key: connection sequence, value: number of trips using this connection
        //std::map<boost::uuids::uuid, std::map<int, std::pair<float, std::vector<boost::uuids::uuid>>>> pathsLegsProfile; // parent map key: trip uuid, nested map key: connection sequence, value: number of trips using this connection
        std::map<boost::uuids::uuid, float> lineProfiles; // key: line uuid, value: count od trips using this line
        std::map<boost::uuids::uuid, std::vector<std::vector<float>>> pathProfiles; // key: path uuid, value: [index: segment index, value: [index: hourOfDay, demand]]
        std::map<boost::uuids::uuid, std::vector<float>> pathTotalProfiles; // key: path uuid, value: [index: segment index, value: totalDemand]
        int  legTripIdx;
        int  legLineIdx;
        int  legPathIdx;
        Path legPath;
        int  connectionDepartureTimeSeconds;
        int  connectionDepartureTimeHour;
        bool atLeastOneOdTrip {false};
        bool atLeastOneCompatiblePeriod {false};
        bool attributesMatches {true};
        int  odTripsCount = calculator.odTrips.size();
        float maximumSegmentHourlyDemand = 0.0;
        float maximumSegmentTotalDemand  = 0.0;
        std::string ageGroup;
        
        int legConnectionStartIdx;
        int legConnectionEndIdx;
        
        nlohmann::json json;
        nlohmann::json odTripJson;
        nlohmann::json lineProfilesJson;
        nlohmann::json pathProfilesJson;
        //nlohmann::json pathsOdTripsProfilesSequenceJson;
        //std::vector<unsigned long long> pathsOdTripsProfilesOdTripUuids;
        
        if (calculator.params.responseFormat == "csv" && calculator.params.batchNumber == 1) // write header only on first batch, so we can easily append subsequent batches to the same csv file
        {
          // write csv header:
          csvResponse += "uuid,internalId,status,ageGroup,gender,occupation,destinationActivity,mode,expansionFactor,travelTimeSeconds,onlyWalkingTravelTimeSeconds,"
                         "declaredDepartureTimeSeconds,departureTimeSeconds,minimizedDepartureTimeSeconds,arrivalTimeSeconds,numberOfTransfers,inVehicleTravelTimeSeconds,"
                         "transferTravelTimeSeconds,waitingTimeSeconds,accessTravelTimeSeconds,egressTravelTimeSeconds,transferWaitingTimeSeconds,"
                         "firstWaitingTimeSeconds,nonTransitTravelTimeSeconds,lineUuids,modeShortnames,agencyUuids,boardingNodeUuids,unboardingNodeUuids,tripUuids\n";
        }
        else
        {
          json["odTrips"] = nlohmann::json::array();
        }
                
        int i {0};
        int j {0};
        bool resetFilters {true};

        for (auto & line : calculator.lines)
        {
          lineProfiles[line.uuid] = 0.0;
        }

        std::vector<float> demandByHourOfDay;
        for (int i = 0; i <= 28; i++)
        {
          demandByHourOfDay.push_back(0.0);
        }

        /*for (auto & path : calculator.paths)
        {
          pathProfiles[path.uuid] = std::vector<std::vector<float>>(path.nodesIdx.size() - 1, demandByHourOfDay);
        }*/

        for (auto & odTrip : calculator.odTrips)
        {
          
          if ( i % calculator.params.batchesCount != calculator.params.batchNumber - 1) // when using multiple parallel calculators
          {
            i++;
            continue;
          }
          
          attributesMatches          = true;
          atLeastOneCompatiblePeriod = false;
          
          // verify that od trip matches selected attributes:
          if ( (odTripsAgeGroups.size()   > 0 && std::find(odTripsAgeGroups.begin(), odTripsAgeGroups.end(), calculator.persons[odTrip.personIdx].ageGroup)       == odTripsAgeGroups.end()) 
            || (odTripsGenders.size()     > 0 && std::find(odTripsGenders.begin(), odTripsGenders.end(), calculator.persons[odTrip.personIdx].gender)             == odTripsGenders.end())
            || (odTripsOccupations.size() > 0 && std::find(odTripsOccupations.begin(), odTripsOccupations.end(), calculator.persons[odTrip.personIdx].occupation) == odTripsOccupations.end())
            || (odTripsActivities.size()  > 0 && std::find(odTripsActivities.begin(), odTripsActivities.end(), odTrip.destinationActivity)                        == odTripsActivities.end())
            || (odTripsModes.size()       > 0 && std::find(odTripsModes.begin(), odTripsModes.end(), odTrip.mode)                                                 == odTripsModes.end())
          )
          {
            attributesMatches = false;
          }

          // verify that od trip matches at least one selected period:
          for (auto & period : calculator.params.odTripsPeriods)
          {
            if (odTrip.departureTimeSeconds >= period.first && odTrip.departureTimeSeconds < period.second)
            {
              atLeastOneCompatiblePeriod = true;
            }
          }
          
          if (attributesMatches && (atLeastOneCompatiblePeriod || calculator.params.odTripsPeriods.size() == 0) && (!odTripUuid.is_initialized() || odTripUuid == odTrip.uuid) )
          {

            if (calculator.params.debugDisplay)
            {
            std::cout << "od trip uuid " << odTrip.uuid << " (" << (i+1) << "/" << odTripsCount << ")" << std::endl << " dts: " << odTrip.departureTimeSeconds << " atLeastOneCompatiblePeriod: " << (atLeastOneCompatiblePeriod ? "true " : "false ") << "attributesMatches: " << (attributesMatches ? "true " : "false ") << std::endl;
            }
            else
            {
              std::cout << (i+1) << "/" << odTripsCount << std::endl;
            }
            
            calculator.params.origin      = odTrip.origin;
            calculator.params.destination = odTrip.destination;
            calculator.params.odTrip      = &odTrip;
            routingResult = calculator.calculate(true, resetFilters); // reset filters only on first calculation
            resetFilters  = false;
            countOdTripsCalculated++;
            if (true/*routingResult.status == "success"*/)
            {
              atLeastOneOdTrip = true;
              if (routingResult.legs.size() > 0)
              {
                if (calculator.params.responseFormat != "csv")
                {
                  for (auto & leg : routingResult.legs)
                  {
                    legTripIdx            = std::get<0>(leg);
                    legLineIdx            = std::get<1>(leg);
                    legPathIdx            = std::get<2>(leg);
                    legPath               = calculator.paths[legPathIdx];
                    legConnectionStartIdx = std::get<3>(leg);
                    legConnectionEndIdx   = std::get<4>(leg);
                    lineProfiles[calculator.lines[legLineIdx].uuid] += odTrip.expansionFactor;

                    if (pathProfiles.find(legPath.uuid) == pathProfiles.end())
                    {
                      pathProfiles[legPath.uuid] = std::vector<std::vector<float>>(legPath.nodesIdx.size() - 1, demandByHourOfDay);
                      pathTotalProfiles[legPath.uuid] = std::vector<float>(legPath.nodesIdx.size() - 1, 0.0);
                    }

                    for (int connectionIndex = legConnectionStartIdx; connectionIndex <= legConnectionEndIdx; connectionIndex++)
                    {
                      connectionDepartureTimeSeconds = calculator.tripConnectionDepartureTimes[legTripIdx][connectionIndex];
                      calculator.tripConnectionDemands[legTripIdx][connectionIndex] += odTrip.expansionFactor;
                      connectionDepartureTimeHour    = connectionDepartureTimeSeconds / 3600;
                      //std::cout << "pUuid:" << legPath.uuid << " dth:" << connectionDepartureTimeHour << " cI:" << connectionIndex << " oldD:" << pathProfiles[legPath.uuid][connectionIndex][connectionDepartureTimeHour] << std::endl;
                      pathProfiles[legPath.uuid][connectionIndex][connectionDepartureTimeHour] += odTrip.expansionFactor;
                      pathTotalProfiles[legPath.uuid][connectionIndex] += odTrip.expansionFactor;
                      if (maximumSegmentHourlyDemand < pathProfiles[legPath.uuid][connectionIndex][connectionDepartureTimeHour])
                      {
                        maximumSegmentHourlyDemand = pathProfiles[legPath.uuid][connectionIndex][connectionDepartureTimeHour];
                      }
                      if (maximumSegmentTotalDemand < pathTotalProfiles[legPath.uuid][connectionIndex])
                      {
                        maximumSegmentTotalDemand = pathTotalProfiles[legPath.uuid][connectionIndex];
                      }
                    }
                  }
                }
              }
              
              if (calculator.params.responseFormat == "csv")
              {
                //ageGroup = odTrip.ageGroup;
                //std::replace( ageGroup.begin(), ageGroup.end(), '-', '_' ); // remove dash so Excel does not convert to age groups to numbers...
                csvResponse += boost::uuids::to_string(odTrip.uuid) + ",\"" + odTrip.internalId + "\",\"" + routingResult.status + "\",\"" /*+ ageGroup*/ + "\",\"" /*+ odTrip.gender*/ + "\",\"" /*+ odTrip.occupation*/ + "\",\"";
                csvResponse += odTrip.destinationActivity + "\",\"" + odTrip.mode + "\"," + std::to_string(odTrip.expansionFactor) + "," + std::to_string(routingResult.travelTimeSeconds) + ",";
                csvResponse += std::to_string(odTrip.walkingTravelTimeSeconds) + "," + std::to_string(odTrip.departureTimeSeconds) + "," + std::to_string(routingResult.departureTimeSeconds) + "," + std::to_string(routingResult.minimizedDepartureTimeSeconds) + ",";
                csvResponse += std::to_string(routingResult.arrivalTimeSeconds) + "," + std::to_string(routingResult.numberOfTransfers) + "," + std::to_string(routingResult.inVehicleTravelTimeSeconds) + ",";
                csvResponse += std::to_string(routingResult.transferTravelTimeSeconds) + "," + std::to_string(routingResult.waitingTimeSeconds) + "," + std::to_string(routingResult.accessTravelTimeSeconds) + ",";
                csvResponse += std::to_string(routingResult.egressTravelTimeSeconds) + "," + std::to_string(routingResult.transferWaitingTimeSeconds) + "," + std::to_string(routingResult.firstWaitingTimeSeconds) + ",";
                csvResponse += std::to_string(routingResult.nonTransitTravelTimeSeconds) + ",";
                
                int countLineUuids = routingResult.lineUuids.size();
                j = 0;
                for (auto & lineUuid : routingResult.lineUuids)
                {
                  csvResponse += boost::uuids::to_string(lineUuid);
                  if (j < countLineUuids - 1)
                  {
                    csvResponse += "|";
                  }
                  j++;
                }
                csvResponse += ",";
                j = 0;
                for (auto & modeShortname : routingResult.modeShortnames)
                {
                  csvResponse += modeShortname;
                  if (j < countLineUuids - 1)
                  {
                    csvResponse += "|";
                  }
                  j++;
                }
                csvResponse += ",";
                j = 0;
                for (auto & agencyUuid : routingResult.agencyUuids)
                {
                  csvResponse += boost::uuids::to_string(agencyUuid);
                  if (j < countLineUuids - 1)
                  {
                    csvResponse += "|";
                  }
                  j++;
                }
                csvResponse += ",";
                j = 0;
                for (auto & boardingNodeUuid : routingResult.boardingNodeUuids)
                {
                  csvResponse += boost::uuids::to_string(boardingNodeUuid);
                  if (j < countLineUuids - 1)
                  {
                    csvResponse += "|";
                  }
                  j++;
                }
                csvResponse += ",";
                j = 0;
                for (auto & unboardingNodeUuid : routingResult.unboardingNodeUuids)
                {
                  csvResponse += boost::uuids::to_string(unboardingNodeUuid);
                  if (j < countLineUuids - 1)
                  {
                    csvResponse += "|";
                  }
                  j++;
                }
                csvResponse += ",";
                j = 0;
                for (auto & tripUuid : routingResult.tripUuids)
                {
                  csvResponse += boost::uuids::to_string(tripUuid);
                  if (j < countLineUuids - 1)
                  {
                    csvResponse += "|";
                  }
                  j++;
                }
                csvResponse += "\n";
              }
              else
              {
                odTripJson = {};
                odTripJson["uuid"]                          = boost::uuids::to_string(odTrip.uuid);
                odTripJson["status"]                        = routingResult.status;
                /*odTripJson["ageGroup"]                    = calculator.persons[odTrip.personIdx].ageGroup; // this fails (segmentation fault)...
                odTripJson["gender"]                        = calculator.persons[odTrip.personIdx].gender;
                odTripJson["occupation"]                    = calculator.persons[odTrip.personIdx].occupation;*/
                odTripJson["internalId"]                    = odTrip.internalId;
                odTripJson["originActivity"]                = odTrip.originActivity;
                odTripJson["destinationActivity"]           = odTrip.destinationActivity;
                odTripJson["declaredMode"]                  = odTrip.mode;
                odTripJson["expansionFactor"]               = odTrip.expansionFactor;
                odTripJson["travelTimeSeconds"]             = routingResult.travelTimeSeconds;
                odTripJson["minimizedTravelTimeSeconds"]    = routingResult.travelTimeSeconds - routingResult.firstWaitingTimeSeconds + calculator.params.minWaitingTimeSeconds;
                odTripJson["onlyWalkingTravelTimeSeconds"]  = odTrip.walkingTravelTimeSeconds;
                odTripJson["declaredDepartureTimeSeconds"]  = odTrip.departureTimeSeconds;
                odTripJson["declaredArrivalTimeSeconds"]    = odTrip.arrivalTimeSeconds;
                odTripJson["departureTimeSeconds"]          = routingResult.departureTimeSeconds;
                odTripJson["minimizedDepartureTimeSeconds"] = routingResult.minimizedDepartureTimeSeconds;
                odTripJson["arrivalTimeSeconds"]            = routingResult.arrivalTimeSeconds;
                odTripJson["numberOfTransfers"]             = routingResult.numberOfTransfers;
                odTripJson["inVehicleTravelTimeSeconds"]    = routingResult.inVehicleTravelTimeSeconds;
                odTripJson["transferTravelTimeSeconds"]     = routingResult.transferTravelTimeSeconds;
                odTripJson["waitingTimeSeconds"]            = routingResult.waitingTimeSeconds;
                odTripJson["accessTravelTimeSeconds"]       = routingResult.accessTravelTimeSeconds;
                odTripJson["egressTravelTimeSeconds"]       = routingResult.egressTravelTimeSeconds;
                odTripJson["transferWaitingTimeSeconds"]    = routingResult.transferWaitingTimeSeconds;
                odTripJson["firstWaitingTimeSeconds"]       = routingResult.firstWaitingTimeSeconds;
                odTripJson["nonTransitTravelTimeSeconds"]   = routingResult.nonTransitTravelTimeSeconds;
                odTripJson["lineUuids"]                     = Toolbox::uuidsToStrings(routingResult.lineUuids);
                odTripJson["modesShortnames"]               = routingResult.modeShortnames;
                odTripJson["agencyUuids"]                   = Toolbox::uuidsToStrings(routingResult.agencyUuids);
                odTripJson["boardingNodeUuids"]             = Toolbox::uuidsToStrings(routingResult.boardingNodeUuids);
                odTripJson["unboardingNodeUuids"]           = Toolbox::uuidsToStrings(routingResult.unboardingNodeUuids);
                odTripJson["tripUuids"]                     = Toolbox::uuidsToStrings(routingResult.tripUuids);
                json["odTrips"].push_back(odTripJson);
              }
            }
          }
          i++;
          if (calculator.params.odTripsSampleSize > 0 && i + 1 >= calculator.params.odTripsSampleSize)
          {
            break;
          }
        }
        if (calculator.params.responseFormat != "csv")
        {
          json["maxSegmentHourlyDemand"] = maximumSegmentHourlyDemand;
          json["maxSegmentTotalDemand"]  = maximumSegmentTotalDemand;
          lineProfilesJson = {};
          for (auto & lineCount : lineProfiles)
          {
            lineProfilesJson[boost::uuids::to_string(lineCount.first)] = lineCount.second;
          }
          json["lineProfiles"] = lineProfilesJson;
          
          pathProfilesJson = {};
          for (auto & pathProfile : pathProfiles)
          {
            pathProfilesJson[boost::uuids::to_string(pathProfile.first)] = pathProfile.second;
            //pathsOdTripsProfilesSequenceJson = {};
            /*for (auto & segmentProfile : pathProfile)
            {
              
              //pathsOdTripsProfilesOdTripUuids.clear();
              //for (auto & odTripUuid : std::get<1>(sequenceProfile.second))
              //{
              //  pathsOdTripsProfilesOdTripUuids.push_back()
              //}
              //pathsOdTripsProfilesSequenceJson[std::to_string(sequenceProfile.first)] = {{"demand", std::get<0>(sequenceProfile.second)}, {"odTripUuids", Toolbox::uuidsToStrings(std::get<1>(sequenceProfile.second))}};
            }*/
            //pathsOdTripsProfilesJson[boost::uuids::to_string(pathProfile.first)] = pathsProfile;
          }
          json["pathProfiles"] = pathProfilesJson;
          jsonResponse = json.dump(2);
        }
        if (calculator.params.calculateAllOdTrips && calculator.params.responseFormat == "csv")
        {
          
          if (calculator.params.saveResultToFile)
          {
            std::cerr << "writing csv file" << std::endl;
            std::ofstream csvFile;
            //csvFile.imbue(std::locale("en_US.UTF8"));
            csvFile.open(calculator.params.calculationName + "__batch_" + std::to_string(calculator.params.batchNumber) + "_of_" + std::to_string(calculator.params.batchesCount) + ".csv", std::ios_base::trunc);
            csvFile << csvResponse;
            csvFile.close();
          }
        }
      }
      else if (!calculator.params.alternatives)
      {
        jsonResponse = calculator.calculate().json;
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
      if (calculator.params.saveResultToFile && calculator.params.responseFormat == "json")
      {
        std::cerr << "writing json file" << std::endl;
        std::ofstream jsonFile;
        //jsonFile.imbue(std::locale("en_US.UTF8"));
        jsonFile.open(calculator.params.calculationName + ".json", std::ios_base::trunc);
        jsonFile << jsonResponse;
        jsonFile.close();
      }
      
    }
    else
    {
      jsonResponse = "{\"status\": \"failed\", \"error\": \"Wrong or malformed query\"}";
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
        
    if (calculator.params.responseFormat == "csv")
    {
      *response << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/csv; charset=utf-8\r\nContent-Length: " << csvResponse.length() << "\r\n\r\n" << csvResponse;
    }
    else
    {
      *response << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " << jsonResponse.length() << "\r\n\r\n" << jsonResponse;
    }
    
  };
  
  std::cout << "starting server..." << std::endl;
  std::thread server_thread([&server](){
      server.start();
  });
    
  // Wait for server to start so that the client can connect:
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  
  std::cout << "ready." << std::endl;
  
  server_thread.join();
  
  std::cout << "done..." << std::endl;
  
  return 0;
}



void default_resource_send(const HttpServer &server, const std::shared_ptr<HttpServer::Response> &response, const std::shared_ptr<std::ifstream> &ifs) {
  
  // Read and send 128 KB at a time:
  static std::vector<char> buffer(131072);
  
  // Safe when server is running on one thread:
  std::streamsize read_length;
  if ((read_length = ifs->read(&buffer[0], buffer.size()).gcount())>0)
  {
    response->write(&buffer[0], read_length);
    if (read_length==static_cast<std::streamsize>(buffer.size()))
    {
      server.send(response, [&server, response, ifs](const boost::system::error_code &ec)
      {
        if (!ec)
        {
          default_resource_send(server, response, ifs);
        }
        else
        {
          std::cerr << "Connection interrupted" << std::endl;
        }
      });
    }
  }
}
