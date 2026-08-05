#include <errno.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "gtest/gtest.h"
#include "connection_set_test.hpp"
#include "connection_set.hpp"
#include "alternative_filter.hpp"
#include "line.hpp"
#include "trip.hpp"
#include "scenario.hpp"

// Tests for the alternative filters, which mark trips as disabled based on the
// lines excluded by a given alternative combination.
//
// Filters accumulate: runFilter only ever adds to tripsDisabled, so several
// filters can be applied to the same container.

class AlternativeFilterFixtureTests : public ConnectionSetFixtureTests
{
protected:
  // Mirrors Calculator::isTripDisabled: presence in the map means disabled
  static bool isDisabled(const std::unordered_map<TrRouting::Trip::uid_t, bool> & tripsDisabled,
                         const TrRouting::Trip & trip)
  {
    return tripsDisabled.find(trip.uid) != tripsDisabled.end();
  }

  const TrRouting::Line & getLine(const boost::uuids::uuid & uuid) const
  {
    return transitData.getLines().at(uuid);
  }

  const TrRouting::Trip & getTrip(const boost::uuids::uuid & uuid) const
  {
    return transitData.getTrips().at(uuid);
  }

  // All 5 trips of the test data (2 on line SN, 2 on line EW, 1 on line Extra)
  std::shared_ptr<TrRouting::ConnectionSet> getFullConnectionSet() const
  {
    return transitData.getConnectionsForScenario(
      transitData.getScenarios().at(TestDataFetcher::scenarioUuid));
  }
};

// An empty exclusion list should leave every trip enabled
TEST_F(AlternativeFilterFixtureTests, EmptyExcludeListDisablesNothing)
{
  std::shared_ptr<TrRouting::ConnectionSet> connectionSet = getFullConnectionSet();
  ASSERT_EQ(5u, connectionSet->getTrips().size());

  std::vector<std::reference_wrapper<const TrRouting::Line>> excludeLines;
  TrRouting::AlternativeLineFilter filter(excludeLines);

  std::unordered_map<TrRouting::Trip::uid_t, bool> tripsDisabled;
  filter.runFilter(tripsDisabled, *connectionSet);

  EXPECT_EQ(0u, tripsDisabled.size());
}

// Excluding a single line should disable exactly the trips of that line
TEST_F(AlternativeFilterFixtureTests, SingleExcludedLineDisablesItsTripsOnly)
{
  std::shared_ptr<TrRouting::ConnectionSet> connectionSet = getFullConnectionSet();

  std::vector<std::reference_wrapper<const TrRouting::Line>> excludeLines;
  excludeLines.push_back(getLine(TestDataFetcher::lineSNUuid));
  TrRouting::AlternativeLineFilter filter(excludeLines);

  std::unordered_map<TrRouting::Trip::uid_t, bool> tripsDisabled;
  filter.runFilter(tripsDisabled, *connectionSet);

  EXPECT_EQ(2u, tripsDisabled.size());
  EXPECT_TRUE(isDisabled(tripsDisabled, getTrip(TestDataFetcher::trip1SNUuid)));
  EXPECT_TRUE(isDisabled(tripsDisabled, getTrip(TestDataFetcher::trip2SNUuid)));
  EXPECT_FALSE(isDisabled(tripsDisabled, getTrip(TestDataFetcher::trip1EWUuid)));
  EXPECT_FALSE(isDisabled(tripsDisabled, getTrip(TestDataFetcher::trip2EWUuid)));
  EXPECT_FALSE(isDisabled(tripsDisabled, getTrip(TestDataFetcher::trip1ExtraUuid)));
}

// Excluding several lines should disable the union of their trips
TEST_F(AlternativeFilterFixtureTests, MultipleExcludedLinesDisableAllTheirTrips)
{
  std::shared_ptr<TrRouting::ConnectionSet> connectionSet = getFullConnectionSet();

  std::vector<std::reference_wrapper<const TrRouting::Line>> excludeLines;
  excludeLines.push_back(getLine(TestDataFetcher::lineSNUuid));
  excludeLines.push_back(getLine(TestDataFetcher::lineExtraUuid));
  TrRouting::AlternativeLineFilter filter(excludeLines);

  std::unordered_map<TrRouting::Trip::uid_t, bool> tripsDisabled;
  filter.runFilter(tripsDisabled, *connectionSet);

  EXPECT_EQ(3u, tripsDisabled.size());
  EXPECT_TRUE(isDisabled(tripsDisabled, getTrip(TestDataFetcher::trip1SNUuid)));
  EXPECT_TRUE(isDisabled(tripsDisabled, getTrip(TestDataFetcher::trip2SNUuid)));
  EXPECT_TRUE(isDisabled(tripsDisabled, getTrip(TestDataFetcher::trip1ExtraUuid)));
  EXPECT_FALSE(isDisabled(tripsDisabled, getTrip(TestDataFetcher::trip1EWUuid)));
  EXPECT_FALSE(isDisabled(tripsDisabled, getTrip(TestDataFetcher::trip2EWUuid)));
}

