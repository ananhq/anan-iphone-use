// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef struct {
    bool valid;
    uint32_t stage;
    int32_t error;
    uint32_t free_internal, largest_internal, stack_free;
} reset_trace_info;
// App-start capture only. No NVS, secrets, control state or automatic recovery.
void reset_trace_init(void);
void reset_trace_record(int stage, int error);
reset_trace_info reset_trace_previous(void);
