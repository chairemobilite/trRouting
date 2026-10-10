#ifndef BATCHED_CSA_H
#define BATCHED_CSA_H

/* C interface of the batched_csa Rust crate, matching src/ffi.rs and the
   #[repr(C)] structs of src/forward.rs. Keep both in sync. */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BATCHED_CSA_CAN_BOARD 1u
#define BATCHED_CSA_CAN_UNBOARD 2u

/* Forward connections sorted by departure time, one array per field */
typedef struct {
  const int32_t *departure_node;
  const int32_t *arrival_node;
  const int32_t *departure_time;
  const int32_t *arrival_time;
  const int32_t *trip;
  const int32_t *min_waiting_time; /* -1 to use the default of the parameters */
  const uint32_t *flags; /* Combination of BATCHED_CSA_CAN_BOARD and BATCHED_CSA_CAN_UNBOARD */
  size_t connections_count;
  const int32_t *trip_line; /* Line uid of each trip */
  size_t trips_count;
} BatchedCsaConnections;

/* Transferable nodes in compressed sparse row form indexed by node uid */
typedef struct {
  const int32_t *offsets; /* nodes_count + 1 entries */
  size_t nodes_count;
  const int32_t *node;
  const int32_t *time;
  size_t transfers_count;
} BatchedCsaTransfers;

/* Parameters shared by all the queries of a batch, in seconds */
typedef struct {
  int32_t min_waiting_time;
  int32_t max_total_travel_time;
  int32_t max_transfer_walking_time;
  int32_t max_inner_time_of_trip_buffer;
  float walking_speed_factor;
} BatchedCsaParameters;

/* Walking time in seconds to a node, before the walking speed factor */
typedef struct {
  int32_t node;
  int32_t time;
} BatchedCsaFootpath;

/* The footpaths and excluded lines of query q are at [offsets[q], offsets[q + 1])
   of their array. Offsets have queries_count + 1 entries. */
typedef struct {
  size_t queries_count;
  const int32_t *departure_time;
  const int32_t *access_offsets;
  const BatchedCsaFootpath *access;
  const int32_t *egress_offsets;
  const BatchedCsaFootpath *egress;
  const int32_t *excluded_lines_offsets;
  const int32_t *excluded_lines;
} BatchedCsaQueries;

typedef struct {
  int32_t no_routing_reason; /* -1 when there is an arrival, otherwise the NoRoutingReason */
  int32_t arrival_time;
  int32_t egress_node;
} BatchedCsaForwardResult;

/* Forward CSA of all the queries in one scan of the connections. Writes
   queries->queries_count results, in the order of the queries. */
void batched_csa_forward_calculate(
  const BatchedCsaConnections *connections,
  const BatchedCsaTransfers *transfers,
  const BatchedCsaParameters *parameters,
  const BatchedCsaQueries *queries,
  BatchedCsaForwardResult *results
);

#ifdef __cplusplus
}
#endif

#endif /* BATCHED_CSA_H */
