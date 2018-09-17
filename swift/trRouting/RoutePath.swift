//
//  RoutePath.swift
//  trRouting
//
//  Created by Pierre-Léo Bourbonnais on 2017-10-24.
//
//

import Foundation
import "Route"
import "RouteType"
import "Agency"

struct RoutePath
{
  var id          : Int
  var routeId     : Int
  var route       : Route
  var routeTypeId : Int
  var routeType   : RouteType
  var agencyId    : Int
  var agency      : Agency
}
