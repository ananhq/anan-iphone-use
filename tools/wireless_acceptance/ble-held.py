#!/usr/bin/env python3
"""Predefined actual-sender host fixture; no NimBLE, radio or device access."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from firmware_paths import include_flags
import os
import re
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
source = (root / 'main/input/ble_input.c').read_text()
usb = (root / 'main/input/usb.c').read_text()
ble_map = source.split('static const uint8_t report_map[] = {', 1)[1].split('};', 1)[0]
usb_map = usb.split('static const uint8_t reports[] = {', 1)[1].split('};', 1)[0]
assert re.sub(r'\s', '', ble_map).rstrip(',') == re.sub(r'\s', '', usb_map).rstrip(','), 'BLE map must match USB'
assert '#define BLE_INPUT_REPORT_COUNT 3' in source
assert 'INPUT_REPORT(3)' not in source and 'REFERENCE(4)' not in source
assert '.descriptors=reference_3' in source
assert 'index == BLE_INPUT_REPORT_COUNT ? 2' in source



def function(name):
    start = source.index(name)
    brace = source.index('{', start)
    depth = 1
    end = brace + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]


preamble = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdatomic.h>
#include "ble_reports.h"
static atomic_uint link_generation, subscribed;
static atomic_bool initialized=true, enabled=true, connected=true, encrypted=true, suspended=false;
static unsigned report_generation;
static bool consumer_held, online=true;
static uint8_t last_reports[3][8];
static int report_lock __attribute__((unused)), fail_index=-1;
static unsigned count, keys;
static struct { unsigned index; uint8_t bytes[8]; } events[128];
#define portENTER_CRITICAL(lock) ((void)(lock))
#define portEXIT_CRITICAL(lock) ((void)(lock))
static bool send_report(unsigned index,uint8_t *data,size_t size) {
    if (!online) return false;
    assert(count<128 && size<=8 && index<3);
    events[count].index=index;
    memcpy(events[count++].bytes,data,size);
    if ((int)index==fail_index) { fail_index=-1; return false; }
    memcpy(last_reports[index],data,size);
    return true;
}
static bool ble_key(uint8_t m,uint8_t k) { assert(!m && !k); ++keys; return online; }
static bool ble_consumer(uint16_t u) { assert(!u); return online; }
'''
checks = r'''
static void move(uint8_t b,uint16_t x,uint16_t y) {
    unsigned start=count;
    assert(ble_mouse(b,x,y));
    assert(count==start+1 && events[start].index==0);
    assert(events[start].bytes[0]==b);
    assert(events[start].bytes[1]==((x>>1)&255));
    assert(events[start].bytes[2]==(x>>9));
    assert(events[start].bytes[3]==((y>>1)&255));
    assert(events[start].bytes[4]==(y>>9));
    assert(!events[start].bytes[5] && !events[start].bytes[6]);
}
int main(void) {
    atomic_store(&subscribed,3); assert(ble_input_online());
    atomic_store(&subscribed,1); assert(!ble_input_online());
    atomic_store(&subscribed,2); assert(!ble_input_online());
    atomic_store(&subscribed,7); assert(ble_input_online());
    atomic_store(&encrypted,false); assert(!ble_input_online());
    atomic_store(&encrypted,true);
    for(unsigned stroke=0;stroke<3;++stroke) {
        move(0,1000,2000); move(1,1000,2000);
        move(1,3000,4000); move(1,5000,6000); move(0,5000,6000);
        unsigned start=count;
        ble_release();
        assert(count==start+1 && events[start].index==0 && !events[start].bytes[0]);
    }
    move(1,7000,8000);
    unsigned start=count;
    ble_release();
    assert(count==start+1 && events[start].index==0 && !events[start].bytes[0]);
    assert(events[start].bytes[1]==((7000>>1)&255));
    assert(events[start].bytes[3]==((8000>>1)&255));
    move(1,7000,8000);
    fail_index=0; start=count;
    assert(!ble_mouse(1,9000,10000) && count==start+1);
    ble_release();
    assert(count==start+2 && !last_reports[0][0]);
    assert(events[start+1].bytes[1]==((7000>>1)&255));
    start=count;
    assert(ble_scroll(21000,22000,-127,127));
    assert(count==start+1 && events[start].index==0 && !events[start].bytes[0]);
    assert(events[start].bytes[1]==((21000>>1)&255));
    assert(events[start].bytes[3]==((22000>>1)&255));
    assert(events[start].bytes[5]==129 && events[start].bytes[6]==127);
    start=count; ble_release();
    assert(count==start+1 && !events[start].bytes[5] && !events[start].bytes[6]);
    start=count; online=false;
    assert(!ble_mouse(1,13000,14000)); ble_release();
    assert(count==start);
    online=true; memset(last_reports,0,sizeof(last_reports));
    atomic_fetch_add(&link_generation,1);
    move(0,15000,16000);
    assert(!last_reports[0][0] && keys>=6);
    puts("Single BLE mouse map/readiness, three strokes, wheel, release and no-replay passed (offline).");
}
'''
with tempfile.TemporaryDirectory(prefix='stopwatch-ble-held-') as tmp:
    path = Path(tmp)
    fixture = path / 'held.c'
    fixture.write_text(preamble + function('bool ble_input_ready(') + '\n' +
                       function('bool ble_input_online(') + '\n' +
                       function('bool ble_mouse(') + '\n' +
                       function('bool ble_scroll(') + '\n' +
                       function('void ble_release(') + checks)
    executable = path / 'held'
    subprocess.run([os.environ.get('CC', 'cc'), '-std=c11', '-Wall', '-Wextra', '-Werror',
                    *include_flags(), str(fixture),
                    str(root / 'main/input/ble_reports.c'), '-o', str(executable)], check=True)
    subprocess.run([str(executable)], check=True)
