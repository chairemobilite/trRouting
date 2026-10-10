//! Forward CSA of many queries at once. For each query, it returns the same
//! best arrival time and egress node as `Calculator::reset` followed by
//! `Calculator::forwardCalculation` in trRouting, without the journey steps.
//!
//! The state of each node and trip is stored for all queries next to each
//! other (`state[index * queries_count + query]`), so the inner loop over the
//! queries reads contiguous memory. Memory use is proportional to
//! (nodes + trips) * queries.

#[cfg(test)]
mod tests;

/// `Connections::flags` bit: passengers can board at the departure node
pub const CAN_BOARD: u32 = 1;
/// `Connections::flags` bit: passengers can unboard at the arrival node
pub const CAN_UNBOARD: u32 = 2;

/// Forward connections sorted by departure time, one slice per field, like
/// `CompactConnectionSet` in trRouting. Nodes are identified by their uid,
/// trips by their index in `trip_line`.
pub struct Connections<'a> {
  pub departure_node: &'a [i32],
  pub arrival_node: &'a [i32],
  pub departure_time: &'a [i32],
  pub arrival_time: &'a [i32],
  pub trip: &'a [i32],
  /// -1 to use `Parameters::min_waiting_time`
  pub min_waiting_time: &'a [i32],
  /// Combination of `CAN_BOARD` and `CAN_UNBOARD`
  pub flags: &'a [u32],
  /// Line uid of each trip
  pub trip_line: &'a [i32],
}

/// Transferable nodes in compressed sparse row form indexed by node uid, like
/// `CompactTransfers` in trRouting: the transfers from node `n` are at
/// `offsets[n]..offsets[n + 1]` of `node` and `time`.
pub struct Transfers<'a> {
  pub offsets: &'a [i32],
  pub node: &'a [i32],
  /// Walking time in seconds, before the walking speed factor
  pub time: &'a [i32],
}

/// Parameters shared by all the queries of a batch, in seconds
pub struct Parameters {
  pub min_waiting_time: i32,
  pub max_total_travel_time: i32,
  pub max_transfer_walking_time: i32,
  pub max_inner_time_of_trip_buffer: i32,
  pub walking_speed_factor: f32,
}

/// Walking time in seconds to a node, before the walking speed factor
#[repr(C)]
#[derive(Clone, Copy)]
pub struct Footpath {
  pub node: i32,
  pub time: i32,
}

pub struct Query<'a> {
  pub departure_time: i32,
  pub access: &'a [Footpath],
  pub egress: &'a [Footpath],
  /// The trips of these lines are not used, like `AlternativeLineFilter`
  pub excluded_lines: &'a [i32],
}

