//
//  Connection.swift
//  trRouting
//
//  Created by Pierre-Leo Bourbonnais on 2017-10-21.
//
//

import Foundation

struct Connection
{
    var id : Int
    var serviceId : Int
    var agencyId : Int
    var routeId : Int
    var routeTypeId : Int
    var tripId : Int
    var sequence : Int
    var departureFromOriginTimeMinuteOfDay : Int
    var arrivalAtDestinationTimeMinuteOfDay : Int
    var pathStopSequenceStartId : Int
    var pathStopSequenceEndId : Int
    var stopStartId : Int
    var stopEndId : Int
    var canBoard : Bool
    var canUnboard : Bool
    var reachable : Int // ConnectionScanAlgorithm::calculationId will be assigned for each reachable connections
    var enabled: Int // connection is enabled in database
    var calculationEnabled: Int // connection is enabled for a single calculation (after filtering route ids, route type ids, service ids, etc.)
    var nextConnectionId : Int
    var previousConnectionId : Int
    var numBoardings: Int
    var totalInVehicleTravelTimeMinutes: Int
    var totalNotInVehicleTravelTimeMinutes: Int
    var journeySteps : [SimplifiedJourneyStep]
    var lastJourneyStepIndex: Int
}