// Excluding every line should disable every trip of the connection set
TEST_F(AlternativeFilterFixtureTests, AllLinesExcludedDisablesAllTrips)
{
  std::shared_ptr<TrRouting::ConnectionSet> connectionSet = getFullConnectionSet();

  std::vector<std::reference_wrapper<const TrRouting::Line>> excludeLines;
  excludeLines.push_back(getLine(TestDataFetcher::lineSNUuid));
  excludeLines.push_back(getLine(TestDataFetcher::lineEWUuid));
  excludeLines.push_back(getLine(TestDataFetcher::lineExtraUuid));
  TrRouting::AlternativeLineFilter filter(excludeLines);

  std::unordered_map<TrRouting::Trip::uid_t, bool> tripsDisabled;
  filter.runFilter(tripsDisabled, *connectionSet);

  EXPECT_EQ(connectionSet->getTrips().size(), tripsDisabled.size());
  for (auto & tripIte : connectionSet->getTrips())
  {
    EXPECT_TRUE(isDisabled(tripsDisabled, tripIte.get()));
  }
}

// Excluding a line that has no trip in this connection set should be a no-op.
// Scenario 2 already excludes line EW, so no trip of that line is present
TEST_F(AlternativeFilterFixtureTests, ExcludedLineAbsentFromConnectionSet)
{
  std::shared_ptr<TrRouting::ConnectionSet> connectionSet =
    transitData.getConnectionsForScenario(
      transitData.getScenarios().at(TestDataFetcher::scenario2Uuid));
  ASSERT_EQ(3u, connectionSet->getTrips().size());

  std::vector<std::reference_wrapper<const TrRouting::Line>> excludeLines;
  excludeLines.push_back(getLine(TestDataFetcher::lineEWUuid));
  TrRouting::AlternativeLineFilter filter(excludeLines);

  std::unordered_map<TrRouting::Trip::uid_t, bool> tripsDisabled;
  filter.runFilter(tripsDisabled, *connectionSet);

  EXPECT_EQ(0u, tripsDisabled.size());
}

// Successive filters accumulate: the second filter must add to the results of
// the first rather than replace them
TEST_F(AlternativeFilterFixtureTests, SuccessiveFiltersAccumulate)
{
  std::shared_ptr<TrRouting::ConnectionSet> connectionSet = getFullConnectionSet();

  std::unordered_map<TrRouting::Trip::uid_t, bool> tripsDisabled;

  std::vector<std::reference_wrapper<const TrRouting::Line>> firstExcludeLines;
  firstExcludeLines.push_back(getLine(TestDataFetcher::lineSNUuid));
  TrRouting::AlternativeLineFilter firstFilter(firstExcludeLines);
  firstFilter.runFilter(tripsDisabled, *connectionSet);
  ASSERT_EQ(2u, tripsDisabled.size());

  std::vector<std::reference_wrapper<const TrRouting::Line>> secondExcludeLines;
  secondExcludeLines.push_back(getLine(TestDataFetcher::lineExtraUuid));
  TrRouting::AlternativeLineFilter secondFilter(secondExcludeLines);
  secondFilter.runFilter(tripsDisabled, *connectionSet);

  // Both the SN trips and the Extra trip must now be disabled
  EXPECT_EQ(3u, tripsDisabled.size());
  EXPECT_TRUE(isDisabled(tripsDisabled, getTrip(TestDataFetcher::trip1SNUuid)));
  EXPECT_TRUE(isDisabled(tripsDisabled, getTrip(TestDataFetcher::trip2SNUuid)));
  EXPECT_TRUE(isDisabled(tripsDisabled, getTrip(TestDataFetcher::trip1ExtraUuid)));
  EXPECT_FALSE(isDisabled(tripsDisabled, getTrip(TestDataFetcher::trip1EWUuid)));
  EXPECT_FALSE(isDisabled(tripsDisabled, getTrip(TestDataFetcher::trip2EWUuid)));
}

// Accumulating in either order must give the same result, so that the caller
// does not have to care about the order the filters are applied in
TEST_F(AlternativeFilterFixtureTests, AccumulationIsOrderIndependent)
{
  std::shared_ptr<TrRouting::ConnectionSet> connectionSet = getFullConnectionSet();

  std::vector<std::reference_wrapper<const TrRouting::Line>> snExcludeLines;
  snExcludeLines.push_back(getLine(TestDataFetcher::lineSNUuid));
  TrRouting::AlternativeLineFilter snFilter(snExcludeLines);

  std::vector<std::reference_wrapper<const TrRouting::Line>> extraExcludeLines;
  extraExcludeLines.push_back(getLine(TestDataFetcher::lineExtraUuid));
  TrRouting::AlternativeLineFilter extraFilter(extraExcludeLines);

  std::unordered_map<TrRouting::Trip::uid_t, bool> snThenExtra;
  snFilter.runFilter(snThenExtra, *connectionSet);
  extraFilter.runFilter(snThenExtra, *connectionSet);

  std::unordered_map<TrRouting::Trip::uid_t, bool> extraThenSn;
  extraFilter.runFilter(extraThenSn, *connectionSet);
  snFilter.runFilter(extraThenSn, *connectionSet);

  EXPECT_EQ(snThenExtra.size(), extraThenSn.size());
  for (auto & tripIte : connectionSet->getTrips())
  {
    const TrRouting::Trip & trip = tripIte.get();
    EXPECT_EQ(isDisabled(snThenExtra, trip), isDisabled(extraThenSn, trip));
  }
}

