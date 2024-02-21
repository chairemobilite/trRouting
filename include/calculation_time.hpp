#ifndef CALCULATION_TIME
#define CALCULATION_TIME

#include <chrono>


namespace TrRouting
{
  
  class CalculationTime {
    
    public:
      CalculationTime();
      //static CalculationTime algorithmCalculationTime;
      long long getDurationMicroseconds();
      long long getDurationMicrosecondsNoStop();
      long long getEpoch();
      void startStep();
      void stopStep();
      long long getStepDurationMicroseconds();
      void start();
      void stop();
      
    private:
      
      long long startEpoch;
      long long startStepEpoch;
      long long endEpoch;
      long long endStepEpoch;
      std::chrono::microseconds calculationEpoch;
    
  };
  
}


#endif // CALCULATION_TIME
