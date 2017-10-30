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
  var params                                                     : Parameters
  
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
    self.setup()
  }
  
  func setup() -> Void
  {
    self.calculationId = 1
    self.resetAccessEgressModes()
    self.maxTimeValue = 9999 // that's almost 7 days. No travel time should take that long.
    if (self.params.databasePassword != nil)
    {
      //DbFetcher::setDbSetupStr("dbname=" + params.databaseName + " user=" + params.databaseUser + " hostaddr=" + params.databaseHost + " password=" + params.databasePassword + " port=" + params.databasePort + ""); // todo: add config to set this
    }
    else
    {
      //DbFetcher::setDbSetupStr("dbname=" + params.databaseName + " user=" + params.databaseUser + " hostaddr=" + params.databaseHost + " port=" + params.databasePort + ""); // todo: add config to set this
    }
    //DbFetcher::disconnect();
    //pickUpTypes                          = DbFetcher::getPickUpTypes(params.applicationShortname);
    //dropOffTypes                         = DbFetcher::getDropOffTypes(params.applicationShortname);
    //transferDurationsByStopId            = DbFetcher::getTransferDurationsByStopId(params.applicationShortname, params.dataFetcher, params.maxTransferWalkingTravelTimeMinutes, params.transfersSqlWhereClause);
    //pathStopSequencesById                = DbFetcher::getPathStopSequencesById(params.applicationShortname, params.dataFetcher);
    //stopsById                            = DbFetcher::getStopsById(params.applicationShortname, params.dataFetcher, maxTimeValue);
    //routesById                           = DbFetcher::getRoutesById(params.applicationShortname, params.dataFetcher);
    //forwardConnectionsById               = DbFetcher::getConnectionsById(params.applicationShortname, params.dataFetcher, params.connectionsSqlWhereClause, params);
    //reverseConnectionsById               = forwardConnectionsById;
  }
  
  func refresh() -> Void
  {
    
  }
  
  func updateParams() -> Void
  {
    
  }
  
  func resetAccessEgressModes() -> Void
  {
    self.accessMode = self.params.accessMode;
    self.egressMode = self.params.egressMode;
    self.maxAccessWalkingTravelTimeFromOriginToFirstStopMinutes     = self.params.maxAccessWalkingTravelTimeFromOriginToFirstStopMinutes;
    self.maxAccessWalkingTravelTimeFromLastStopToDestinationMinutes = self.params.maxAccessWalkingTravelTimeFromLastStopToDestinationMinutes;
  }
  
  func calculate() -> RoutingResult
  {
    
  }
  
}
