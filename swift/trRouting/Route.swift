//
//  Route.swift
//  trRouting
//
//  Created by Pierre-Léo Bourbonnais on 2017-10-24.
//
//

import Foundation
import "Agency"
import "RouteType"

struct Route
{
  var id          : Int
  var agencyId    : Int
  var agency      : Agency
  var routeTypeId : Int
  var routeType   : RouteType
}
