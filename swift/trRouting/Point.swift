//
//  Point.swift
//  trRouting
//
//  Created by Pierre-Léo Bourbonnais on 2017-10-23.
//
//

import Foundation

struct Point
{

  var latitude  : Float
  var longitude : Float
  
  init() {}
  
  init(latitude: Float, longitude: Float)
  {
    self.latitude  = latitude
    self.longitude = longitude
  }
  
  func asWKT() -> String
  {
    return "POINT(\(longitude) \(latitude))"
  }
  
}
