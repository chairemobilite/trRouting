#include "gtest/gtest.h" // we will add the path to C preprocessor later

//Added for the json-example
#define BOOST_SPIRIT_THREADSAFE

#include <fstream>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/string_generator.hpp>
#include "spdlog/spdlog.h"

#include "calculator.hpp"
#include "cache_fetcher.hpp"
#include "parameters.hpp"
#include "routing_result.hpp"
#include "scenario.hpp"
#include "calculator.hpp"
#include "benchmark_CSA_test.hpp"
#include "osrm_fetcher.hpp"

using namespace TrRouting;

const int NB_ITER = 30;

// Global test suite variables, they should not be reset for each test
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
    OsrmFetcher::osrmWalkingPort = "5000";
    OsrmFetcher::osrmWalkingHost = "localhost"; //"http://localhost";
    OsrmFetcher::osrmCyclingPort = "8000";
    OsrmFetcher::osrmCyclingHost = "localhost";
    OsrmFetcher::osrmDrivingPort = "7000";
    OsrmFetcher::osrmDrivingHost = "localhost";

    CacheFetcher cacheFetcher = TrRouting::CacheFetcher("cache/demo_transition");
    OsrmFetcher::birdDistanceAccessibilityEnabled = true;

    calculator = new TrRouting::Calculator(cacheFetcher);

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

  void benchmarkCurrentParams(TrRouting::RouteParameters &routeParams, bool expectResult, int nbIter)
  {
    // TODO Shouldn't have to do this, a query is not a benchmark
    calculator->params.setDefaultValues();
    calculator->algorithmCalculationTime.start();
    calculator->benchmarking.clear();

    double results[nbIter];
    for (int i = 0; i < nbIter; i++)
    {
      spdlog::info("Benchmark iteration {} of {} ...", i, nbIter);

      auto start = std::chrono::high_resolution_clock::now();

      if (routeParams.isWithAlternatives()) {
        try {
          calculator->alternativesRouting(routeParams);
          ASSERT_TRUE(expectResult);
        } catch (TrRouting::NoRoutingFoundException& e) {
          ASSERT_FALSE(expectResult);
        }
      } else {
        try {
          calculator->calculate(routeParams);
          ASSERT_TRUE(expectResult);
        } catch (TrRouting::NoRoutingFoundException& e) {
          ASSERT_FALSE(expectResult);
        }
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
    const Scenario & scenario = calculator->scenarios.at(scenarioUuid);

    TrRouting::RouteParameters routeParams = TrRouting::RouteParameters(
      std::make_unique<TrRouting::Point>(std::get<parameterIndexes::LAT_ORIG>(paramTuple), std::get<parameterIndexes::LON_ORIG>(paramTuple)),
      std::make_unique<TrRouting::Point>(std::get<parameterIndexes::LAT_DEST>(paramTuple), std::get<parameterIndexes::LON_DEST>(paramTuple)),
      scenario,
      std::get<parameterIndexes::TIME>(paramTuple),
      3 * 60,
      180 * 60,
      20 * 60,
      20 * 60,
      20 * 60,
      15 * 60,
      alternatives,
      forward
    );

    try
    {
      // One line in the detailed results per calculation type
      benchmarkDetailedResultsFile << std::get<parameterIndexes::TEST_DESCRIPTION>(paramTuple) << "," << testType;
      benchmarkCurrentParams(routeParams, std::get<parameterIndexes::EXPECT_RESULTS>(paramTuple), nbIter);
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
