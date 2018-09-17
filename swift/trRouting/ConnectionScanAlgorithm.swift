//
//  ConnectionScanAlgorithm.swift
//  trRouting
//
//  Created by Pierre-Leo Bourbonnais on 2017-10-21.
//
//

import Foundation
import "Parameters"
import "Connection"
import "RoutingResult"
import "Stop"
import "Agency"
import "Route"
import "RouteType"
import "RoutePath"
import "Station"
import "Point"
import "SimplifiedJourneyStep"
import "PathStopSequence"

class ConnectionScanAlgorithm
{
  
  var calculationId                                              : Int
  var parameters                                                 : Parameters
  
  var pickUpTypes                                                : [String: Int]
  var dropOffTypes                                               : [String: Int]
  var transferDurationsByStopId                                  : [Int: [Int: Int]]
  var pathStopSequencesById                                      : [Int: PathStopSequence]
  var pathStopSequencesByStopId                                  : [Int: [Int]]
  var stopsById                                                  : [Int: Stop]
  var routesById                                                 : [Int: Route]
  var forwardConnectionsById                                     : [Int: Connection]
  var reverseConnectionsById                                     : [Int: Connection]
  var connectionsByArrivalTime                                   : [Connection]
  var connectionsByDepartureTime                                 : [Connection]
  var connectionsByStartPathStopSequenceId                       : [Int: [Connection]]
  var connectionsByEndPathStopSequenceId                         : [Int: [Connection]]
  
  var maxUnboardingTimeMinutes                                   : Int
  var maxTimeValue                                               : Int
  var accessMode                                                 : String
  var egressMode                                                 : String
  var maxAccessWalkingTravelTimeFromOriginToFirstStopMinutes     : Int
  var maxAccessWalkingTravelTimeFromLastStopToDestinationMinutes : Int
  
  init(parameters: Parameters)
  {
    self.parameters = parameters
    
  }
  
  func setup() -> Void
  {
    
  }
  
  func refresh() -> Void
  {
    
  }
  
  func updateParams() -> Void
  {
    
  }
  
  func resetAccessEgressModes() -> Void
  {
    
  }
  
  func calculate() -> RoutingResult
  {
    
  }
  
}
