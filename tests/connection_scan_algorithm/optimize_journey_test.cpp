#include <deque>

#include "gtest/gtest.h"
#include "calculator.hpp"
#include "csa_test_base.hpp"
#include "journey_step.hpp"
#include "connection.hpp"
#include "node.hpp"
#include "trip.hpp"

class OptimizeJourneyTests : public BaseCsaFixtureTests
{
protected:
    // Create an in-vehicle step on a trip, from the departure of its connection at enterIdx to the arrival of its connection at exitIdx
    TrRouting::JourneyStep makeStep(const boost::uuids::uuid &tripUuid, size_t enterIdx, size_t exitIdx)
    {
        const TrRouting::Trip &trip = transitData.getTrips().at(tripUuid);
        return TrRouting::JourneyStep(trip.forwardConnections[enterIdx], trip.forwardConnections[exitIdx], trip, 0, true, 0);
    }

    // Create an access or egress walking step, which has no trip, as added around the in-vehicle steps of a calculated journey
    TrRouting::JourneyStep makeWalkingStep()
    {
        return TrRouting::JourneyStep(std::nullopt, std::nullopt, std::nullopt, 0, false, 0);
    }
};

// The parameter is the number of steps between the step that can be extended
// and the step it makes superfluous
class OptimizeJourneyCutSuperfluousLineTests : public OptimizeJourneyTests, public ::testing::WithParamInterface<int>
{
};

// The South2 to North2 step and the East1 to Extra1 step do not share any
// node, so the journey should be left unchanged.
TEST_F(OptimizeJourneyTests, NothingToOptimize)
{
    std::deque<TrRouting::JourneyStep> journey;
    journey.push_back(makeWalkingStep());
    journey.push_back(makeStep(TestDataFetcher::trip1SNUuid, 0, 3));
    journey.push_back(makeStep(TestDataFetcher::trip1ExtraUuid, 0, 0));
    journey.push_back(makeWalkingStep());

    TrRouting::Calculator calculator(transitData, geoFilter);
    EXPECT_EQ(std::vector<int>(), calculator.optimizeJourney(journey));

    ASSERT_EQ(4u, journey.size());
    EXPECT_EQ(TestDataFetcher::trip1SNUuid, journey[1].getFinalTrip().value().get().uuid);
    EXPECT_EQ(TestDataFetcher::nodeNorth2Uuid, journey[1].getFinalExitConnection().value().get().getArrivalNode().uuid);
    EXPECT_EQ(TestDataFetcher::trip1ExtraUuid, journey[2].getFinalTrip().value().get().uuid);
}

// The first in-vehicle step, South2 to North2, passes by MidPoint, where the
// last in-vehicle step, East2 to MidPoint, alights. The steps in between, East1
// to Extra1, do not share any node with them. The first in-vehicle step should
// be cut at MidPoint and all the following in-vehicle steps removed, while the
// access and egress walking steps are kept.
TEST_P(OptimizeJourneyCutSuperfluousLineTests, CutSuperfluousLine)
{
    std::deque<TrRouting::JourneyStep> journey;
    journey.push_back(makeWalkingStep());
    journey.push_back(makeStep(TestDataFetcher::trip1SNUuid, 0, 3));
    for (int i = 0; i < GetParam(); i++) {
        journey.push_back(makeStep(TestDataFetcher::trip1ExtraUuid, 0, 0));
    }
    journey.push_back(makeStep(TestDataFetcher::trip1EWUuid, 0, 1));
    journey.push_back(makeWalkingStep());

    TrRouting::Calculator calculator(transitData, geoFilter);
    EXPECT_EQ(std::vector<int>({1}), calculator.optimizeJourney(journey));

    ASSERT_EQ(3u, journey.size());
    EXPECT_FALSE(journey[0].getFinalTrip().has_value());
    EXPECT_EQ(TestDataFetcher::trip1SNUuid, journey[1].getFinalTrip().value().get().uuid);
    EXPECT_EQ(TestDataFetcher::nodeMidNodeUuid, journey[1].getFinalExitConnection().value().get().getArrivalNode().uuid);
    EXPECT_FALSE(journey[2].getFinalTrip().has_value());
}

INSTANTIATE_TEST_SUITE_P(StepsInBetween, OptimizeJourneyCutSuperfluousLineTests, ::testing::Values(0, 1, 2));
