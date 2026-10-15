#!/usr/bin/env python3
"""PSRAM acceptance defined before implementation; synthetic heaps, no device I/O."""
from pathlib import Path
import os
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
idf = Path(os.environ['IDF_PATH'])
expected = {
    'CONFIG_SPIRAM': 'y', 'CONFIG_SPIRAM_MODE_OCT': 'y',
    'CONFIG_SPIRAM_SPEED_80M': 'y', 'CONFIG_SPIRAM_BOOT_INIT': 'y',
    'CONFIG_SPIRAM_MEMTEST': 'y', 'CONFIG_SPIRAM_USE_MALLOC': 'y',
    'CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL': '1024',
    'CONFIG_SPIRAM_MALLOC_RESERVE_INTERNAL': '32768',
    'CONFIG_SPIRAM_TRY_ALLOCATE_WIFI_LWIP': 'y',
    'CONFIG_ESP_WIFI_STATIC_RX_BUFFER_NUM': '10',
    'CONFIG_ESP_WIFI_DYNAMIC_RX_BUFFER_NUM': '32',
    'CONFIG_ESP_WIFI_STATIC_TX_BUFFER': 'y',
    'CONFIG_ESP_WIFI_STATIC_TX_BUFFER_NUM': '16',
    'CONFIG_ESP_WIFI_CACHE_TX_BUFFER_NUM': '32',
    'CONFIG_BT_NIMBLE_MEM_ALLOC_MODE_INTERNAL': 'y',
    'CONFIG_MBEDTLS_INTERNAL_MEM_ALLOC': 'y',
}
disabled = ('CONFIG_SPIRAM_FETCH_INSTRUCTIONS', 'CONFIG_SPIRAM_RODATA',
            'CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY',
            'CONFIG_SPIRAM_ALLOW_NOINIT_SEG_EXTERNAL_MEMORY',
            'CONFIG_SPIRAM_IGNORE_NOTFOUND', 'CONFIG_FREERTOS_TASK_CREATE_ALLOW_EXT_MEM')
config_file = Path(sys.argv[1]) if len(sys.argv) > 1 else root/'sdkconfig.defaults'
config = dict(line.split('=', 1) for line in config_file.read_text().splitlines()
              if line.startswith('CONFIG_') and '=' in line)
if config.get('CONFIG_SPIRAM','n') != 'y':
    # The reset-info recovery candidate deliberately restores the non-PSRAM baseline.
    expected = {'CONFIG_IDF_TARGET':'"esp32s3"','CONFIG_BT_ENABLED':'y'}
    disabled += ('CONFIG_SPIRAM','CONFIG_SPIRAM_TRY_ALLOCATE_WIFI_LWIP')
    if config_file.name == 'sdkconfig':
        expected.update({'CONFIG_ESP_WIFI_STATIC_RX_BUFFER_NUM':'10',
                         'CONFIG_ESP_WIFI_DYNAMIC_TX_BUFFER':'y',
                         'CONFIG_ESP_WIFI_DYNAMIC_TX_BUFFER_NUM':'32',
                         'CONFIG_ESP_WIFI_RX_BA_WIN':'6','CONFIG_LWIP_TCP_OOSEQ_MAX_PBUFS':'4'})
for key, value in expected.items():
    assert config.get(key) == value, (config_file.name, key, config.get(key), value)
for key in disabled:
    assert config.get(key, 'n') == 'n', (config_file.name, key)
assert 'esp_psram' in (root/'main/CMakeLists.txt').read_text()
display = (root/'main/hardware/display.c').read_text()
assert 'MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL' in display

source = (root/'main/network/mcp.c').read_text()
start = source.index('static void add_memory_diagnostics(')
end = source.index('\n}', start) + 2
body = source[start:end]
assert 'add_memory_diagnostics(data);' in source
stub = r'''
#include "cJSON.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#define MALLOC_CAP_8BIT 1u
#define MALLOC_CAP_INTERNAL 2u
#define MALLOC_CAP_SPIRAM 4u
static bool available;
static size_t esp_get_free_heap_size(void){return available?8123456:2396;}
static size_t esp_get_minimum_free_heap_size(void){return available?8000000:1084;}
#if CONFIG_SPIRAM
static bool esp_psram_is_initialized(void){return available;}
static size_t esp_psram_get_size(void){assert(available);return 8388608;}
#endif
static size_t heap_caps_get_free_size(unsigned caps){
 assert(caps==(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT)||caps==(MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));
 return (caps&MALLOC_CAP_INTERNAL)?2396:available?8121060:0;
}
static size_t heap_caps_get_minimum_free_size(unsigned caps){
 assert(caps==(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT)||caps==(MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));
 return (caps&MALLOC_CAP_INTERNAL)?1084:available?7998916:0;
}
static size_t heap_caps_get_largest_free_block(unsigned caps){
 assert(caps==(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT)||caps==(MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));
 return (caps&MALLOC_CAP_INTERNAL)?512:available?7900000:0;
}
'''
cases = r'''
static double number(cJSON *data,const char *key){
 cJSON *v=cJSON_GetObjectItemCaseSensitive(data,key);assert(cJSON_IsNumber(v));return v->valuedouble;
}
int main(void){
 for(int i=0;i<2;i++){
  available=CONFIG_SPIRAM&&i; cJSON *data=cJSON_CreateObject();add_memory_diagnostics(data);
  assert(number(data,"freeHeapBytes")==esp_get_free_heap_size());
  assert(number(data,"minimumFreeHeapBytes")==esp_get_minimum_free_heap_size());
  assert(cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(data,"psramInitialized"))==available);
  assert(number(data,"psramSizeBytes")== (available?8388608:0));
  assert(number(data,"freeInternalHeapBytes")==2396);
  assert(number(data,"minimumFreeInternalHeapBytes")==1084);
  assert(number(data,"largestFreeInternalBlockBytes")==512);
  assert(number(data,"freePsramBytes")== (available?8121060:0));
  assert(number(data,"minimumFreePsramBytes")== (available?7998916:0));
  assert(number(data,"largestFreePsramBlockBytes")== (available?7900000:0));
  assert(cJSON_GetArraySize(data)==10);cJSON_Delete(data);
 }
 puts("Actual diagnostic helper: PSRAM availability, capacity and separate heap/fragmentation counters passed");
}
'''
with tempfile.TemporaryDirectory(prefix='anan-memory-') as work:
    work = Path(work)
    c = work/'memory.c'
    c.write_text(stub+body+cases)
    exe = work/'memory'
    for enabled in (0,1):
        subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror', '-DCONFIG_SPIRAM='+str(enabled),
                    '-I', str(idf/'components/json/cJSON'), str(c),
                    str(idf/'components/json/cJSON/cJSON.c'), '-lm', '-o', str(exe)], check=True)
        subprocess.run([str(exe)], check=True)
print('Selected PSRAM or recovery baseline configuration, critical storage and both compiled diagnostic branches: passed')
