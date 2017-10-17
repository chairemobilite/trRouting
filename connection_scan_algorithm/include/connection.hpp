#ifndef TR_CONNECTION
#define TR_CONNECTION

#include <boost/serialization/access.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "simplified_journey_step.hpp"

namespace TrRouting
{
  
  struct Connection {
        
    unsigned long long id;
    unsigned long long serviceId;
    unsigned long long agencyId;
    unsigned long long routeId;
    unsigned long long routeTypeId;
    unsigned long long tripId;
    unsigned long long sequence;
    int departureFromOriginTimeMinuteOfDay;
    int arrivalAtDestinationTimeMinuteOfDay;
    unsigned long long pathStopSequenceStartId;
    unsigned long long pathStopSequenceEndId;
    unsigned long long stopStartId;
    unsigned long long stopEndId;
    bool canBoard;
    bool canUnboard;
    int reachable; // ConnectionScanAlgorithm::calculationId will be assigned for each reachable connections
    bool enabled; // connection is enabled in database
    bool calculationEnabled; // connection is enabled for a single calculation (after filtering route ids, route type ids, service ids, etc.)
    long long nextConnectionId;
    long long previousConnectionId;
    int numBoardings;
    int totalInVehicleTravelTimeMinutes;
    int totalNotInVehicleTravelTimeMinutes;
    std::vector<std::shared_ptr<SimplifiedJourneyStep> > journeySteps;
    int lastJourneyStepIndex;
    
  private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive&ar, const unsigned int version)
    {
        ar & id;
        ar & serviceId;
        ar & agencyId;
        ar & routeId;
        ar & routeTypeId;
        ar & tripId;
        ar & sequence;
        ar & departureFromOriginTimeMinuteOfDay;
        ar & arrivalAtDestinationTimeMinuteOfDay;
        ar & pathStopSequenceStartId;
        ar & pathStopSequenceEndId;
        ar & stopStartId;
        ar & stopEndId;
        ar & canBoard;
        ar & canUnboard;
        ar & nextConnectionId;
        ar & previousConnectionId;
        ar & enabled;

    }
  };

}

#endif // TR_CONNECTION
