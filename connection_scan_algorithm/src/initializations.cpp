#include "calculator.hpp"

namespace TrRouting
{
  
  Calculator::Calculator(Parameters& theParams) : params(theParams)
  {
    
    algorithmCalculationTime = CalculationTime();
    
    if (params.osrmWalkingUseLib == true)
    {
      osrm::EngineConfig osrmConfig;
      osrmConfig.storage_config    = {params.osrmWalkingFilePath};
      osrmConfig.use_shared_memory = false;
      osrmConfig.algorithm         = osrm::EngineConfig::Algorithm::CH;
      osrm::OSRM osrmWalkingRouter{osrmConfig};
      params.osrmWalkingRouter     = std::move(osrmWalkingRouter);
    }
    if (params.osrmCyclingUseLib == true)
    {
      osrm::EngineConfig osrmConfig;
      osrmConfig.storage_config    = {params.osrmCyclingFilePath};
      osrmConfig.use_shared_memory = false;
      osrmConfig.algorithm         = osrm::EngineConfig::Algorithm::CH;
      osrm::OSRM osrmCyclingRouter{osrmConfig};
      params.osrmCyclingRouter     = std::move(osrmCyclingRouter);
    }
    if (params.osrmDrivingUseLib == true)
    {
      osrm::EngineConfig osrmConfig;
      osrmConfig.storage_config    = {params.osrmDrivingFilePath};
      osrmConfig.use_shared_memory = false;
      osrmConfig.algorithm         = osrm::EngineConfig::Algorithm::CH;
      osrm::OSRM osrmDrivingRouter{osrmConfig};
      params.osrmDrivingRouter     = std::move(osrmDrivingRouter);
    }

  }
  

}
