#ifndef _CSA_TEST_H
#define _CSA_TEST_H

#include "gtest/gtest.h"
#include "calculator.hpp"
#include "parameters.hpp"

inline int getTimeInSeconds(int hour, int minutes = 0, int seconds = 0) { return hour * 3600 + minutes * 60 + seconds; }
const int MIN_WAITING_TIME{180};

class BaseCsaFixtureTests : public ::testing::Test
{

protected:
    inline static const boost::uuids::string_generator uuidGenerator;
    // String uuids of various objects
    inline static const boost::uuids::uuid nodeSouth2Uuid = uuidGenerator("5feed91e-f98f-46d6-972d-5f008b95c7b8");
    inline static const boost::uuids::uuid nodeSouth1Uuid = uuidGenerator("35bad963-c585-42a1-80d5-5422d6a36f46");
    inline static const boost::uuids::uuid nodeMidNodeUuid = uuidGenerator("9049d802-0428-46c2-9613-75862135edb0");
    inline static const boost::uuids::uuid nodeNorth1Uuid = uuidGenerator("ef75bf69-7eb4-4511-8cbc-104359d00b6f");
    inline static const boost::uuids::uuid nodeNorth2Uuid = uuidGenerator("abf94539-008b-4f15-b34c-f3b0f6b6d770");
    inline static const boost::uuids::uuid nodeEast2Uuid = uuidGenerator("384988e7-83b9-4074-bc41-44e754419d62");
    inline static const boost::uuids::uuid nodeEast1Uuid = uuidGenerator("d67940e7-05a5-44c9-bfa1-710ac031bfb9");
    inline static const boost::uuids::uuid nodeWest1Uuid = uuidGenerator("185abf2e-484f-426b-b4b2-eaab15d82bd1");
    inline static const boost::uuids::uuid nodeWest2Uuid = uuidGenerator("44def3b3-bfa0-4c0c-8304-8b0059e3361f");
    inline static const boost::uuids::uuid nodeExtra1Uuid = uuidGenerator("88774418-34cc-4761-8195-bc6e3b48b2e0");

    inline static const boost::uuids::uuid agencyUuid = uuidGenerator("aaaaaaaa-bbbb-cccc-dddd-eeeeeeffffff");

    // One line from South to North, one from East to West
    inline static const boost::uuids::uuid lineSNUuid = uuidGenerator("7b3ef82b-8630-45f1-aba9-a79687150d02");
    inline static const boost::uuids::uuid lineEWUuid = uuidGenerator("9d707cab-1ba3-4311-a6ed-64f893281097");
    inline static const boost::uuids::uuid lineExtraUuid = uuidGenerator("dcf8991c-ccbb-4903-9ce5-fde5f4d449a1");

    inline static const boost::uuids::uuid serviceUuid = uuidGenerator("11111111-2222-3333-4444-555555666666");

    inline static const boost::uuids::uuid scenarioUuid = uuidGenerator("11111111-2222-2345-789a-555555666666");

    inline static const boost::uuids::uuid pathSNUuid = uuidGenerator("2f31893e-b2fb-4ebe-bf1b-2ee77fe1f96b");
    inline static const boost::uuids::uuid pathEWUuid = uuidGenerator("50f8e656-99b5-4ddd-b41f-b4804fe599f3");
    inline static const boost::uuids::uuid pathExtraUuid = uuidGenerator("2f8fa7f9-e27c-4e98-9491-737147aa5a3e");

    inline static const boost::uuids::uuid trip1SNUuid = uuidGenerator("89f1ee54-1284-4861-939a-3f3241a3e461");
    inline static const boost::uuids::uuid trip2SNUuid = uuidGenerator("71a1f3eb-e98c-49bc-b578-7d2e490e1f2f");
    inline static const boost::uuids::uuid trip1EWUuid = uuidGenerator("af0a429f-84f3-4a83-9883-f6387d445cd8");
    inline static const boost::uuids::uuid trip2EWUuid = uuidGenerator("ea8c90ba-7deb-46af-87c6-8af211e8fab7");
    inline static const boost::uuids::uuid trip1ExtraUuid = uuidGenerator("4aff9220-e72e-4b41-9cbf-0d86565d5128");

    // Parameter object is required to build the calculator.
    // TODO As code evolves, it won't be necessary anymore
    TrRouting::Parameters params;
    // Calculator is the entry point to run the algorithm, it is part of the test object since (for now) there is nothing specific for its initialization.
    TrRouting::Calculator calculator;

    // Setup functions. The base class will define a complete default data set,
    // but specific test cases can override these methods and add their own data
    void setUpModes();
    void setUpNodes();
    void setUpAgencies();
    void setUpLines();
    void setUpServices();
    void setUpScenarios();
    void setUpPaths();
    void setUpSchedules(std::vector<std::shared_ptr<TrRouting::ConnectionTuple>> &connections);

public:
    BaseCsaFixtureTests() : params(TrRouting::Parameters()), calculator(TrRouting::Calculator(params)) {}
    void SetUp();
    // Assert the result returned an exception and that the reason matches the expected reason
    void assertNoRouting(const TrRouting::NoRoutingFoundException& exception, TrRouting::NoRoutingFoundException::NoRoutingReason expectedReason);
    // Asserts the successful result fields, given some easy to provide expected test data
    void assertSuccessResults(TrRouting::RoutingResult& result,
        int origDepartureTime,
        int expTransitDepartureTime,
        int expInVehicleTravelTime,
        int expAccessTime = 0,
        int expEgressTime = 0,
        int expNbTransfers = 0,
        int minWaitingTime = MIN_WAITING_TIME,
        int expTotalWaitingTime = MIN_WAITING_TIME,
        int expTransferWaitingTime = 0,
        int expTransferTravelTime = 0);

};

#endif // _CACHE_FETCHER_TEST_H