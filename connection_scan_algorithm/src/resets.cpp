#include "calculator.hpp"

namespace TrRouting
{
  
  void Calculator::reset()
  {
    
    std::fill(stopsTentativeTime.begin(), stopsTentativeTime.end(), MAX_INT);
    std::fill(stopsAccessTravelTime.begin(), stopsAccessTravelTime.end(), -1);
    std::fill(stopsEgressTravelTime.begin(), stopsEgressTravelTime.end(), -1);
    std::fill(tripsEnterConnection.begin(), tripsEnterConnection.end(), -1);
    std::fill(tripsEnterConnectionTransferTravelTime.begin(), tripsEnterConnectionTransferTravelTime.end(), MAX_INT);
    std::fill(tripsEnabled.begin(), tripsEnabled.end(), 1);
    std::fill(journeys.begin(), journeys.end(), std::make_tuple(-1,-1,-1,-1,-1,-1));
    
    departureTimeSeconds = params.departureTimeHour * 3600 + params.departureTimeMinutes * 60;
    
    // disable trips according to parameters:
    //bool hasOnlyServices     = !params.onlyServiceIds.empty();
    bool hasOnlyRoutes       = !params.onlyRouteIds.empty();
    bool hasOnlyRouteTypes   = !params.onlyRouteTypeIds.empty();
    bool hasOnlyAgencies     = !params.onlyAgencyIds.empty();
    bool hasExceptServices   = !params.exceptServiceIds.empty();
    bool hasExceptRoutes     = !params.exceptRouteIds.empty();
    bool hasExceptRouteTypes = !params.exceptRouteTypeIds.empty();
    bool hasExceptAgencies   = !params.exceptAgencyIds.empty();
    
    int i {0};
    for (auto & trip : trips)
    {
      if (params.onlyServiceIds.size() > 0)
      {
        if (std::find(params.onlyServiceIds.begin(), params.onlyServiceIds.end(), trip.serviceId) == params.onlyServiceIds.end())
        {
          tripsEnabled[i] = -1;
        }
      }
      
      if (hasOnlyRoutes)
      {
        if (tripsEnabled[i] == 1 && params.onlyRouteIds.find(trip.routeId) == params.onlyRouteIds.end())
        {
          tripsEnabled[i] = -1;
        }
      }
      
      if (hasOnlyRouteTypes)
      {
        if (tripsEnabled[i] == 1 && params.onlyRouteTypeIds.find(trip.routeTypeId) == params.onlyRouteTypeIds.end())
        {
          tripsEnabled[i] = -1;
        }
      }
      
      if (hasOnlyAgencies)
      {
        if (tripsEnabled[i] == 1 && params.onlyAgencyIds.find(trip.agencyId) == params.onlyAgencyIds.end())
        {
          tripsEnabled[i] = -1;
        }
      }
      
      if (hasExceptServices)
      {
        if (tripsEnabled[i] == 1 && params.exceptServiceIds.find(trip.serviceId) != params.exceptServiceIds.end())
        {
          tripsEnabled[i] = -1;
        }
      }
      
      if (hasExceptRoutes)
      {
        if (tripsEnabled[i] == 1 && params.exceptRouteIds.find(trip.routeId) != params.exceptRouteIds.end())
        {
          tripsEnabled[i] = -1;
        }
      }
      
      if (hasExceptRouteTypes)
      {
        if (tripsEnabled[i] == 1 && params.exceptRouteTypeIds.find(trip.routeTypeId) != params.exceptRouteTypeIds.end())
        {
          tripsEnabled[i] = -1;
        }
      }
      
      if (hasExceptAgencies)
      {
        if (tripsEnabled[i] == 1 && params.exceptAgencyIds.find(trip.agencyId) != params.exceptAgencyIds.end())
        {
          tripsEnabled[i] = -1;
        }
      }
      
      i++;
    }
    
  }

}