/// Same reasons and order as `NoRoutingReason` in trRouting
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum NoRouting {
  NoRoutingFound,
  NoAccessAtOrigin,
  NoAccessAtDestination,
  NoServiceFromOrigin,
  NoServiceToDestination,
  NoAccessAtOriginAndDestination,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Arrival {
  pub time: i32,
  pub egress_node: i32,
}

const NODE_ACCESSED: u8 = 1; // The node is in the access footpaths
const NODE_REACHED_BY_TRANSIT: u8 = 2; // The tentative time comes from a transit trip
const TRIP_ENTERED: u8 = 1;
const TRIP_DISABLED: u8 = 2;
const UNREACHED: i32 = i32::MAX; // MAX_INT in trRouting

/// Same rounding as `Calculator::forwardCalculation` for the transfer footpaths
fn transfer_walking_time(time: i32, walking_speed_factor: f32) -> i32 {
  if walking_speed_factor == 1.0 {
    time
  } else {
    walking_time(time, walking_speed_factor)
  }
}

/// Same rounding as `Calculator::reset` for the access and egress footpaths
fn walking_time(time: i32, walking_speed_factor: f32) -> i32 {
  (time as f32 / walking_speed_factor).ceil() as i32
}

/// State of a batch. Per node and per trip vectors are indexed by
/// `index * queries_count + query`.
struct BatchState {
  queries_count: usize,
  min_access_time: Vec<i32>,
  max_egress_time: Vec<i32>,
  /// Arrival time at the first egress node reached, for the early stop
  first_egress_arrival_time: Vec<i32>,
  reachable_connections_count: Vec<u32>,
  /// False when the query has no access/egress or reached its stop condition
  scanning: Vec<bool>,
  /// Arrival time at the node
  tentative_time: Vec<i32>,
  /// Egress walking time from the node, -1 if not an egress node
  egress_time: Vec<i32>,
  /// Earliest arrival at the node by a trip, when its self transfer is allowed
  exit_arrival_time: Vec<i32>,
  node_flags: Vec<u8>,
  trip_flags: Vec<u8>,
}

impl BatchState {
  /// Same initialization as `Calculator::reset`, with `AlternativeLineFilter`
  /// for the excluded lines
  fn new(
    nodes_count: usize,
    connections: &Connections,
    parameters: &Parameters,
    queries: &[Query],
  ) -> Self {
    let n = queries.len();
    let mut state = BatchState {
      queries_count: n,
      min_access_time: vec![UNREACHED; n],
      max_egress_time: vec![-1; n],
      first_egress_arrival_time: vec![UNREACHED; n],
      reachable_connections_count: vec![0; n],
      scanning: vec![false; n],
      tentative_time: vec![UNREACHED; nodes_count * n],
      egress_time: vec![-1; nodes_count * n],
      exit_arrival_time: vec![UNREACHED; nodes_count * n],
      node_flags: vec![0; nodes_count * n],
      trip_flags: vec![0; connections.trip_line.len() * n],
    };
    for (q, query) in queries.iter().enumerate() {
      for footpath in query.access {
        let time = walking_time(footpath.time, parameters.walking_speed_factor);
        let node = footpath.node as usize * n + q;
        state.tentative_time[node] = query.departure_time + time;
        state.node_flags[node] |= NODE_ACCESSED;
        state.min_access_time[q] = state.min_access_time[q].min(time);
      }
      for footpath in query.egress {
        let time = walking_time(footpath.time, parameters.walking_speed_factor);
        let egress_time = &mut state.egress_time[footpath.node as usize * n + q];
        // Keep the first footpath of a node, like nodesEgress.emplace
        if *egress_time == -1 {
          *egress_time = time;
        }
        state.max_egress_time[q] = state.max_egress_time[q].max(time);
      }
      for (trip, line) in connections.trip_line.iter().enumerate() {
        if query.excluded_lines.contains(line) {
          state.trip_flags[trip * n + q] |= TRIP_DISABLED;
        }
      }
      state.scanning[q] = !query.access.is_empty() && !query.egress.is_empty();
    }
    state
  }

  /// Same rules as the main loop of `Calculator::forwardCalculation`, for
  /// every scanning query
  fn scan(
    &mut self,
    connections: &Connections,
    transfers: &Transfers,
    parameters: &Parameters,
    queries: &[Query],
  ) {
    let n = self.queries_count;
    let transfer_time: Vec<i32> = transfers
      .time
      .iter()
      .map(|&time| transfer_walking_time(time, parameters.walking_speed_factor))
      .collect();

    // Start at the earliest connection any query can board
    let earliest_departure_time = (0..n)
      .filter(|&q| self.scanning[q])
      .map(|q| queries[q].departure_time + self.min_access_time[q])
      .min()
      .unwrap_or(UNREACHED);
    let first = connections
      .departure_time
      .partition_point(|&time| time < earliest_departure_time);

    for c in first..connections.departure_time.len() {
      let departure_time = connections.departure_time[c];
      let min_waiting_time = if connections.min_waiting_time[c] >= 0 {
        connections.min_waiting_time[c]
      } else {
        parameters.min_waiting_time
      };
      let departure_node = connections.departure_node[c] as usize * n;
      let arrival_node_uid = connections.arrival_node[c] as usize;
      let arrival_node = arrival_node_uid * n;
      let arrival_time = connections.arrival_time[c];
      let trip = connections.trip[c] as usize * n;
      let mut any_scanning = false;

      for (q, query) in queries.iter().enumerate() {
        if !self.scanning[q] {
          continue;
        }
        any_scanning = true;
        let query_departure_time = query.departure_time;
        let trip_flags = self.trip_flags[trip + q];
        if departure_time < query_departure_time + self.min_access_time[q]
          || trip_flags & TRIP_DISABLED != 0
        {
          continue;
        }
        // Departure time and not arrival time, see Calculator::forwardCalculation
        let first_egress_arrival_time = self.first_egress_arrival_time[q];
        if (first_egress_arrival_time < UNREACHED
          && self.max_egress_time[q] >= 0
          && departure_time > first_egress_arrival_time + self.max_egress_time[q])
          || departure_time - query_departure_time > parameters.max_total_travel_time
        {
          self.scanning[q] = false;
          continue;
        }

        let node_departure_time = self.tentative_time[departure_node + q];
        let node_flags = self.node_flags[departure_node + q];
        let accessed_from_origin = parameters.max_inner_time_of_trip_buffer > 0
          && node_flags & NODE_ACCESSED != 0
          && node_flags & NODE_REACHED_BY_TRANSIT == 0;
        let can_reach = trip_flags & TRIP_ENTERED != 0
          || node_departure_time <= departure_time - min_waiting_time;
        if !can_reach
          || (accessed_from_origin
            && departure_time - node_departure_time > parameters.max_inner_time_of_trip_buffer)
        {
          continue;
        }
        self.reachable_connections_count[q] += 1;

        if connections.flags[c] & CAN_BOARD != 0 {
          self.trip_flags[trip + q] |= TRIP_ENTERED;
        }
        if connections.flags[c] & CAN_UNBOARD == 0 || self.trip_flags[trip + q] & TRIP_ENTERED == 0
        {
          continue;
        }
        if first_egress_arrival_time == UNREACHED && self.egress_time[arrival_node + q] != -1 {
          self.first_egress_arrival_time[q] = arrival_time;
        }
        let transfers_range = transfers.offsets[arrival_node_uid] as usize
          ..transfers.offsets[arrival_node_uid + 1] as usize;
        for t in transfers_range {
          let transfer_node = transfers.node[t] as usize * n + q;
          let self_transfer = transfer_node == arrival_node + q;
          if (!self_transfer && self.tentative_time[transfer_node] < arrival_time)
            || transfer_time[t] > parameters.max_transfer_walking_time
          {
            continue;
          }
          if transfer_time[t] + arrival_time < self.tentative_time[transfer_node] {
            self.tentative_time[transfer_node] = transfer_time[t] + arrival_time;
            self.node_flags[transfer_node] |= NODE_REACHED_BY_TRANSIT;
          }
          if self_transfer {
            self.exit_arrival_time[transfer_node] =
              self.exit_arrival_time[transfer_node].min(arrival_time);
          }
        }
      }
      if !any_scanning {
        break;
      }
    }
  }

  /// Same best egress selection and no routing reasons as `Calculator::reset`,
  /// `Calculator::forwardCalculation` and `Calculator::calculateSingle`
  fn result(&self, parameters: &Parameters, query: &Query, q: usize) -> Result<Arrival, NoRouting> {
    match (query.access.is_empty(), query.egress.is_empty()) {
      (true, true) => return Err(NoRouting::NoAccessAtOriginAndDestination),
      (true, false) => return Err(NoRouting::NoAccessAtOrigin),
      (false, true) => return Err(NoRouting::NoAccessAtDestination),
      (false, false) => {}
    }
    if self.reachable_connections_count[q] == 0 {
      return Err(NoRouting::NoServiceFromOrigin);
    }
    let mut best = Arrival {
      time: UNREACHED,
      egress_node: -1,
    };
    for footpath in query.egress {
      let node = footpath.node as usize * self.queries_count + q;
      if self.exit_arrival_time[node] == UNREACHED {
        continue;
      }
      let time = self.exit_arrival_time[node] + self.egress_time[node];
      if time >= 0
        && time - query.departure_time <= parameters.max_total_travel_time
        && time < best.time
      {
        best = Arrival {
          time,
          egress_node: footpath.node,
        };
      }
    }
    if best.time == UNREACHED {
      Err(NoRouting::NoRoutingFound)
    } else {
      Ok(best)
    }
  }
}

/// Run the forward CSA for all the queries in one scan of the connections.
///
/// # Arguments
/// * `connections` - The connections of the scenario
/// * `transfers` - The transferable nodes of all nodes
/// * `parameters` - The parameters shared by all queries
/// * `queries` - The queries
///
/// Returns one result per query, in the same order: the best arrival time
/// and egress node, or the reason why there is no routing.
pub fn calculate(
  connections: &Connections,
  transfers: &Transfers,
  parameters: &Parameters,
  queries: &[Query],
) -> Vec<Result<Arrival, NoRouting>> {
  let nodes_count = transfers.offsets.len() - 1;
  let mut state = BatchState::new(nodes_count, connections, parameters, queries);
  state.scan(connections, transfers, parameters, queries);
  queries
    .iter()
    .enumerate()
    .map(|(q, query)| state.result(parameters, query, q))
    .collect()
}
