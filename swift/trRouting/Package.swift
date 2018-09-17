//
//  Package.swift
//  trRouting
//
//  Created by Pierre-Léo Bourbonnais on 2017-10-21.
//  Copyright © 2017 Transition. All rights reserved.
//

import Foundation
import PackageDescription

let package = Package(
  name: "trRouting",
  dependencies: [
    .Package(url: "https://github.com/a2/MessagePack.swift.git", majorVersion: 3),
    .Package(url: "https://github.com/vapor/vapor.git", majorVersion: 2),
    .Package(url: "https://github.com/vapor-community/postgresql-provider.git", majorVersion: 2),
    .Package(url: "https://github.com/rxwei/cuda-swift", majorVersion: 1)
      
  ]/*,
  exclude: []*/
)
