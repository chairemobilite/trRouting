#include "gtest/gtest.h" // we will add the path to C preprocessor later
#include "parameters.hpp"
#include "alternative_routing_test.hpp"

using namespace TrRouting;

class AlternativeRoutingTests : public ConstantAlternativeRoutingTests {

    protected:
    int numberOfRequests = 0;

    int setup() {
        // TODO get OSRM DATA, set birdDistanceAccessibilityEnabled to true_type
        // Make sure gtfs cache zip is present
        // Make sure cvs file with TRrouting requests is present
        // set number of requests
        // start TRrouting
    }

    int evaluateRequests() {
        // Disable open mp with conditional compilation
        // TODO: send all requests from cvs file to TRrouting
        // wait for responses
    }

    int evaluateRequestsWithParallelization() {
        // TODO: send all requests from cvs file to TRrouting
        // wait for responses
    }
};


TEST_F(AlternativeRoutingTests, TestGetFilePath){

    setup();

    // TODO: start timer 
    evaluateRequests();
    // end timer 
    // get average duration by dividing with the numberOfRequests
    // cout the result

    // TODO: start timer 
    evaluateRequestsWithParallelization();
    // end timer 
    // get average duration by dividing with the numberOfRequests
    // cout the result

    // Calculate the ratio between both duration
    // Calculate the time saved or lost time
    // Cout comparison results

}

