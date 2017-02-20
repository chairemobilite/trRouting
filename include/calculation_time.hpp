#ifndef CALCULATION_TIME
#define CALCULATION_TIME

#include <chrono>


namespace TrRouting
{
  
  class CalculationTime {
    
    public:
      
      //static CalculationTime algorithmCalculationTime;
      long long getDurationMilliseconds();
      long long getEpoch();
      void startStep();
      void stopStep();
      long long getStepDurationMilliseconds();
      void start();
      void stop();
      
    private:
      
      long long startEpoch;
      long long startStepEpoch;
      long long endEpoch;
      long long endStepEpoch;
      std::chrono::milliseconds calculationEpoch;
    
  };
  
}


#endif // CALCULATION_TIME
