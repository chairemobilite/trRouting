#include "calculator.hpp"

namespace TrRouting
{
  
  Calculator::Calculator(Parameters& theParams) : params(theParams)
  {
    algorithmCalculationTime = CalculationTime();
    std::cout << "preparing calculator..." << std::endl; 
    prepare();
  }
  
  Calculator::Calculator()
  {
    
  }

}
