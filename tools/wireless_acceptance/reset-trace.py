#!/usr/bin/env python3
"""Reset-record acceptance defined before implementation; fake RTC/SDK only."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from firmware_paths import include_flags
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
stub = r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "esp_system.h"
static int reason;
static unsigned free_bytes=2396,largest_bytes=512,stack_bytes=768;
int esp_reset_reason(void){return reason;}
unsigned heap_caps_get_free_size(unsigned caps){assert(caps==3);return free_bytes;}
unsigned heap_caps_get_largest_free_block(unsigned caps){assert(caps==3);return largest_bytes;}
unsigned uxTaskGetStackHighWaterMark(void *task){assert(!task);return stack_bytes;}
#include "reset_trace.c"
static void expect_empty(void){reset_trace_info p=reset_trace_previous();assert(!p.valid);}
int main(void){
 reason=ESP_RST_POWERON;reset_trace_init();expect_empty();
 reset_trace_record(4,0x101);
 // Ordinary navigation does not change the boot-latched previous record.
 expect_empty();
 int retained[]={ESP_RST_SW,ESP_RST_PANIC,ESP_RST_INT_WDT,ESP_RST_TASK_WDT,ESP_RST_WDT};
 for(unsigned i=0;i<sizeof(retained)/sizeof(*retained);i++){
  reset_trace_record(4,0x101);reason=retained[i];reset_trace_init();
  reset_trace_info p=reset_trace_previous();
  assert(p.valid&&p.stage==4&&p.error==0x101&&p.free_internal==2396&&p.largest_internal==512&&p.stack_free==768);
 }
 // A later boot with no new AP record must not resurrect the older event.
 reason=ESP_RST_PANIC;reset_trace_init();expect_empty();
 int discarded[]={ESP_RST_UNKNOWN,ESP_RST_POWERON,ESP_RST_EXT,ESP_RST_BROWNOUT,ESP_RST_DEEPSLEEP};
 for(unsigned i=0;i<sizeof(discarded)/sizeof(*discarded);i++){
  reset_trace_record(4,0x101);reason=discarded[i];reset_trace_init();expect_empty();
 }
 reset_trace_record(7,0);reason=ESP_RST_SW;reset_trace_init();assert(reset_trace_previous().stage==7);
 // Change one retained byte without updating its checksum, simulating a torn write.
 reset_trace_record(4,0x101);unsigned char *p=(unsigned char*)&retained_record;p[8]^=1;
 reason=ESP_RST_PANIC;reset_trace_init();expect_empty();
 puts("RTC AP trace: software/panic/watchdog retention, cold/brownout/torn-record rejection, boot latch and scalar heap/stack values passed");
}
'''
with tempfile.TemporaryDirectory(prefix='anan-reset-trace-') as work:
    work=Path(work);(work/'freertos').mkdir()
    (work/'esp_attr.h').write_text('#define RTC_NOINIT_ATTR\n')
    (work/'esp_system.h').write_text('#pragma once\nenum {ESP_RST_UNKNOWN,ESP_RST_POWERON,ESP_RST_EXT,ESP_RST_SW,ESP_RST_PANIC,ESP_RST_INT_WDT,ESP_RST_TASK_WDT,ESP_RST_WDT,ESP_RST_DEEPSLEEP,ESP_RST_BROWNOUT};\nint esp_reset_reason(void);\n')
    (work/'esp_heap_caps.h').write_text('#define MALLOC_CAP_INTERNAL 1u\n#define MALLOC_CAP_8BIT 2u\nunsigned heap_caps_get_free_size(unsigned);\nunsigned heap_caps_get_largest_free_block(unsigned);\n')
    (work/'freertos/FreeRTOS.h').write_text('#include <stddef.h>\ntypedef int portMUX_TYPE;\n#define portMUX_INITIALIZER_UNLOCKED 0\n#define portENTER_CRITICAL(p) ((void)(p))\n#define portEXIT_CRITICAL(p) ((void)(p))\n')
    (work/'freertos/task.h').write_text('unsigned uxTaskGetStackHighWaterMark(void*);\n')
    c=work/'fixture.c';c.write_text(stub);exe=work/'fixture'
    subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-I',str(work),*include_flags(),str(c),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
