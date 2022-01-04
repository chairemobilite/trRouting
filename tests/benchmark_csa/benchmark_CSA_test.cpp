#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "parameters.hpp"

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
#include <bits/stdc++.h>
#include <chrono>
#include <iostream>
#include <fstream>
#include <ctime>

#include "toolbox.hpp"
#include "gtfs_fetcher.hpp"
#include "csv_fetcher.hpp"
#include "cache_fetcher.hpp"
#include "calculation_time.hpp"
#include "parameters.hpp"
#include "scenario.hpp"
#include "calculator.hpp"
#include "program_options.hpp"
#include "benchmark_CSA_test.hpp"
#include "constants.hpp"

using namespace TrRouting;

const int NB_ITER = 30;

// Global test suite variables, they should not be reset for each test
Parameters algorithmParams;
Calculator* calculator;
std::ofstream benchmarkResultsFile;
std::ofstream benchmarkDetailedResultsFile;

/**
 * The benchmarks require cache data to be available in the
 * tests/benchmark_csa/cache/demo_transition directory. The
 * cache data to match the benchmark can be found here:
 * https://nuage.facil.services/s/Gs7bwfEGaBjHCLg Simply unzip in the
 * tests/benchmark_csa/cache directory. It corresponds to the
 * STM's fall 2018 18S_S service (Société des Transports de Montréal).
 * */
class BenchmarkCSATests : public ConstantBenchmarkCSATests
{

protected:

  inline static const boost::uuids::string_generator uuidGenerator;
  // String uuids of various objects
  inline static const boost::uuids::uuid scenarioUuid = uuidGenerator("ed42d920-0349-4f64-8590-4698056c2734");


  static bool updateCalculatorFromCache(Calculator *calculator)
  {
    std::cout << "Preparing calculator..." << std::endl;
    Calculator::DataStatus dataStatus = calculator->prepare();

    if (dataStatus != Calculator::DataStatus::READY)
    {
      std::cout << "Something is wrong with the calculator. Data status: " << (int) dataStatus << std::endl;
      return false;
    }
    return true;
  }

public:

