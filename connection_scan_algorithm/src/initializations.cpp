#include "calculator.hpp"

namespace TrRouting
{
  
  Calculator::Calculator(Parameters& theParams) : params(theParams)
  {
    
    if (params.osrmUseLib == true)
    {
      osrm::EngineConfig osrmConfig;
      osrmConfig.storage_config    = {params.osrmFilePath};
      osrmConfig.use_shared_memory = false;
      osrmConfig.algorithm         = osrm::EngineConfig::Algorithm::CH;
      osrm::OSRM osrmRouter{osrmConfig};
      params.osrmRouter            = std::move(osrmRouter);
      
    }

    algorithmCalculationTime = CalculationTime();
    std::cout << "preparing calculator..." << std::endl; 
    prepare();
  }
  
  //Calculator::Calculator()
  //{
  //  
  //}

}
