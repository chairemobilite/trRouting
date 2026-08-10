#include <errno.h>
#include <algorithm>
#include <vector>
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
// Filters accumulate: runFilter only ever sets disabled flags, so several
// filters can be applied to the same overlay.

class AlternativeFilterFixtureTests : public ConnectionSetFixtureTests
{
protected:
  // A fresh overlay, sized and zeroed the same way Calculator::reset does it
  static std::vector<TrRouting::TripQueryData> makeOverlay()
  {
    return std::vector<TrRouting::TripQueryData>(TrRouting::Trip::getMaxUid() + 1);
  }

  static bool isDisabled(const std::vector<TrRouting::TripQueryData> & tripsQueryOverlay,
                         const TrRouting::Trip & trip)
  {
    return tripsQueryOverlay.at(trip.uid).disabled;
  }

  static size_t countDisabled(const std::vector<TrRouting::TripQueryData> & tripsQueryOverlay)
  {
    return std::count_if(tripsQueryOverlay.begin(), tripsQueryOverlay.end(),
                         [](const TrRouting::TripQueryData & data) { return data.disabled; });
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

  std::vector<TrRouting::TripQueryData> tripsQueryOverlay = makeOverlay();
  filter.runFilter(tripsQueryOverlay, *connectionSet);

  EXPECT_EQ(0u, countDisabled(tripsQueryOverlay));
}

// Excluding a single line should disable exactly the trips of that line
TEST_F(AlternativeFilterFixtureTests, SingleExcludedLineDisablesItsTripsOnly)
{
  std::shared_ptr<TrRouting::ConnectionSet> connectionSet = getFullConnectionSet();

  std::vector<std::reference_wrapper<const TrRouting::Line>> excludeLines;
  excludeLines.push_back(getLine(TestDataFetcher::lineSNUuid));
  TrRouting::AlternativeLineFilter filter(excludeLines);

  std::vector<TrRouting::TripQueryData> tripsQueryOverlay = makeOverlay();
  filter.runFilter(tripsQueryOverlay, *connectionSet);

  EXPECT_EQ(2u, countDisabled(tripsQueryOverlay));
  EXPECT_TRUE(isDisabled(tripsQueryOverlay, getTrip(TestDataFetcher::trip1SNUuid)));
  EXPECT_TRUE(isDisabled(tripsQueryOverlay, getTrip(TestDataFetcher::trip2SNUuid)));
  EXPECT_FALSE(isDisabled(tripsQueryOverlay, getTrip(TestDataFetcher::trip1EWUuid)));
  EXPECT_FALSE(isDisabled(tripsQueryOverlay, getTrip(TestDataFetcher::trip2EWUuid)));
  EXPECT_FALSE(isDisabled(tripsQueryOverlay, getTrip(TestDataFetcher::trip1ExtraUuid)));
}

// Excluding several lines should disable the union of their trips
TEST_F(AlternativeFilterFixtureTests, MultipleExcludedLinesDisableAllTheirTrips)
{
  std::shared_ptr<TrRouting::ConnectionSet> connectionSet = getFullConnectionSet();

  std::vector<std::reference_wrapper<const TrRouting::Line>> excludeLines;
  excludeLines.push_back(getLine(TestDataFetcher::lineSNUuid));
  excludeLines.push_back(getLine(TestDataFetcher::lineExtraUuid));
  TrRouting::AlternativeLineFilter filter(excludeLines);

  std::vector<TrRouting::TripQueryData> tripsQueryOverlay = makeOverlay();
  filter.runFilter(tripsQueryOverlay, *connectionSet);

  EXPECT_EQ(3u, countDisabled(tripsQueryOverlay));
  EXPECT_TRUE(isDisabled(tripsQueryOverlay, getTrip(TestDataFetcher::trip1SNUuid)));
  EXPECT_TRUE(isDisabled(tripsQueryOverlay, getTrip(TestDataFetcher::trip2SNUuid)));
  EXPECT_TRUE(isDisabled(tripsQueryOverlay, getTrip(TestDataFetcher::trip1ExtraUuid)));
  EXPECT_FALSE(isDisabled(tripsQueryOverlay, getTrip(TestDataFetcher::trip1EWUuid)));
  EXPECT_FALSE(isDisabled(tripsQueryOverlay, getTrip(TestDataFetcher::trip2EWUuid)));
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

  std::vector<TrRouting::TripQueryData> tripsQueryOverlay = makeOverlay();
  filter.runFilter(tripsQueryOverlay, *connectionSet);

  EXPECT_EQ(connectionSet->getTrips().size(), countDisabled(tripsQueryOverlay));
  for (auto & tripIte : connectionSet->getTrips())
  {
    EXPECT_TRUE(isDisabled(tripsQueryOverlay, tripIte.get()));
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

  std::vector<TrRouting::TripQueryData> tripsQueryOverlay = makeOverlay();
  filter.runFilter(tripsQueryOverlay, *connectionSet);

  EXPECT_EQ(0u, countDisabled(tripsQueryOverlay));
}

// Successive filters accumulate: the second filter must add to the results of
// the first rather than replace them
TEST_F(AlternativeFilterFixtureTests, SuccessiveFiltersAccumulate)
{
  std::shared_ptr<TrRouting::ConnectionSet> connectionSet = getFullConnectionSet();

  std::vector<TrRouting::TripQueryData> tripsQueryOverlay = makeOverlay();

  std::vector<std::reference_wrapper<const TrRouting::Line>> firstExcludeLines;
  firstExcludeLines.push_back(getLine(TestDataFetcher::lineSNUuid));
  TrRouting::AlternativeLineFilter firstFilter(firstExcludeLines);
  firstFilter.runFilter(tripsQueryOverlay, *connectionSet);
  ASSERT_EQ(2u, countDisabled(tripsQueryOverlay));

  std::vector<std::reference_wrapper<const TrRouting::Line>> secondExcludeLines;
  secondExcludeLines.push_back(getLine(TestDataFetcher::lineExtraUuid));
  TrRouting::AlternativeLineFilter secondFilter(secondExcludeLines);
  secondFilter.runFilter(tripsQueryOverlay, *connectionSet);

  // Both the SN trips and the Extra trip must now be disabled
  EXPECT_EQ(3u, countDisabled(tripsQueryOverlay));
  EXPECT_TRUE(isDisabled(tripsQueryOverlay, getTrip(TestDataFetcher::trip1SNUuid)));
  EXPECT_TRUE(isDisabled(tripsQueryOverlay, getTrip(TestDataFetcher::trip2SNUuid)));
  EXPECT_TRUE(isDisabled(tripsQueryOverlay, getTrip(TestDataFetcher::trip1ExtraUuid)));
  EXPECT_FALSE(isDisabled(tripsQueryOverlay, getTrip(TestDataFetcher::trip1EWUuid)));
  EXPECT_FALSE(isDisabled(tripsQueryOverlay, getTrip(TestDataFetcher::trip2EWUuid)));
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

  std::vector<TrRouting::TripQueryData> snThenExtra = makeOverlay();
  snFilter.runFilter(snThenExtra, *connectionSet);
  extraFilter.runFilter(snThenExtra, *connectionSet);

  std::vector<TrRouting::TripQueryData> extraThenSn = makeOverlay();
  extraFilter.runFilter(extraThenSn, *connectionSet);
  snFilter.runFilter(extraThenSn, *connectionSet);

  EXPECT_EQ(countDisabled(snThenExtra), countDisabled(extraThenSn));
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

  std::vector<TrRouting::TripQueryData> tripsQueryOverlay = makeOverlay();

  std::vector<std::reference_wrapper<const TrRouting::Line>> firstExcludeLines;
  firstExcludeLines.push_back(getLine(TestDataFetcher::lineSNUuid));
  firstExcludeLines.push_back(getLine(TestDataFetcher::lineEWUuid));
  TrRouting::AlternativeLineFilter firstFilter(firstExcludeLines);
  firstFilter.runFilter(tripsQueryOverlay, *connectionSet);
  ASSERT_EQ(4u, countDisabled(tripsQueryOverlay));

  // Line SN is already disabled, line Extra is not
  std::vector<std::reference_wrapper<const TrRouting::Line>> secondExcludeLines;
  secondExcludeLines.push_back(getLine(TestDataFetcher::lineSNUuid));
  secondExcludeLines.push_back(getLine(TestDataFetcher::lineExtraUuid));
  TrRouting::AlternativeLineFilter secondFilter(secondExcludeLines);
  secondFilter.runFilter(tripsQueryOverlay, *connectionSet);

  EXPECT_EQ(5u, countDisabled(tripsQueryOverlay));
}

// The filter must never remove entries it did not add, so that results set by
// the caller or by a previous filter survive
TEST_F(AlternativeFilterFixtureTests, ExistingResultsArePreserved)
{
  std::shared_ptr<TrRouting::ConnectionSet> connectionSet = getFullConnectionSet();

  std::vector<TrRouting::TripQueryData> tripsQueryOverlay = makeOverlay();
  tripsQueryOverlay.at(getTrip(TestDataFetcher::trip1EWUuid).uid).disabled = true;

  std::vector<std::reference_wrapper<const TrRouting::Line>> excludeLines;
  excludeLines.push_back(getLine(TestDataFetcher::lineSNUuid));
  TrRouting::AlternativeLineFilter filter(excludeLines);
  filter.runFilter(tripsQueryOverlay, *connectionSet);

  EXPECT_EQ(3u, countDisabled(tripsQueryOverlay));
  EXPECT_TRUE(isDisabled(tripsQueryOverlay, getTrip(TestDataFetcher::trip1EWUuid)));
  EXPECT_TRUE(isDisabled(tripsQueryOverlay, getTrip(TestDataFetcher::trip1SNUuid)));
  EXPECT_TRUE(isDisabled(tripsQueryOverlay, getTrip(TestDataFetcher::trip2SNUuid)));
}

// An empty exclusion list must be a no-op, in particular it must not clear
TEST_F(AlternativeFilterFixtureTests, EmptyExcludeListPreservesExistingResults)
{
  std::shared_ptr<TrRouting::ConnectionSet> connectionSet = getFullConnectionSet();

  std::vector<TrRouting::TripQueryData> tripsQueryOverlay = makeOverlay();
  tripsQueryOverlay.at(getTrip(TestDataFetcher::trip1SNUuid).uid).disabled = true;

  std::vector<std::reference_wrapper<const TrRouting::Line>> excludeLines;
  TrRouting::AlternativeLineFilter filter(excludeLines);
  filter.runFilter(tripsQueryOverlay, *connectionSet);

  EXPECT_EQ(1u, countDisabled(tripsQueryOverlay));
  EXPECT_TRUE(isDisabled(tripsQueryOverlay, getTrip(TestDataFetcher::trip1SNUuid)));
}

// Running the same filter twice must be idempotent: accumulating means the
// second run re-adds the same uids rather than growing the result
TEST_F(AlternativeFilterFixtureTests, RunFilterIsIdempotent)
{
  std::shared_ptr<TrRouting::ConnectionSet> connectionSet = getFullConnectionSet();

  std::vector<std::reference_wrapper<const TrRouting::Line>> excludeLines;
  excludeLines.push_back(getLine(TestDataFetcher::lineEWUuid));
  TrRouting::AlternativeLineFilter filter(excludeLines);

  std::vector<TrRouting::TripQueryData> tripsQueryOverlay = makeOverlay();
  filter.runFilter(tripsQueryOverlay, *connectionSet);
  ASSERT_EQ(2u, countDisabled(tripsQueryOverlay));

  filter.runFilter(tripsQueryOverlay, *connectionSet);

  EXPECT_EQ(2u, countDisabled(tripsQueryOverlay));
  EXPECT_TRUE(isDisabled(tripsQueryOverlay, getTrip(TestDataFetcher::trip1EWUuid)));
  EXPECT_TRUE(isDisabled(tripsQueryOverlay, getTrip(TestDataFetcher::trip2EWUuid)));
}

// The disabled flag now shares TripQueryData with the rest of the per query trip
// scratch data, so the filter must not disturb the other fields
TEST_F(AlternativeFilterFixtureTests, RunFilterLeavesOtherOverlayFieldsUntouched)
{
  std::shared_ptr<TrRouting::ConnectionSet> connectionSet = getFullConnectionSet();

  std::vector<TrRouting::TripQueryData> tripsQueryOverlay = makeOverlay();
  for (auto & tripIte : connectionSet->getTrips())
  {
    tripsQueryOverlay.at(tripIte.get().uid).usable = true;
  }

  std::vector<std::reference_wrapper<const TrRouting::Line>> excludeLines;
  excludeLines.push_back(getLine(TestDataFetcher::lineSNUuid));
  TrRouting::AlternativeLineFilter filter(excludeLines);
  filter.runFilter(tripsQueryOverlay, *connectionSet);

  ASSERT_EQ(2u, countDisabled(tripsQueryOverlay));
  for (auto & tripIte : connectionSet->getTrips())
  {
    const TrRouting::TripQueryData & data = tripsQueryOverlay.at(tripIte.get().uid);
    EXPECT_TRUE(data.usable);
    EXPECT_FALSE(data.enterConnection.has_value());
    EXPECT_FALSE(data.exitConnection.has_value());
    EXPECT_EQ(TrRouting::MAX_INT, data.enterConnectionTransferTravelTime);
    EXPECT_EQ(TrRouting::MAX_INT, data.exitConnectionTransferTravelTime);
  }
}

// Trips outside the connection set must keep their default state, since the
// overlay is sized for every trip but the filter only walks the scenario trips
TEST_F(AlternativeFilterFixtureTests, TripsOutsideConnectionSetAreNotTouched)
{
  std::shared_ptr<TrRouting::ConnectionSet> connectionSet =
    transitData.getConnectionsForScenario(
      transitData.getScenarios().at(TestDataFetcher::scenario2Uuid));

  std::vector<TrRouting::TripQueryData> tripsQueryOverlay = makeOverlay();

  std::vector<std::reference_wrapper<const TrRouting::Line>> excludeLines;
  excludeLines.push_back(getLine(TestDataFetcher::lineSNUuid));
  TrRouting::AlternativeLineFilter filter(excludeLines);
  filter.runFilter(tripsQueryOverlay, *connectionSet);

  // Line EW is excluded from scenario 2, so its trips must stay enabled
  EXPECT_FALSE(isDisabled(tripsQueryOverlay, getTrip(TestDataFetcher::trip1EWUuid)));
  EXPECT_FALSE(isDisabled(tripsQueryOverlay, getTrip(TestDataFetcher::trip2EWUuid)));
}
