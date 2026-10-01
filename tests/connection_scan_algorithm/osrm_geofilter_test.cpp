#include <string>
#include <tuple>

#include "gtest/gtest.h"
#include "osrmgeofilter.hpp"
#include "point.hpp"

// Gives the tests access to the protected formatting of the coordinates
class TestableOsrmGeoFilter : public TrRouting::OsrmGeoFilter
{
public:
    using TrRouting::OsrmGeoFilter::formatOsrmCoordinates;
};

// The parameters are the latitude, the longitude and the expected coordinates sent to OSRM
class OsrmCoordinatesFormatTests : public ::testing::TestWithParam<std::tuple<double, double, std::string>>
{
};

TEST_P(OsrmCoordinatesFormatTests, FormatWithSixDecimals)
{
    auto [latitude, longitude, expected] = GetParam();
    EXPECT_EQ(expected, TestableOsrmGeoFilter::formatOsrmCoordinates(TrRouting::Point(latitude, longitude)));
}

INSTANTIATE_TEST_SUITE_P(Coordinates, OsrmCoordinatesFormatTests, ::testing::Values(
    std::make_tuple(45.5269, -73.58912, "-73.589120,45.526900"),
    std::make_tuple(45.98765432, -73.12345678, "-73.123457,45.987654"),
    std::make_tuple(0.0, 0.0, "0.000000,0.000000"),
    std::make_tuple(-33.8688, 151.2093, "151.209300,-33.868800")
));
