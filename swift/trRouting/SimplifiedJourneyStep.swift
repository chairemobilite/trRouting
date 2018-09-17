//
//  SimplifiedJourneyStep.swift
//  trRouting
//
//  Created by Pierre-Léo Bourbonnais on 2017-10-21.
//
//

import Foundation

enum JourneyStepAction
{ 
  case WALK
  case CYCLE
  case DRIVE
  case RIDE
  case BOARD
  case UNBOARD
} // CYCLE, DRIVE are not yet implemented

struct SimplifiedJourneyStep
{
  var action                  : JourneyStepAction
  var id                      : Int // board, ride, unboard
  var connectionId            : Int // walk, cycle, drive
  var accessTimeMinutes       : Int // walk, cycle, drive
  var accessFromStopId        : Int // walk, cycle, drive
  var accessFromTripId        : Int // walk, cycle, drive
  var readyToBoardMinuteOfDay : Int // walk, cycle, drive
}
