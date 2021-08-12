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

#include <boost/optional.hpp>.evaluateRequests();
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
#include "benchmark_CSA_test.hpp"

using namespace TrRouting;
using namespace std;
class BenchmarkCSATests : public ConstantBenchmarkCSATests
{

protected:
  int numberOfRequests = 0;

  Parameters setupAlgorithmParams()
  {
    Parameters algorithmParams;

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

    return algorithmParams;
  }

  bool updateCalculatorFromCache(Calculator *calculator)
  {
    std::cout << "Preparing calculator..." << std::endl;
    int dataStatus = calculator->prepare();

    if (dataStatus < 0)
    {
      std::cout << "Something is wrong with the calculator. Data status: " << dataStatus << std::endl;
      return false;
    }
    if (assertCacheOk(calculator) != 0)
    {
      std::cout << "Invalid cache" << std::endl;
      return false;
    }
    return true;
  }

  int assertCacheOk(Calculator *calculator)
  {
    if (calculator->countAgencies() == 0)
    {
      std::cout << "no agencies";
      return -1;
    }
    else if (calculator->countServices() == 0)
    {
      std::cout << "no services";
      return -1;
    }
    else if (calculator->countNodes() == 0)
    {
      std::cout << "no nodes";
      return -1;
    }
    else if (calculator->countLines() == 0)
    {
      return -1;
    }
    else if (calculator->countPaths() == 0)
    {
      return -1;
    }
    else if (calculator->countScenarios() == 0)
    {
      return -1;
    }
    else if (calculator->countConnections() == 0 || calculator->countTrips() == 0)
    {
      return -1;
    }
    return 0;
  }

  std::vector<std::string> createCalculationQuery()
  {
    std::vector<std::string> parametersWithValues;
    pair<string, string> queryFields[] = {
        make_pair("destination", "45.55239801892435,-73.57786713522127"),
        make_pair("alternatives", "1"),
        make_pair("scenario_uuid", "4b39c2bc-fa43-4ca3-8bae-e21b6c89f1db"),
        make_pair("origin", "45.542273017129446,-73.62259417256861"),
        make_pair("max_transfer_travel_time_seconds", "600"),
        make_pair("max_egress_travel_time_seconds", "900"),
        make_pair("max_access_travel_time_seconds", "900"),
        make_pair("departure_time_seconds", "28800"),
        make_pair("min_waiting_time_seconds", "180")};

    for (auto &field : queryFields)
    {
      parametersWithValues.push_back(field.first + "=" + field.second);
    }
    return parametersWithValues;
  }

  bool updateCalculatorParams(Calculator *calculator, std::vector<std::string> *parametersWithValues)
  {
    calculator->params.setDefaultValues();
    calculator->params.update(*parametersWithValues, calculator->scenarioIndexesByUuid, calculator->scenarios, calculator->nodeIndexesByUuid, calculator->agencyIndexesByUuid, calculator->lineIndexesByUuid, calculator->serviceIndexesByUuid, calculator->modeIndexesByShortname, calculator->dataSourceIndexesByUuid);
    if (calculator->params.isCompleteForCalculation())
    {
      // find OdTrip if provided:
      bool foundOdTrip{false};

      calculator->origin = &calculator->params.origin;
      calculator->destination = &calculator->params.destination;
      calculator->odTrip = nullptr;

      if (calculator->params.odTripUuid.is_initialized() && calculator->odTripIndexesByUuid.count(calculator->params.odTripUuid.get()))
      {
        calculator->odTrip = calculator->odTrips[calculator->odTripIndexesByUuid[calculator->params.odTripUuid.get()]].get();
        foundOdTrip = true;
        std::cout << "od trip uuid " << calculator->odTrip->uuid << std::endl;
        std::cout << "dts " << calculator->odTrip->departureTimeSeconds << std::endl;
        calculator->origin = calculator->odTrip->origin.get();
        calculator->destination = calculator->odTrip->destination.get();
      }

      if (calculator->params.alternatives)
      {
        return true;
      }
    }
    return false;
  }

  void benchmarkCurrentParams(TrRouting::Calculator *calculator)
  {
    int nbIter = 10;
    double results[nbIter];
    for (int i = 0; i < nbIter; i++)
    {
      auto start = chrono::high_resolution_clock::now();
      calculator->alternativesRouting();
      auto end = chrono::high_resolution_clock::now();
      results[i] = chrono::duration_cast<chrono::nanoseconds>(end - start).count() * 1e-9;
    }
    std::cout << "Execution time results: " << std::endl;
    long long resultSum = 0;
    for (int i = 0; i < nbIter; i++)
    {
      std::cout << "Iteration " << i << ": " << fixed << results[i] << setprecision(9) << " seconds" << std::endl;
      resultSum += results[i];
    }
    std::cout << "Average execution time: "<< fixed << resultSum / (double)nbIter << setprecision(9) << " seconds per routing calculation" << std::endl;
  }
};

TEST_F(BenchmarkCSATests, TestGetFilePath)
{

  Parameters algorithmParams = setupAlgorithmParams();

  Calculator calculator(algorithmParams);

  if (!updateCalculatorFromCache(&calculator))
    ASSERT_EQ(true, false);

  calculator.params.setDefaultValues();

  // TODO Shouldn't need to do this, but we do for now, those benchmarks are not the same as those in this program though. Here we loop and have microseconds precision.
  calculator.algorithmCalculationTime.start();
  calculator.benchmarking.clear();

  std::vector<std::string> parametersWithValues = createCalculationQuery();

  if (updateCalculatorParams(&calculator, &parametersWithValues))
  {
    benchmarkCurrentParams(&calculator);
    ASSERT_EQ(true, true);
  }
  else
  {
    ASSERT_EQ(true, false);
  }
}