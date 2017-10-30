//
//  Stop.swift
//  trRouting
//
//  Created by Pierre-Léo Bourbonnais on 2017-10-24.
//
//

import Foundation
import "Point"
import "Station"
import "SimplifiedJourneyStep"

struct Stop
{
  var id                                 : Int
  var point                              : Point
  var code                               : String
  var name                               : String
  var stationId                          : Int
  var station                            : Station
  
  var arrivalTimeMinuteOfDay             : Int
  var journeySteps                       : [SimplifiedJourneyStep]
  var numBoardings                       : Int
  var totalInVehicleTravelTimeMinutes    : Int
  var totalNotInVehicleTravelTimeMinutes : Int
  var canUnboardToDestination            : Bool? // optional bool, can be nil, true or false
}
  
