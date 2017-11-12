#include "connection_scan_algorithm.hpp"

namespace TrRouting
{
  
  void ConnectionScanAlgorithm::prepare()
  {

    ConnectionScanAlgorithm::ConnectionScanAlgorithm(Parameters& theParams) : params(theParams)
    {
      algorithmCalculationTime = CalculationTime();
      prepare();
    }
    
    ConnectionScanAlgorithm::ConnectionScanAlgorithm()
    {
      
    }
    
  }

}