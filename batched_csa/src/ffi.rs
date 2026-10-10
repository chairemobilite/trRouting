//! C interface of the crate, declared in include/batched_csa.h. Arrays are
//! passed as pointers with their length.

use crate::forward::{self, Connections, Footpath, Parameters, Query, Transfers};

#[repr(C)]
pub struct BatchedCsaConnections {
  pub departure_node: *const i32,
  pub arrival_node: *const i32,
  pub departure_time: *const i32,
  pub arrival_time: *const i32,
  pub trip: *const i32,
  pub min_waiting_time: *const i32,
  pub flags: *const u32,
  pub connections_count: usize,
  pub trip_line: *const i32,
  pub trips_count: usize,
}

#[repr(C)]
pub struct BatchedCsaTransfers {
  /// nodes_count + 1 entries
  pub offsets: *const i32,
  pub nodes_count: usize,
  pub node: *const i32,
  pub time: *const i32,
  pub transfers_count: usize,
}

/// The footpaths and excluded lines of query `q` are at
/// `offsets[q]..offsets[q + 1]` of their array. Offsets have
/// queries_count + 1 entries.
#[repr(C)]
pub struct BatchedCsaQueries {
  pub queries_count: usize,
  pub departure_time: *const i32,
  pub access_offsets: *const i32,
  pub access: *const Footpath,
  pub egress_offsets: *const i32,
  pub egress: *const Footpath,
  pub excluded_lines_offsets: *const i32,
  pub excluded_lines: *const i32,
}

#[repr(C)]
pub struct BatchedCsaForwardResult {
  /// -1 when there is an arrival, otherwise the NoRoutingReason
  pub no_routing_reason: i32,
  pub arrival_time: i32,
  pub egress_node: i32,
}

/// Slice from a C array, which can be null when empty
unsafe fn slice<'a, T>(data: *const T, len: usize) -> &'a [T] {
  if len == 0 {
    &[]
  } else {
    std::slice::from_raw_parts(data, len)
  }
}

/// Element `q` of a C array of offsets, as a range of the array they index
unsafe fn range(offsets: *const i32, q: usize) -> std::ops::Range<usize> {
  *offsets.add(q) as usize..*offsets.add(q + 1) as usize
}

/// Run `forward::calculate` and write one result per query in `results`.
///
/// # Safety
/// Every pointer must be valid for the lengths given in its struct, and
/// `results` for `queries.queries_count` elements.
#[no_mangle]
pub unsafe extern "C" fn batched_csa_forward_calculate(
  connections: &BatchedCsaConnections,
  transfers: &BatchedCsaTransfers,
  parameters: &Parameters,
  queries: &BatchedCsaQueries,
  results: *mut BatchedCsaForwardResult,
) {
  let count = connections.connections_count;
  let connections = Connections {
    departure_node: slice(connections.departure_node, count),
    arrival_node: slice(connections.arrival_node, count),
    departure_time: slice(connections.departure_time, count),
    arrival_time: slice(connections.arrival_time, count),
    trip: slice(connections.trip, count),
    min_waiting_time: slice(connections.min_waiting_time, count),
    flags: slice(connections.flags, count),
    trip_line: slice(connections.trip_line, connections.trips_count),
  };
  let transfers = Transfers {
    offsets: slice(transfers.offsets, transfers.nodes_count + 1),
    node: slice(transfers.node, transfers.transfers_count),
    time: slice(transfers.time, transfers.transfers_count),
  };
  let n = queries.queries_count;
  if n == 0 {
    return;
  }
  let access = slice(queries.access, *queries.access_offsets.add(n) as usize);
  let egress = slice(queries.egress, *queries.egress_offsets.add(n) as usize);
  let excluded_lines = slice(
    queries.excluded_lines,
    *queries.excluded_lines_offsets.add(n) as usize,
  );
  let departure_time = slice(queries.departure_time, n);
  let queries: Vec<Query> = (0..n)
    .map(|q| Query {
      departure_time: departure_time[q],
      access: &access[range(queries.access_offsets, q)],
      egress: &egress[range(queries.egress_offsets, q)],
      excluded_lines: &excluded_lines[range(queries.excluded_lines_offsets, q)],
    })
    .collect();

  let results = std::slice::from_raw_parts_mut(results, n);
  for (result, calculated) in results.iter_mut().zip(forward::calculate(
    &connections,
    &transfers,
    parameters,
    &queries,
  )) {
    *result = match calculated {
      Ok(arrival) => BatchedCsaForwardResult {
        no_routing_reason: -1,
        arrival_time: arrival.time,
        egress_node: arrival.egress_node,
      },
      Err(reason) => BatchedCsaForwardResult {
        no_routing_reason: reason as i32,
        arrival_time: -1,
        egress_node: -1,
      },
    };
  }
}