  // Initialize calculator and parameters. Open the result files and add headers
  static void SetUpTestSuite()
  {
    algorithmParams.projectShortname = "demo_transition";
    algorithmParams.cacheDirectoryPath = "cache";
    algorithmParams.dataFetcherShortname = "cache";
    algorithmParams.osrmWalkingPort = "5000";
    algorithmParams.osrmWalkingHost = "localhost"; //"http://localhost";
    algorithmParams.osrmCyclingPort = "8000";
    algorithmParams.osrmCyclingHost = "localhost";
    algorithmParams.osrmDrivingPort = "7000";
    algorithmParams.osrmDrivingHost = "localhost";
    algorithmParams.serverDebugDisplay = false;

    GtfsFetcher gtfsFetcher = GtfsFetcher();
    algorithmParams.gtfsFetcher = &gtfsFetcher;
    CsvFetcher csvFetcher = CsvFetcher();
    algorithmParams.csvFetcher = &csvFetcher;
    CacheFetcher cacheFetcher = TrRouting::CacheFetcher();
    algorithmParams.cacheFetcher = &cacheFetcher;
    algorithmParams.birdDistanceAccessibilityEnabled = true;

    calculator = new TrRouting::Calculator(algorithmParams);

    if (!updateCalculatorFromCache(calculator)) {
      ASSERT_EQ(true, false);
      return;
    }

    // Prepare the result files
    time_t rawtime;
    struct tm * timeinfo;
    char resultFilename[80];
    char detailedResultFilename[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime (resultFilename, 80, "benchmarkResults_%Y%m%d_%H%M.csv", timeinfo);
    strftime (detailedResultFilename, 80, "benchmarkResultsDetailed_%Y%m%d_%H%M.csv", timeinfo);
    benchmarkResultsFile.open (resultFilename, std::ofstream::out);
    benchmarkDetailedResultsFile.open (detailedResultFilename, std::ofstream::out);

    benchmarkResultsFile << ",Forward - no alternatives,Arrival time - no alternatives,Forward - alternatives,Arrival time - alternatives"  << std::endl;

  }

  // Close result files and delete calculator.
  static void TearDownTestSuite()
  {
    benchmarkResultsFile.close();
    benchmarkDetailedResultsFile.close();
    delete calculator;
  }

  bool updateCalculatorParams(Calculator &calculator, std::vector<std::string> &parametersWithValues)
  {
    calculator.params.setDefaultValues();
    calculator.params.update(parametersWithValues,
      calculator.scenarioIndexesByUuid,
      calculator.scenarios,
      calculator.nodeIndexesByUuid,
      calculator.dataSourceIndexesByUuid);
    calculator.params.birdDistanceAccessibilityEnabled = true;

    if (calculator.params.isCompleteForCalculation())
    {
      calculator.origin = &calculator.params.origin;
      calculator.destination = &calculator.params.destination;
      calculator.odTrip = nullptr;

      return true;
    }
    return false;
  }

  void benchmarkCurrentParams(std::vector<std::string> &parametersWithValues, bool expectResult, int nbIter)
  {
    if (calculator == nullptr) {
      throw "Calculator is null";
    }
    double results[nbIter];
    for (int i = 0; i < nbIter; i++)
    {
      if (!updateCalculatorParams(*calculator, parametersWithValues))
      {
        ASSERT_EQ(true, false);
      }

      calculator->algorithmCalculationTime.start();
      calculator->benchmarking.clear();
      auto start = std::chrono::high_resolution_clock::now();

      if (algorithmParams.alternatives) {
        std::string result = calculator->alternativesRouting();
        nlohmann::json json;
        nlohmann::json jsonResult = json.parse(result);
        ASSERT_EQ(expectResult ? STATUS_SUCCESS : STATUS_NO_ROUTING_FOUND, jsonResult["status"]);
      } else {
        TrRouting::RoutingResult result = calculator->calculate();
        ASSERT_EQ(expectResult ? STATUS_SUCCESS : STATUS_NO_ROUTING_FOUND, result.status);
      }
      auto end = std::chrono::high_resolution_clock::now();
      results[i] = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() * 1e-9;
    }

    double resultSum = 0;
    for (int i = 0; i < nbIter; i++)
    {
      benchmarkDetailedResultsFile << "," << std::fixed << results[i] << std::setprecision(9);
      resultSum += results[i];
    }
    benchmarkResultsFile << "," << std::fixed << resultSum / (double)nbIter << std::setprecision(9);
  }

  void benchmarkCurrentData(std::string testType, BenchmarkDataTuple paramTuple, bool alternatives, bool forward, int nbIter)
  {
    std::vector<std::string> parametersWithValues;
    std::pair<std::string, std::string> queryFields[] = {
        std::make_pair("destination", std::to_string(std::get<parameterIndexes::LAT_DEST>(paramTuple)) + "," + std::to_string(std::get<parameterIndexes::LON_DEST>(paramTuple))),
        std::make_pair("alternatives", alternatives ? "1" : "0"),
        std::make_pair("scenario_uuid", "ed42d920-0349-4f64-8590-4698056c2734"),
        std::make_pair("origin", std::to_string(std::get<parameterIndexes::LAT_ORIG>(paramTuple)) + "," + std::to_string(std::get<parameterIndexes::LON_ORIG>(paramTuple))),
        std::make_pair("max_transfer_travel_time_seconds", "600"),
        std::make_pair("max_egress_travel_time_seconds", "900"),
        std::make_pair("max_access_travel_time_seconds", "900"),
        std::make_pair(forward ? "departure_time_seconds" : "arrival_time_seconds", std::to_string(std::get<parameterIndexes::TIME>(paramTuple))),
        std::make_pair("min_waiting_time_seconds", "180")};

    for (auto &field : queryFields)
    {
      parametersWithValues.push_back(field.first + "=" + field.second);
    }

    try
    {
      // One line in the detailed results per calculation type
      benchmarkDetailedResultsFile << std::get<parameterIndexes::TEST_DESCRIPTION>(paramTuple) << "," << testType;
      benchmarkCurrentParams(parametersWithValues, std::get<parameterIndexes::EXPECT_RESULTS>(paramTuple), nbIter);
      benchmarkDetailedResultsFile << std::endl;
      ASSERT_EQ(true, true);
    }
    catch (...)
    {
      std::exception_ptr eptr = std::current_exception(); // capture
      try {
          std::rethrow_exception(eptr);
      } catch(const std::exception& e) {
          std::cout << "Caught exception \"" << e.what() << "\"\n";
      }
      ASSERT_EQ(true, false);
    }
  }

};

// Values are: (description, lon, lat orig, lon, lat dest, time, whether results are expected)
INSTANTIATE_TEST_SUITE_P(
  BenchmarkSimpleQueries, BenchmarkCSATests,
  testing::Values(
    std::make_tuple("Short Central Trip (Promenade Masson to Villeray)", -73.577867, 45.552398, -73.622594, 45.542273, 8 * 60 * 60, true),
    std::make_tuple("Short Central Trip - outside time range", -73.577867, 45.552398, -73.622594, 45.542273, 4 * 60 * 60, false),
    std::make_tuple("Long Central Trip (Ahuntsic to Sud-Ouest)", -73.64579, 45.576736, -73.563364, 45.477621, 12 * 60 * 60, true),
    std::make_tuple("Periphery of the transit network (Eastern bout de l'île)", -73.538513, 45.670863, -73.49693, 45.662265, 12 * 60 * 60, true),
    std::make_tuple("Crossing more than half the transit network territory - morning (From PAT to Lasalle)", -73.487088, 45.693797, -73.636331, 45.421016, 8 * 60 * 60 + 30 * 60, true),
    std::make_tuple("Crossing more than half the transit network territory - noon (From PAT to Lasalle))", -73.487088, 45.693797, -73.636331, 45.421016, 12 * 60 * 60, true),
    std::make_tuple("Crossing more than half the transit network territory - evening (From PAT to Lasalle)", -73.487088, 45.693797, -73.636331, 45.421016, 17 * 60 * 60 + 30 * 60, true),
    std::make_tuple("Crossing more than half the transit network territory - night (From PAT to Lasalle)", -73.487088, 45.693797, -73.636331, 45.421016, 23 * 60 * 60, true),
    std::make_tuple("Crossing more than half the transit network territory - outside time range (From PAT to Lasalle)", -73.487088, 45.693797, -73.636331, 45.421016, 0, false),
    std::make_tuple("West to east of the transit network (Baie d'urfe to RDP)", -73.936445, 45.413567, -73.57128, 45.648183, 10 * 60 * 60, true),
    std::make_tuple("Central Trip - no alternative (Polytechnique to Villeray)", -73.613914, 45.502476, -73.611551, 45.54865, 16 * 60 * 60, true), // Metro only, no possible bus
    std::make_tuple("Short Downtown trip", -73.568486, 45.502466, -73.581496, 45.486854, 14 * 60 * 60, true)
  )
);

TEST_P(BenchmarkCSATests, BenchmarkOriginDestinationQuery)
{
  BenchmarkDataTuple param = GetParam();

  // One line in results file per test data
  benchmarkResultsFile << std::get<parameterIndexes::TEST_DESCRIPTION>(param);
  // No alternatives, forward
  benchmarkCurrentData("Forward - no alternatives", param, false, true, NB_ITER);
  // No alternatives, arrival time
  benchmarkCurrentData("Arrival time - no alternatives", param, false, false, NB_ITER);
  // Alternatives, forward
  benchmarkCurrentData("Forward - alternatives", param, true, true, NB_ITER);
  // Alternatives, arrival time
  benchmarkCurrentData("Arrival time - alternatives", param, true, false, NB_ITER);
  benchmarkResultsFile << std::endl;
}
