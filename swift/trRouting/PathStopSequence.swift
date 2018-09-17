//
//  PathStopSequence.swift
//  trRouting
//
//  Created by Pierre-Léo Bourbonnais on 2017-10-24.
//
//

import Foundation
import "Stop"
import "RoutePath"
import "Route"
import "RouteType"
import "Agency"

struct PathStopSequence
{
  var id          : Int
  var stopId      : Int
  var stop        : Stop
  var routePathId : Int
  var routePath   : RoutePath
  var routeId     : Int
  var route       : Route
  var routeTypeId : Int
  var routeType   : RouteType
  var agencyId    : Int
  var Agency      : Agency
  var sequence    : Int
}
