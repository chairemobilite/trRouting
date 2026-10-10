//! Batched connection scan algorithm (CSA): the connections are scanned once
//! for many queries, on the CPU and later on the GPU.

pub mod ffi;
pub mod forward;
