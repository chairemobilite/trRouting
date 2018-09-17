//
//  Parameters.swift
//  trRouting
//
//  Created by Pierre-Léo Bourbonnais on 2017-10-23.
//
//

import Foundation
import "Point"

struct Parameters
{
  var applicationShortname                                       : String
  var dataFetcher                                                : String // database (transition), cache (Message Pack files), gtfs (not yet implemented)
  
  var onlyServiceIds                                             : [Int]
  var exceptServiceIds                                           : [Int]
  var onlyRouteIds                                               : [Int]
  var exceptRouteIds                                             : [Int]
  var onlyRouteTypeIds                                           : [Int]
  var exceptRouteTypeIds                                         : [Int]
  var onlyAgencyIds                                              : [Int]
  var exceptAgencyIds                                            : [Int]
  
  var departureTimeHour                                          : Int
  var departureTimeMinutes                                       : Int
  var arrivalTimeHour                                            : Int
  var arrivalTimeMinutes                                         : Int
    
  var maxTotalTravelTimeMinutes                                  : Int
  var maxNumberOfTransfers                                       : Int
  var minWaitingTimeMinutes                                      : Int
  var transferPenaltyMinutes                                     : Int
  var maxAccessWalkingDistanceMeters                             : Int
  var maxAccessWalkingTravelTimeFromOriginToFirstStopMinutes     : Int
  var maxAccessWalkingTravelTimeFromLastStopToDestinationMinutes : Int
  var maxTransferWalkingTravelTimeMinutes                        : Int
  var maxTotalWalkingTravelTimeMinutes                           : Int
  var maxOnlyWalkingAccessTravelTimeRatio                        : Float
  var walkingSpeedMetersPerSecond                                : Float
  var drivingSpeedMetersPerSecond                                : Float
  var cyclingSpeedMetersPerSecond                                : Float
  
  var startingPoint                                              : Point
  var endingPoint                                                : Point
  var startingStopId                                             : Int
  var endingStopId                                               : Int
  
  var connectionsSqlWhereClause                                  : String
  var transfersSqlWhereClause                                    : String
  var databaseName                                               : String
  var databaseHost                                               : String
  var databaseUser                                               : String
  var databasePort                                               : String
  var databasePassword                                           : String
  var osrmRoutingWalkingPort                                     : String
  var osrmRoutingWalkingHost                                     : String
  var osrmRoutingDrivingPort                                     : String
  var osrmRoutingDrivingHost                                     : String
  var osrmRoutingCyclingPort                                     : String
  var osrmRoutingCyclingHost                                     : String
  
  var accessMode                                                 : String
  var egressMode                                                 : String
  var tryNextModeIfRoutingFails                                  : Bool
  var noResultSecondMode                                         : String
  var noResultNextAccessTimeMinutesIncrement                     : Int
  var maxNoResultNextAccessTimeMinutes                           : Int
  
  var returnAllStopsResult                                       : Bool // keep results for all stops (used in creating accessibility map)
  var forwardCalculation                                         : Bool // forward calculation: default. if false: reverse calculation, will ride connections backward (useful when setting the arrival time)
  var detailedResults                                            : Bool // return detailed results when using results for all stops
  var transferOnlyAtSameStation                                  : Bool // will transfer only between stops having the same station_id (better performance, but make sure your stations are well designed and specified)
  var transferBetweenSameRoute                                   : Bool // allow transfers between the same route_id
  var calculateByNumberOfTransfers                               : Bool // calculate first the fastest route, then calculate with decreasing number of transfers until no route is found, return results for each number of transfers.
  
  func setDefaultValues() -> Void
  {
    self.walkingSpeedMetersPerSecond                                = 5/3.6  // 5 km/h
    self.drivingSpeedMetersPerSecond                                = 90/3.6 // 90 km/h
    self.cyclingSpeedMetersPerSecond                                = 25/3.6 // 25 km/h
    self.maxTotalTravelTimeMinutes                                  = -1 // -1 means no limit
    self.maxNumberOfTransfers                                       = -1 // -1 means no limit
    self.minWaitingTimeMinutes                                      = 5
    self.maxAccessWalkingTravelTimeFromOriginToFirstStopMinutes     = 20
    self.maxAccessWalkingTravelTimeFromLastStopToDestinationMinutes = 20
    self.maxTransferWalkingTravelTimeMinutes                        = 20 // depends of transfer data provided
    self.maxTotalWalkingTravelTimeMinutes                           = 60 // not used right now
    self.maxOnlyWalkingAccessTravelTimeRatio                        = 1.5 // prefer walking only if it is faster than transit and total only walking travel time <= maxAccessWalkingTravelTimeFromOriginToFirstStopMinutes * this ratio
    self.transferPenaltyMinutes                                     = 0 // not used right now
    self.connectionsSqlWhereClause                                  = ""
    self.transfersSqlWhereClause                                    = "true"
    self.databaseName                                               = "tr_all_dev"
    self.databasePort                                               = "5432"
    self.databaseHost                                               = "127.0.0.1"
    self.databaseUser                                               = "postgres"
    self.databasePassword                                           = ""
    self.osrmRoutingWalkingHost                                     = "localhost"
    self.osrmRoutingWalkingPort                                     = "5000"
    self.osrmRoutingDrivingHost                                     = "localhost"
    self.osrmRoutingDrivingPort                                     = "7000"
    self.osrmRoutingCyclingHost                                     = "localhost"
    self.osrmRoutingCyclingPort                                     = "8000"
    self.accessMode                                                 = "walking"
    self.egressMode                                                 = "walking"
    self.noResultSecondMode                                         = "driving"
    self.tryNextModeIfRoutingFails                                  = false
    self.noResultNextAccessTimeMinutesIncrement                     = 5
    self.maxNoResultNextAccessTimeMinutes                           = 40
    self.returnAllStopsResult                                       = false
    self.forwardCalculation                                         = true
    self.detailedResults                                            = false
    self.transferOnlyAtSameStation                                  = false
    self.transferBetweenSameRoute                                   = true
    self.calculateByNumberOfTransfers                               = false
  }
    
}
