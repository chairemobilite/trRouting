#include <deque>

#include "gtest/gtest.h"
#include "calculator.hpp"
#include "csa_test_base.hpp"
#include "journey_step.hpp"
#include "connection.hpp"
#include "node.hpp"
#include "trip.hpp"

// The parameter is the number of steps between the step that can be extended
// and the step it makes superfluous
class OptimizeJourneyCutSuperfluousLineTests : public BaseCsaFixtureTests, public ::testing::WithParamInterface<int>
{
protected:
    // Create an in-vehicle step on a trip, from the departure of its connection at enterIdx to the arrival of its connection at exitIdx
    TrRouting::JourneyStep makeStep(const boost::uuids::uuid &tripUuid, size_t enterIdx, size_t exitIdx)
    {
        const TrRouting::Trip &trip = transitData.getTrips().at(tripUuid);
        return TrRouting::JourneyStep(trip.forwardConnections[enterIdx], trip.forwardConnections[exitIdx], trip, 0, true, 0);
    }
};

// The first step, South2 to North2, passes by MidPoint, where the last step,
// East2 to MidPoint, alights. The steps in between, East1 to Extra1, do not
// share any node with them. The first step should be cut at MidPoint and all
// the following steps removed.
TEST_P(OptimizeJourneyCutSuperfluousLineTests, CutSuperfluousLine)
{
    std::deque<TrRouting::JourneyStep> journey;
    journey.push_back(makeStep(TestDataFetcher::trip1SNUuid, 0, 3));
    for (int i = 0; i < GetParam(); i++) {
        journey.push_back(makeStep(TestDataFetcher::trip1ExtraUuid, 0, 0));
    }
    journey.push_back(makeStep(TestDataFetcher::trip1EWUuid, 0, 1));

    TrRouting::Calculator calculator(transitData, geoFilter);
    EXPECT_EQ(std::vector<int>({1}), calculator.optimizeJourney(journey));

    ASSERT_EQ(1u, journey.size());
    EXPECT_EQ(TestDataFetcher::trip1SNUuid, journey[0].getFinalTrip().value().get().uuid);
    EXPECT_EQ(TestDataFetcher::nodeMidNodeUuid, journey[0].getFinalExitConnection().value().get().getArrivalNode().uuid);
}

INSTANTIATE_TEST_SUITE_P(StepsInBetween, OptimizeJourneyCutSuperfluousLineTests, ::testing::Values(0, 1, 2));