// Overlapping filters must not double count or drop anything
TEST_F(AlternativeFilterFixtureTests, OverlappingFiltersAccumulateToTheUnion)
{
  std::shared_ptr<TrRouting::ConnectionSet> connectionSet = getFullConnectionSet();

  std::unordered_map<TrRouting::Trip::uid_t, bool> tripsDisabled;

  std::vector<std::reference_wrapper<const TrRouting::Line>> firstExcludeLines;
  firstExcludeLines.push_back(getLine(TestDataFetcher::lineSNUuid));
  firstExcludeLines.push_back(getLine(TestDataFetcher::lineEWUuid));
  TrRouting::AlternativeLineFilter firstFilter(firstExcludeLines);
  firstFilter.runFilter(tripsDisabled, *connectionSet);
  ASSERT_EQ(4u, tripsDisabled.size());

  // Line SN is already disabled, line Extra is not
  std::vector<std::reference_wrapper<const TrRouting::Line>> secondExcludeLines;
  secondExcludeLines.push_back(getLine(TestDataFetcher::lineSNUuid));
  secondExcludeLines.push_back(getLine(TestDataFetcher::lineExtraUuid));
  TrRouting::AlternativeLineFilter secondFilter(secondExcludeLines);
  secondFilter.runFilter(tripsDisabled, *connectionSet);

  EXPECT_EQ(5u, tripsDisabled.size());
}

// The filter must never remove entries it did not add, so that results set by
// the caller or by a previous filter survive
TEST_F(AlternativeFilterFixtureTests, ExistingResultsArePreserved)
{
  std::shared_ptr<TrRouting::ConnectionSet> connectionSet = getFullConnectionSet();

  std::unordered_map<TrRouting::Trip::uid_t, bool> tripsDisabled;
  tripsDisabled[getTrip(TestDataFetcher::trip1EWUuid).uid] = true;

  std::vector<std::reference_wrapper<const TrRouting::Line>> excludeLines;
  excludeLines.push_back(getLine(TestDataFetcher::lineSNUuid));
  TrRouting::AlternativeLineFilter filter(excludeLines);
  filter.runFilter(tripsDisabled, *connectionSet);

  EXPECT_EQ(3u, tripsDisabled.size());
  EXPECT_TRUE(isDisabled(tripsDisabled, getTrip(TestDataFetcher::trip1EWUuid)));
  EXPECT_TRUE(isDisabled(tripsDisabled, getTrip(TestDataFetcher::trip1SNUuid)));
  EXPECT_TRUE(isDisabled(tripsDisabled, getTrip(TestDataFetcher::trip2SNUuid)));
}

// An empty exclusion list must be a no-op, in particular it must not clear
TEST_F(AlternativeFilterFixtureTests, EmptyExcludeListPreservesExistingResults)
{
  std::shared_ptr<TrRouting::ConnectionSet> connectionSet = getFullConnectionSet();

  std::unordered_map<TrRouting::Trip::uid_t, bool> tripsDisabled;
  tripsDisabled[getTrip(TestDataFetcher::trip1SNUuid).uid] = true;

  std::vector<std::reference_wrapper<const TrRouting::Line>> excludeLines;
  TrRouting::AlternativeLineFilter filter(excludeLines);
  filter.runFilter(tripsDisabled, *connectionSet);

  EXPECT_EQ(1u, tripsDisabled.size());
  EXPECT_TRUE(isDisabled(tripsDisabled, getTrip(TestDataFetcher::trip1SNUuid)));
}

// Running the same filter twice must be idempotent: accumulating means the
// second run re-adds the same uids rather than growing the result
TEST_F(AlternativeFilterFixtureTests, RunFilterIsIdempotent)
{
  std::shared_ptr<TrRouting::ConnectionSet> connectionSet = getFullConnectionSet();

  std::vector<std::reference_wrapper<const TrRouting::Line>> excludeLines;
  excludeLines.push_back(getLine(TestDataFetcher::lineEWUuid));
  TrRouting::AlternativeLineFilter filter(excludeLines);

  std::unordered_map<TrRouting::Trip::uid_t, bool> tripsDisabled;
  filter.runFilter(tripsDisabled, *connectionSet);
  ASSERT_EQ(2u, tripsDisabled.size());

  filter.runFilter(tripsDisabled, *connectionSet);

  EXPECT_EQ(2u, tripsDisabled.size());
  EXPECT_TRUE(isDisabled(tripsDisabled, getTrip(TestDataFetcher::trip1EWUuid)));
  EXPECT_TRUE(isDisabled(tripsDisabled, getTrip(TestDataFetcher::trip2EWUuid)));
}
