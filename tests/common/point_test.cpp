#include <limits>
#include <tuple>

#include "gtest/gtest.h"
#include "point.hpp"

// The parameters are the latitude, the longitude and whether the point is valid
class PointIsValidTests : public ::testing::TestWithParam<std::tuple<double, double, bool>>
{
};

TEST_P(PointIsValidTests, IsValid)
{
    auto [latitude, longitude, expected] = GetParam();
    EXPECT_EQ(expected, TrRouting::Point(latitude, longitude).isValid());
}

INSTANTIATE_TEST_SUITE_P(Coordinates, PointIsValidTests, ::testing::Values(
    std::make_tuple(45.5269, -73.58912, true),
    std::make_tuple(90.0, 180.0, true),
    std::make_tuple(-90.0, -180.0, true),
    std::make_tuple(90.000001, 0.0, false),
    std::make_tuple(-90.000001, 0.0, false),
    std::make_tuple(0.0, 180.000001, false),
    std::make_tuple(0.0, -180.000001, false),
    std::make_tuple(std::numeric_limits<double>::quiet_NaN(), 0.0, false),
    std::make_tuple(0.0, std::numeric_limits<double>::infinity(), false)
));
