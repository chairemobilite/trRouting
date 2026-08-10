#include <deque>
#include <ostream>
#include <string>
#include <vector>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include "gtest/gtest.h"
#include "agency.hpp"
#include "line.hpp"
#include "line_combinations.hpp"
#include "mode.hpp"

// Tests for the combination generation used by the alternatives routing. The
// helpers only ever look at line identity, so plain Line objects are enough
// here and no transit data needs to be loaded.

namespace TrRouting
{
  // Using by gtest. So a failed EXPECT prints line shortnames
  // instead of a raw byte dump
  void PrintTo(const std::reference_wrapper<const Line> & line, std::ostream * os)
  {
    *os << line.get().shortname;
  }
}

class LineCombinationsFixtureTests : public ::testing::Test
{
protected:
  LineCombinationsFixtureTests() :
    agency({boost::uuids::random_generator()(), "AGE", "Test agency", boost::uuids::nil_uuid()}),
    mode("bus", "Bus", 3, 3),
    lineA(makeLine("A")),
    lineB(makeLine("B")),
    lineC(makeLine("C")) {}

  TrRouting::Agency agency;
  TrRouting::Mode mode;
  // std::deque keeps references stable as lines are added, which std::vector
  // would not
  std::deque<TrRouting::Line> lines;
  // Created in this order, so their uids are increasing and a sorted
  // combination lists them alphabetically
  const TrRouting::Line & lineA;
  const TrRouting::Line & lineB;
  const TrRouting::Line & lineC;

  const TrRouting::Line & makeLine(const std::string & shortname)
  {
    lines.emplace_back(boost::uuids::random_generator()(), agency, mode, shortname, shortname, 0);
    return lines.back();
  }

  static bool contains(const std::vector<TrRouting::LineVector> & combinations,
                       const TrRouting::LineVector & expected)
  {
    return std::find(combinations.begin(), combinations.end(), expected) != combinations.end();
  }
};

TEST_F(LineCombinationsFixtureTests, ContainsAllLinesWithEmptySubset)
{
  EXPECT_TRUE(TrRouting::containsAllLines({lineA}, {}));
  EXPECT_TRUE(TrRouting::containsAllLines({}, {}));
}

TEST_F(LineCombinationsFixtureTests, ContainsAllLinesWithSubset)
{
  EXPECT_TRUE(TrRouting::containsAllLines({lineA, lineB}, {lineA}));
  EXPECT_TRUE(TrRouting::containsAllLines({lineA, lineB}, {lineA, lineB}));
}

TEST_F(LineCombinationsFixtureTests, ContainsAllLinesWithMissingLine)
{
  EXPECT_FALSE(TrRouting::containsAllLines({lineA, lineB}, {lineC}));
  // A smaller container cannot contain a larger subset
  EXPECT_FALSE(TrRouting::containsAllLines({lineA}, {lineA, lineB}));
}

TEST_F(LineCombinationsFixtureTests, NothingMatchesEmptyCombinations)
{
  EXPECT_FALSE(TrRouting::isSupersetOfAnyCombinations({lineA}, {}));
}

TEST_F(LineCombinationsFixtureTests, ExactCombinationMatches)
{
  std::vector<TrRouting::LineVector> combinations {{lineA, lineB}};

  EXPECT_TRUE(TrRouting::isSupersetOfAnyCombinations({lineA, lineB}, combinations));
}

TEST_F(LineCombinationsFixtureTests, SupersetOfCombinationMatches)
{
  std::vector<TrRouting::LineVector> combinations {{lineA}};

  // Excluding A and B on top of a failing exclusion of A can only fail as well
  EXPECT_TRUE(TrRouting::isSupersetOfAnyCombinations({lineA, lineB}, combinations));
}

TEST_F(LineCombinationsFixtureTests, SubsetOfCombinationDoesNotMatch)
{
  std::vector<TrRouting::LineVector> combinations {{lineA, lineB}};

  // Excluding fewer lines may still find a route, so it must stay a candidate
  EXPECT_FALSE(TrRouting::isSupersetOfAnyCombinations({lineA}, combinations));
  EXPECT_FALSE(TrRouting::isSupersetOfAnyCombinations({lineC}, combinations));
}

TEST_F(LineCombinationsFixtureTests, GeneratesEverySubsetOfFoundLines)
{
  std::vector<TrRouting::LineVector> allCombinations;
  TrRouting::CombinationMap alreadyCalculatedCombinations;

  TrRouting::generateCombinations({lineA, lineB}, {}, {}, allCombinations, alreadyCalculatedCombinations);

  ASSERT_EQ(3u, allCombinations.size());
  EXPECT_TRUE(contains(allCombinations, {lineA}));
  EXPECT_TRUE(contains(allCombinations, {lineB}));
  EXPECT_TRUE(contains(allCombinations, {lineA, lineB}));
  EXPECT_EQ(3u, alreadyCalculatedCombinations.size());
}

TEST_F(LineCombinationsFixtureTests, GeneratesNothingFromEmptyFoundLines)
{
  std::vector<TrRouting::LineVector> allCombinations;
  TrRouting::CombinationMap alreadyCalculatedCombinations;

  TrRouting::generateCombinations({}, {}, {}, allCombinations, alreadyCalculatedCombinations);

  EXPECT_EQ(0u, allCombinations.size());
  EXPECT_EQ(0u, alreadyCalculatedCombinations.size());
}

TEST_F(LineCombinationsFixtureTests, AlreadyCalculatedCombinationsAreNotGeneratedTwice)
{
  std::vector<TrRouting::LineVector> allCombinations;
  TrRouting::CombinationMap alreadyCalculatedCombinations;

  TrRouting::generateCombinations({lineA, lineB}, {}, {}, allCombinations, alreadyCalculatedCombinations);
  ASSERT_EQ(3u, allCombinations.size());

  TrRouting::generateCombinations({lineA, lineB}, {}, {}, allCombinations, alreadyCalculatedCombinations);

  EXPECT_EQ(3u, allCombinations.size());
}

TEST_F(LineCombinationsFixtureTests, PrefixIsAddedToEveryGeneratedCombinationAndSorted)
{
  std::vector<TrRouting::LineVector> allCombinations;
  TrRouting::CombinationMap alreadyCalculatedCombinations;

  TrRouting::generateCombinations({lineA, lineB}, {lineC}, {}, allCombinations, alreadyCalculatedCombinations);

  ASSERT_EQ(3u, allCombinations.size());
  // The comparison is order sensitive, so it also asserts the sort by uid
  EXPECT_TRUE(contains(allCombinations, {lineA, lineC}));
  EXPECT_TRUE(contains(allCombinations, {lineB, lineC}));
  EXPECT_TRUE(contains(allCombinations, {lineA, lineB, lineC}));
}

TEST_F(LineCombinationsFixtureTests, CombinationsMatchingAFailedOneAreSkipped)
{
  std::vector<TrRouting::LineVector> failedCombinations {{lineA}};
  std::vector<TrRouting::LineVector> allCombinations;
  TrRouting::CombinationMap alreadyCalculatedCombinations;

  TrRouting::generateCombinations({lineA, lineB}, {}, failedCombinations, allCombinations,
                                  alreadyCalculatedCombinations);

  // {A} and {A, B} both contain the failed {A}
  ASSERT_EQ(1u, allCombinations.size());
  EXPECT_TRUE(contains(allCombinations, {lineB}));
  // The 3 subsets are all recorded, even the 2 that were rejected, so a later
  // generation does not have to check them against the failed list again
  EXPECT_EQ(3u, alreadyCalculatedCombinations.size());
}
