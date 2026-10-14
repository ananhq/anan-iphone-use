#pragma once
#include <stdint.h>
extern unsigned fake_ms;
static inline int64_t esp_timer_get_time(void){return (int64_t)fake_ms*1000;}
