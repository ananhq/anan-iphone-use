// SPDX-License-Identifier: Apache-2.0
#include "reset_trace.h"
#include "esp_attr.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#define TRACE_MAGIC 0x41505231u
typedef struct {
    uint32_t magic, stage, error, free_internal, largest_internal, stack_free, check;
} trace_record;
static RTC_NOINIT_ATTR volatile trace_record retained_record;
static reset_trace_info previous;
static portMUX_TYPE trace_lock = portMUX_INITIALIZER_UNLOCKED;
static uint32_t checksum(const trace_record *r) {
    const uint32_t fields[] = {r->magic,r->stage,r->error,r->free_internal,r->largest_internal,r->stack_free};
    uint32_t h=2166136261u;
    for(unsigned i=0;i<sizeof(fields)/sizeof(*fields);i++)
        for(unsigned shift=0;shift<32;shift+=8)h=(h^((fields[i]>>shift)&255u))*16777619u;
    return h;
}
void reset_trace_init(void) {
    int reason=esp_reset_reason();
    bool retain=reason==ESP_RST_SW||reason==ESP_RST_PANIC||reason==ESP_RST_INT_WDT||
                reason==ESP_RST_TASK_WDT||reason==ESP_RST_WDT;
    trace_record saved=retained_record;
    previous=(reset_trace_info){0};
    if(retain&&saved.magic==TRACE_MAGIC&&saved.stage<=7&&saved.check==checksum(&saved))
        previous=(reset_trace_info){true,saved.stage,(int32_t)saved.error,
                                   saved.free_internal,saved.largest_internal,saved.stack_free};
    // Consume once; a boot with no AP activity must not revive an older event.
    retained_record.magic=0;
}
void reset_trace_record(int stage,int error) {
    const unsigned caps=MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT;
    trace_record next={.magic=TRACE_MAGIC,.stage=(uint32_t)stage,.error=(uint32_t)error,
        .free_internal=heap_caps_get_free_size(caps),
        .largest_internal=heap_caps_get_largest_free_block(caps),
        .stack_free=uxTaskGetStackHighWaterMark(NULL)};
    next.check=checksum(&next);
    // Heap/task queries happen outside the critical section. Commit magic last.
    portENTER_CRITICAL(&trace_lock);
    retained_record.magic=0;
    retained_record.stage=next.stage;retained_record.error=next.error;
    retained_record.free_internal=next.free_internal;retained_record.largest_internal=next.largest_internal;
    retained_record.stack_free=next.stack_free;retained_record.check=next.check;
    retained_record.magic=TRACE_MAGIC;
    portEXIT_CRITICAL(&trace_lock);
}
reset_trace_info reset_trace_previous(void) {return previous;}
