#include <chrono>
#include <cassert>
#include "calculation_time.hpp"

namespace TrRouting
{

  long long CalculationTime::getEpoch()
  {
    calculationEpoch = std::chrono::duration_cast< std::chrono::milliseconds >(
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
  
  long long CalculationTime::getDurationMilliseconds()
  {
    assert(startEpoch && endEpoch && startEpoch >= 0 && endEpoch >= 0 && endEpoch >= startEpoch);
    return endEpoch - startEpoch;
  }
  
  void CalculationTime::startStep()
  {
    startStepEpoch = getEpoch();
  }
  
  void CalculationTime::stopStep()
  {
    endStepEpoch = getEpoch();
  }
  
  long long CalculationTime::getStepDurationMilliseconds()
  {
    assert(startStepEpoch && endStepEpoch && startStepEpoch >= 0 && endStepEpoch >= 0 && endStepEpoch >= startStepEpoch);
    return endStepEpoch - startStepEpoch;
  }
  
  CalculationTime CalculationTime::algorithmCalculationTime;
  
}
