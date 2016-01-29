#pragma once

#include <gflags/gflags.h>
#include <cstdint>


DECLARE_int32(communication_factor);
DECLARE_int32(virtual_staleness);
DECLARE_bool(is_bipartite);
DECLARE_bool(is_local_sync);
DECLARE_int32(num_vocabs);
DECLARE_int32(num_clocks_per_work_unit);
