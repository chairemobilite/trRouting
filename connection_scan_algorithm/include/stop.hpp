#ifndef TR_STOP
#define TR_STOP

#include <vector>
#include <boost/serialization/access.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "point.hpp"
#include "simplified_journey_step.hpp"

namespace TrRouting
{
  
  struct Stop {
  
  public:
   
    long long id;
    std::string code;
    std::string name;
    long long stationId;
    Point point;
    int arrivalTimeMinuteOfDay;
    std::vector<std::shared_ptr<SimplifiedJourneyStep> > journeySteps;
    int numBoardings;
    unsigned long long canUnboardToDestination; // ConnectionScanAlgorithm::calculationId will be assigned if the stop is unboardable to reach destination
  
  private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive&ar, const unsigned int version)
    {
        ar & id;
        ar & code;
        ar & name;
        ar & stationId;
        ar & point;
        ar & arrivalTimeMinuteOfDay;
    }
  };

}

#endif // TR_STOP
