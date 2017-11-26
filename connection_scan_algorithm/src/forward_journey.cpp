#include "calculator.hpp"
#include "json.hpp"

namespace TrRouting
{
    
  RoutingResult Calculator::forwardJourney(int bestArrivalTime, int bestEgressStopIndex, int bestEgressTravelTime)
  {
    RoutingResult    result;
    nlohmann::json   json;
    int              stopsCount           {1};
    int              i                    {0};
    int              reachableStopsCount  {0};
    bool             foundRoute           {false};
    
    std::vector<int> resultingStops;
    if (params.returnAllStopsResult)
    {
      stopsCount     = stops.size();
      json["stops"]  = nlohmann::json::array();
      resultingStops = std::vector<int>(stopsCount);
      std::iota (std::begin(resultingStops), std::end(resultingStops), 0); // generate sequencial indexes of each stops
    }
    else if (bestArrivalTime < MAX_INT) // route found
    {
      foundRoute = true;
      resultingStops.push_back(bestEgressStopIndex);
    }

    if (foundRoute || params.returnAllStopsResult)
    {

      std::deque<std::tuple<int,int,int,int,int,short>> journey;
      std::tuple<int,int,int,int,int,short>             resultingStopJourneyStep;
      std::tuple<int,int,int,int,int,short>             emptyJourneyStep {-1,-1,-1,-1,-1,-1};
      std::tuple<int,int,int,int,int,short,short,int>   journeyStepEnterConnection; // connection tuple: departureStopIndex, arrivalStopIndex, departureTimeSeconds, arrivalTimeSeconds, tripIndex, canBoard, canUnboard, sequenceinTrip
      std::tuple<int,int,int,int,int,short,short,int>   journeyStepExitConnection;
      std::vector<std::tuple<unsigned long long, unsigned long long, unsigned long long, int, int>> legs; // tuple: tripId, routeId, routePathId, boarding sequence, unboarding sequence
      nlohmann::json stepJson = {};
      nlohmann::json stopJson = {};

      Stop  journeyStepStopDeparture;
      Stop  journeyStepStopArrival;
      Trip  journeyStepTrip;
      Route journeyStepRoute;
      
      int totalInVehicleTime       { 0}; int transferArrivalTime {-1}; int firstDepartureTime     {-1};
      int totalWalkingTime         { 0}; int transferReadyTime   {-1}; int minimizedDepartureTime {-1};
      int totalWaitingTime         { 0}; int departureTime       {-1}; int numberOfTransfers      {-1};
      int totalTransferWalkingTime { 0}; int arrivalTime         {-1}; int boardingSequence       {-1};
      int totalTransferWaitingTime { 0}; int inVehicleTime       {-1}; int unboardingSequence     {-1};
      int journeyStepTravelTime    {-1}; int accessWalkingTime   {-1}; int bestAccessStopIndex    {-1};
      int transferTime             {-1}; int egressWalkingTime   {-1}; 
      int waitingTime              {-1}; int accessWaitingTime   {-1}; 
  
      for (auto & resultingStopIndex : resultingStops)
      {
        
        legs.clear();
        journey.clear();
        
        totalInVehicleTime       =  0; transferArrivalTime = -1; firstDepartureTime     = -1;
        totalWalkingTime         =  0; transferReadyTime   = -1; minimizedDepartureTime = -1;
        totalWaitingTime         =  0; departureTime       = -1; numberOfTransfers      = -1;
        totalTransferWalkingTime =  0; arrivalTime         = -1; boardingSequence       = -1;
        totalTransferWaitingTime =  0; inVehicleTime       = -1; unboardingSequence     = -1;
        journeyStepTravelTime    = -1; accessWalkingTime   = -1; bestAccessStopIndex    = -1;
        transferTime             = -1; egressWalkingTime   = -1; 
        waitingTime              = -1; accessWaitingTime   = -1; 
        
        //std::cerr << stops[resultingStopIndex].name << " : " << std::get<0>(forwardJourneys[resultingStopIndex]) << std::endl; 
        // recreate journey:
        resultingStopJourneyStep = forwardJourneys[resultingStopIndex];
        
        if (resultingStopJourneyStep == emptyJourneyStep) // ignore stops with no route
        {
          continue;
        }
        
        i = 0;
        while ((std::get<0>(resultingStopJourneyStep) != -1 && std::get<1>(resultingStopJourneyStep) != -1))
        {
          journey.push_front(resultingStopJourneyStep);
          bestAccessStopIndex = std::get<connectionIndexes::STOP_DEP>(forwardConnections[std::get<0>(resultingStopJourneyStep)]);
          //std::cerr << "sequence: " << std::get<connectionIndexes::SEQUENCE>(forwardConnections[std::get<0>(resultingStopJourneyStep)]) << " sequence2: " << std::get<connectionIndexes::SEQUENCE>(forwardConnections[std::get<1>(journeyStep)]) << " stop:" << stops[bestAccessStopIndex].name << " tenterc:" <<  std::get<0>(journeyStep) << " texitc:" <<  std::get<1>(journeyStep) << " jss:" <<  std::get<5>(journeyStep) << " jtt:" << std::get<4>(journeyStep) << " tectt:" << tripsEnterConnectionTransferTravelTime[std::get<3>(journeyStep)] << std::endl;
          //std::cerr << stops[bestAccessStopIndex].name << " > " << stops[std::get<connectionIndexes::STOP_ARR>(forwardConnections[std::get<1>(resultingStopJourneyStep)])].name << std::endl;
          resultingStopJourneyStep = forwardJourneys[bestAccessStopIndex];
          i++;
        }
        
        if (!params.returnAllStopsResult)
        {
          json["steps"] = nlohmann::json::array();
          journey.push_back(std::make_tuple(-1,-1,-1,-1,stopsEgressTravelTime[resultingStopIndex],-1));
        }
        journey.push_front(std::make_tuple(-1,-1,-1,-1,stopsAccessTravelTime[bestAccessStopIndex],-1));
        
        //std::string stepsJson = "  \"steps\":\n  [\n";
       
        i = 0;
        int journeyStepsCount = journey.size();
        for (auto & journeyStep : journey)
        {
          
          if (std::get<0>(journeyStep) != -1 && std::get<1>(journeyStep) != -1)
          {
            // journey tuple: final enter connection, final exit connection, final footpath
            journeyStepEnterConnection = forwardConnections[std::get<0>(journeyStep)];
            journeyStepExitConnection  = forwardConnections[std::get<1>(journeyStep)];
            journeyStepStopDeparture   = stops[std::get<connectionIndexes::STOP_DEP>(journeyStepEnterConnection)];
            journeyStepStopArrival     = stops[std::get<connectionIndexes::STOP_ARR>(journeyStepExitConnection)];
            journeyStepTrip            = trips[std::get<3>(journeyStep)];
            journeyStepRoute           = routes[routeIndexesById[journeyStepTrip.routeId]];
            transferTime               = std::get<4>(journeyStep);
            departureTime              = std::get<connectionIndexes::TIME_DEP>(journeyStepEnterConnection);
            arrivalTime                = std::get<connectionIndexes::TIME_ARR>(journeyStepExitConnection);
            boardingSequence           = std::get<connectionIndexes::SEQUENCE>(journeyStepEnterConnection);
            unboardingSequence         = std::get<connectionIndexes::SEQUENCE>(journeyStepExitConnection);
            inVehicleTime              = arrivalTime   - departureTime;
            waitingTime                = departureTime - transferArrivalTime;
            transferArrivalTime        = arrivalTime   + transferTime;
            transferReadyTime          = transferArrivalTime;
            totalInVehicleTime         += inVehicleTime;
            totalWaitingTime           += waitingTime;
            numberOfTransfers          += 1;
            legs.push_back(std::make_tuple(journeyStepTrip.id, journeyStepTrip.routeId, journeyStepTrip.routePathId, boardingSequence, unboardingSequence));
            
            if (i == 1) // first leg
            {
              accessWaitingTime  = waitingTime;
              firstDepartureTime = departureTime;
            }
            else
            {
              totalTransferWaitingTime += waitingTime;
            }
            
            if (!params.returnAllStopsResult)
            {
              stepJson                         = {};
              stepJson["action"]               = "board";
              stepJson["agencyAcronym"]        = journeyStepRoute.agencyAcronym;
              stepJson["agencyName"]           = journeyStepRoute.agencyName;
              stepJson["agencyId"]             = journeyStepRoute.agencyId;
              stepJson["routeShortname"]       = journeyStepRoute.shortname;
              stepJson["routeLongname"]        = journeyStepRoute.longname;
              stepJson["routeId"]              = journeyStepRoute.id;
              stepJson["routeTypeName"]        = journeyStepRoute.routeTypeName;
              stepJson["routeTypeId"]          = journeyStepRoute.routeTypeId;
              stepJson["tripId"]               = journeyStepTrip.id;
              stepJson["sequenceInTrip"]       = boardingSequence;
              stepJson["stopName"]             = journeyStepStopDeparture.name;
              stepJson["stopCode"]             = journeyStepStopDeparture.code;
              stepJson["stopId"]               = journeyStepStopDeparture.id;
              stepJson["stopCoordinates"]      = {journeyStepStopDeparture.point.longitude, journeyStepStopDeparture.point.latitude};
              stepJson["departureTime"]        = Toolbox::convertSecondsToFormattedTime(departureTime);
              stepJson["departureTimeSeconds"] = departureTime;
              stepJson["waitingTimeSeconds"]   = waitingTime;
              stepJson["waitingTimeMinutes"]   = Toolbox::convertSecondsToMinutes(waitingTime);
              json["steps"].push_back(stepJson);
              stepJson                         = {};
              stepJson["action"]               = "unboard";
              stepJson["agencyAcronym"]        = journeyStepRoute.agencyAcronym;
              stepJson["agencyName"]           = journeyStepRoute.agencyName;
              stepJson["agencyId"]             = journeyStepRoute.agencyId;
              stepJson["routeShortname"]       = journeyStepRoute.shortname;
              stepJson["routeLongname"]        = journeyStepRoute.longname;
              stepJson["routeId"]              = journeyStepRoute.id;
              stepJson["routeTypeName"]        = journeyStepRoute.routeTypeName;
              stepJson["routeTypeId"]          = journeyStepRoute.routeTypeId;
              stepJson["tripId"]               = journeyStepTrip.id;
              stepJson["sequenceInTrip"]       = unboardingSequence;
              stepJson["stopName"]             = journeyStepStopArrival.name;
              stepJson["stopCode"]             = journeyStepStopArrival.code;
              stepJson["stopId"]               = journeyStepStopArrival.id;
              stepJson["stopCoordinates"]      = {journeyStepStopArrival.point.longitude, journeyStepStopArrival.point.latitude};
              stepJson["arrivalTime"]          = Toolbox::convertSecondsToFormattedTime(arrivalTime);
              stepJson["arrivalTimeSeconds"]   = arrivalTime;
              stepJson["inVehicleTimeSeconds"] = inVehicleTime;
              stepJson["inVehicleTimeMinutes"] = Toolbox::convertSecondsToMinutes(inVehicleTime);
              json["steps"].push_back(stepJson);
            }
            if (i < journeyStepsCount - 2) // if not the last transit leg
            {
              totalTransferWalkingTime += transferTime;
              totalWalkingTime         += transferTime;
              if (!params.returnAllStopsResult)
              {
                stepJson                         = {};
                stepJson["action"]               = "walking";
                stepJson["type"]                 = "transfer";
                stepJson["travelTimeSeconds"]    = transferTime;
                stepJson["travelTimeMinutes"]    = Toolbox::convertSecondsToMinutes(transferTime);
                stepJson["departureTime"]        = Toolbox::convertSecondsToFormattedTime(arrivalTime);
                stepJson["arrivalTime"]          = Toolbox::convertSecondsToFormattedTime(transferArrivalTime);
                stepJson["departureTimeSeconds"] = arrivalTime;
                stepJson["arrivalTimeSeconds"]   = transferArrivalTime;
                stepJson["readyToBoardAt"]       = Toolbox::convertSecondsToFormattedTime(transferReadyTime + params.minWaitingTimeSeconds);
                json["steps"].push_back(stepJson);
              }
            }
          }
          else // access or egress journey step
          {
            
            transferTime = std::get<4>(journeyStep);
            if (i == 0) // access
            {
              transferArrivalTime  = departureTimeSeconds + transferTime;
              transferReadyTime    = transferArrivalTime;
              totalWalkingTime    += transferTime;
              accessWalkingTime    = transferTime;
              if (!params.returnAllStopsResult)
              {
                stepJson                         = {};
                stepJson["action"]               = "walking";
                stepJson["type"]                 = "access";
                stepJson["travelTimeSeconds"]    = transferTime;
                stepJson["travelTimeMinutes"]    = Toolbox::convertSecondsToMinutes(transferTime);
                stepJson["departureTime"]        = Toolbox::convertSecondsToFormattedTime(departureTimeSeconds);
                stepJson["arrivalTime"]          = Toolbox::convertSecondsToFormattedTime(transferArrivalTime);
                stepJson["departureTimeSeconds"] = departureTimeSeconds;
                stepJson["arrivalTimeSeconds"]   = transferArrivalTime;
                stepJson["readyToBoardAt"]       = Toolbox::convertSecondsToFormattedTime(transferReadyTime + params.minWaitingTimeSeconds);
                json["steps"].push_back(stepJson);
                
              }
            }
            else // egress
            {
              totalWalkingTime   += transferTime;
              egressWalkingTime   = transferTime;
              transferArrivalTime = arrivalTime + transferTime;
              if (!params.returnAllStopsResult)
              {
                stepJson                         = {};
                stepJson["action"]               = "walking";
                stepJson["type"]                 = "egress";
                stepJson["travelTimeSeconds"]    = transferTime;
                stepJson["travelTimeMinutes"]    = Toolbox::convertSecondsToMinutes(transferTime);
                stepJson["departureTime"]        = Toolbox::convertSecondsToFormattedTime(departureTimeSeconds);
                stepJson["arrivalTime"]          = Toolbox::convertSecondsToFormattedTime(arrivalTime + transferTime);
                stepJson["departureTimeSeconds"] = departureTimeSeconds;
                stepJson["arrivalTimeSeconds"]   = arrivalTime + transferTime;
                json["steps"].push_back(stepJson);
              }
              arrivalTime = transferArrivalTime;
            }
          }
          i++;
        }
        
        minimizedDepartureTime = firstDepartureTime - accessWalkingTime - params.minWaitingTimeSeconds;
        
        if (params.returnAllStopsResult)
        {
          arrivalTime = stopsTentativeTime[resultingStopIndex] - params.minWaitingTimeSeconds;
          if (arrivalTime - departureTimeSeconds <= params.maxTotalTravelTimeSeconds)
          {
            reachableStopsCount++;
            stopJson                           = {};
            stopJson["id"]                     = stops[resultingStopIndex].id;
            stopJson["arrivalTime"]            = Toolbox::convertSecondsToFormattedTime(arrivalTime);
            stopJson["totalTravelTimeSeconds"] = arrivalTime - departureTimeSeconds;
            stopJson["numberOfTransfers"]      = numberOfTransfers;
            json["stops"].push_back(stopJson);
          }
        }
        else if (!params.returnAllStopsResult)
        {
          if (numberOfTransfers >= 0)
          {

            json["status"]                                      = "success";
            json["origin"]                                      = {params.origin.longitude, params.origin.latitude};
            json["destination"]                                 = {params.destination.longitude, params.destination.latitude};
            json["departureTime"]                               = Toolbox::convertSecondsToFormattedTime(departureTimeSeconds);
            json["arrivalTime"]                                 = Toolbox::convertSecondsToFormattedTime(arrivalTime);
            json["departureTimeSeconds"]                        = departureTimeSeconds;
            json["arrivalTimeSeconds"]                          = arrivalTime;
            json["totalTravelTimeMinutes"]                      = Toolbox::convertSecondsToMinutes(arrivalTime - departureTimeSeconds);
            json["totalTravelTimeSeconds"]                      = arrivalTime - departureTimeSeconds;
            json["totalInVehicleTimeMinutes"]                   = Toolbox::convertSecondsToMinutes(totalInVehicleTime);
            json["totalInVehicleTimeSeconds"]                   = totalInVehicleTime;
            json["totalNonTransitTravelTimeMinutes"]            = Toolbox::convertSecondsToMinutes(totalWalkingTime);
            json["totalNonTransitTravelTimeSeconds"]            = totalWalkingTime;
            json["numberOfBoardings"]                           = numberOfTransfers + 1;
            json["numberOfTransfers"]                           = numberOfTransfers;
            json["transferWalkingTimeMinutes"]                  = Toolbox::convertSecondsToMinutes(totalTransferWalkingTime);
            json["transferWalkingTimeSeconds"]                  = totalTransferWalkingTime;
            json["accessTravelTimeMinutes"]                     = Toolbox::convertSecondsToMinutes(accessWalkingTime);
            json["accessTravelTimeSeconds"]                     = accessWalkingTime;
            json["egressTravelTimeMinutes"]                     = Toolbox::convertSecondsToMinutes(egressWalkingTime);
            json["egressTravelTimeSeconds"]                     = egressWalkingTime;
            json["transferWaitingTimeMinutes"]                  = Toolbox::convertSecondsToMinutes(totalTransferWaitingTime);
            json["transferWaitingTimeSeconds"]                  = totalTransferWaitingTime;
            json["firstWaitingTimeMinutes"]                     = Toolbox::convertSecondsToMinutes(accessWaitingTime);
            json["firstWaitingTimeSeconds"]                     = accessWaitingTime;
            json["totalWaitingTimeMinutes"]                     = Toolbox::convertSecondsToMinutes(totalWaitingTime);
            json["totalWaitingTimeSeconds"]                     = totalWaitingTime;
            json["departureTimeToMinimizeFirstWaitingTime"]     = Toolbox::convertSecondsToFormattedTime(minimizedDepartureTime);
            json["minimizedTotalTravelTimeMinutes"]             = Toolbox::convertSecondsToMinutes(arrivalTime - minimizedDepartureTime);
            json["minimizedTotalTravelTimeSeconds"]             = arrivalTime - minimizedDepartureTime;
            json["minimumWaitingTimeBeforeEachBoardingMinutes"] = Toolbox::convertSecondsToMinutes(params.minWaitingTimeSeconds);
            json["minimumWaitingTimeBeforeEachBoardingSeconds"] = params.minWaitingTimeSeconds;

            result.travelTimeSeconds    = arrivalTime - departureTimeSeconds;
            result.arrivalTimeSeconds   = arrivalTime;
            result.departureTimeSeconds = departureTimeSeconds;
            result.numberOfTransfers    = numberOfTransfers;
            result.legs                 = legs;
            result.status               = "success";
            
          }
        }
      }
    }
    else // no route found
    {
      json["status"]               = "no_routing_found";
      json["origin"]               = { params.origin.longitude,      params.origin.latitude };
      json["destination"]          = { params.destination.longitude, params.destination.latitude };
      json["departureTime"]        = Toolbox::convertSecondsToFormattedTime(departureTimeSeconds);
      json["departureTimeSeconds"] = departureTimeSeconds;
      result.status                = "no_routing_found";
      result.travelTimeSeconds     = -1;
    }

    if (params.returnAllStopsResult)
    {
      json["numberOfReachableStops"]  = reachableStopsCount;
      json["percentOfReachableStops"] = round(10000 * (float)reachableStopsCount / (float)(stopsCount))/100.0;
    }

    result.json = json.dump(2); // number of spaces in indent for human readable json, use dump() to put all json content on the same line
    return result;

  }

}