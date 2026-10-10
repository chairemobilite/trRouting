use super::*;

const fn hm(hours: i32, minutes: i32) -> i32 {
  hours * 3600 + minutes * 60
}

// Nodes 0 to 4. Line 10: trip 0, 0 -> 1 -> 2 at 8:00. Line 20: trip 1, 2 -> 3
// at 8:30. Line 30: trip 2, 0 -> 3 at 9:00. Every node can transfer to
// itself, node 1 also to node 4 in 5 minutes.
const CONNECTIONS: Connections<'static> = Connections {
  departure_node: &[0, 1, 2, 0],
  arrival_node: &[1, 2, 3, 3],
  departure_time: &[hm(8, 0), hm(8, 10), hm(8, 30), hm(9, 0)],
  arrival_time: &[hm(8, 10), hm(8, 20), hm(8, 40), hm(9, 50)],
  trip: &[0, 0, 1, 2],
  min_waiting_time: &[-1, -1, -1, -1],
  flags: &[CAN_BOARD | CAN_UNBOARD; 4],
  trip_line: &[10, 20, 30],
};
const TRANSFERS: Transfers<'static> = Transfers {
  offsets: &[0, 1, 3, 4, 5, 6],
  node: &[0, 1, 4, 2, 3, 4],
  time: &[0, 0, 300, 0, 0, 0],
};
const DEFAULT_PARAMETERS: Parameters = Parameters {
  min_waiting_time: 180,
  max_total_travel_time: i32::MAX,
  max_transfer_walking_time: 1200,
  max_inner_time_of_trip_buffer: 0,
  walking_speed_factor: 1.0,
};

fn footpath(node: i32, time: i32) -> Footpath {
  Footpath { node, time }
}

fn arrival(time: i32, egress_node: i32) -> Result<Arrival, NoRouting> {
  Ok(Arrival { time, egress_node })
}

struct Case {
  name: &'static str,
  departure_time: i32,
  access: Vec<Footpath>,
  egress: Vec<Footpath>,
  excluded_lines: Vec<i32>,
  expected: Result<Arrival, NoRouting>,
}

fn case(
  name: &'static str,
  departure_time: i32,
  access: Vec<Footpath>,
  egress: Vec<Footpath>,
  excluded_lines: Vec<i32>,
  expected: Result<Arrival, NoRouting>,
) -> Case {
  Case {
    name,
    departure_time,
    access,
    egress,
    excluded_lines,
    expected,
  }
}

// Run all the cases in one batch and compare each result
fn assert_batch(parameters: &Parameters, cases: &[Case]) {
  let queries: Vec<Query> = cases
    .iter()
    .map(|case| Query {
      departure_time: case.departure_time,
      access: &case.access,
      egress: &case.egress,
      excluded_lines: &case.excluded_lines,
    })
    .collect();
  let results = calculate(&CONNECTIONS, &TRANSFERS, parameters, &queries);
  assert_eq!(cases.len(), results.len());
  for (case, result) in cases.iter().zip(results) {
    assert_eq!(case.expected, result, "{}", case.name);
  }
}

#[test]
fn test_default_parameters() {
  let origin = vec![footpath(0, 60)];
  assert_batch(
    &DEFAULT_PARAMETERS,
    &[
      case(
        "direct trip",
        hm(7, 50),
        origin.clone(),
        vec![footpath(2, 120)],
        vec![],
        arrival(hm(8, 22), 2),
      ),
      case(
        "one transfer",
        hm(7, 50),
        origin.clone(),
        vec![footpath(3, 0)],
        vec![],
        arrival(hm(8, 40), 3),
      ),
      case(
        "excluded line",
        hm(7, 50),
        origin.clone(),
        vec![footpath(3, 0)],
        vec![20],
        arrival(hm(9, 50), 3),
      ),
      case(
        "excluded lines",
        hm(7, 50),
        origin.clone(),
        vec![footpath(3, 0)],
        vec![20, 30],
        Err(NoRouting::NoRoutingFound),
      ),
      case(
        "egress reached by transfer only",
        hm(7, 50),
        origin.clone(),
        vec![footpath(4, 60)],
        vec![],
        Err(NoRouting::NoRoutingFound),
      ),
      case(
        "best of two egress nodes",
        hm(7, 50),
        origin.clone(),
        vec![footpath(3, 0), footpath(2, 600)],
        vec![],
        arrival(hm(8, 30), 2),
      ),
      case(
        "min waiting time",
        hm(7, 59),
        vec![footpath(0, 0)],
        vec![footpath(3, 0)],
        vec![],
        arrival(hm(9, 50), 3),
      ),
      case(
        "no service",
        hm(10, 0),
        origin.clone(),
        vec![footpath(3, 0)],
        vec![],
        Err(NoRouting::NoServiceFromOrigin),
      ),
      case(
        "no access",
        hm(7, 50),
        vec![],
        vec![footpath(3, 0)],
        vec![],
        Err(NoRouting::NoAccessAtOrigin),
      ),
      case(
        "no egress",
        hm(7, 50),
        origin.clone(),
        vec![],
        vec![],
        Err(NoRouting::NoAccessAtDestination),
      ),
      case(
        "no access nor egress",
        hm(7, 50),
        vec![],
        vec![],
        vec![],
        Err(NoRouting::NoAccessAtOriginAndDestination),
      ),
    ],
  );
}

#[test]
fn test_inner_time_of_trip_buffer() {
  let parameters = Parameters {
    max_inner_time_of_trip_buffer: 1800,
    ..DEFAULT_PARAMETERS
  };
  assert_batch(
    &parameters,
    &[
      case(
        "within buffer",
        hm(7, 40),
        vec![footpath(0, 0)],
        vec![footpath(3, 0)],
        vec![],
        arrival(hm(8, 40), 3),
      ),
      case(
        "beyond buffer",
        hm(7, 0),
        vec![footpath(0, 0)],
        vec![footpath(3, 0)],
        vec![],
        Err(NoRouting::NoServiceFromOrigin),
      ),
    ],
  );
}

#[test]
fn test_max_total_travel_time() {
  let parameters = Parameters {
    max_total_travel_time: 1800,
    ..DEFAULT_PARAMETERS
  };
  assert_batch(
    &parameters,
    &[
      case(
        "within max",
        hm(7, 50),
        vec![footpath(0, 0)],
        vec![footpath(2, 0)],
        vec![],
        arrival(hm(8, 20), 2),
      ),
      case(
        "beyond max",
        hm(7, 50),
        vec![footpath(0, 0)],
        vec![footpath(3, 0)],
        vec![],
        Err(NoRouting::NoRoutingFound),
      ),
    ],
  );
}
