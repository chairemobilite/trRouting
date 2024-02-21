#include <chrono>
#include <cassert>
#include "calculation_time.hpp"

namespace TrRouting
{

  CalculationTime::CalculationTime() :
    startEpoch(0),
    startStepEpoch(0),
    endEpoch(0),
    endStepEpoch(0),
    calculationEpoch() {
    //TODO Should we call start() on construction ??
  }

  long long CalculationTime::getEpoch()
  {
    calculationEpoch = std::chrono::duration_cast< std::chrono::microseconds >(
      std::chrono::system_clock::now().time_since_epoch()
    );
    return calculationEpoch.count();
  }
  
  void CalculationTime::start()
  {
    startEpoch = getEpoch();
  }
  
  void CalculationTime::stop()
  {
    endEpoch = getEpoch();
  }
  
  long long CalculationTime::getDurationMicroseconds()
  {
    assert(startEpoch && endEpoch);
    assert(startEpoch >= 0);
    assert(endEpoch >= 0);
    assert(endEpoch >= startEpoch);
    return endEpoch - startEpoch;
  }
  
  long long CalculationTime::getDurationMicrosecondsNoStop()
  {
    long long actualEpoch {getEpoch()};
    assert(startEpoch && actualEpoch);
    assert(startEpoch >= 0);
    assert(actualEpoch >= 0);
    assert(actualEpoch >= startEpoch);
    return actualEpoch - startEpoch;
  }
  
  void CalculationTime::startStep()
  {
    startStepEpoch = getEpoch();
  }
  
  void CalculationTime::stopStep()
  {
    endStepEpoch = getEpoch();
  }
  
  long long CalculationTime::getStepDurationMicroseconds()
  {
    assert(startStepEpoch && endStepEpoch);
    assert(startStepEpoch >= 0);
    assert(endStepEpoch >= 0);
    assert(endStepEpoch >= startStepEpoch);
    return endStepEpoch - startStepEpoch;
  }
  
  //CalculationTime CalculationTime::algorithmCalculationTime;
  
}
