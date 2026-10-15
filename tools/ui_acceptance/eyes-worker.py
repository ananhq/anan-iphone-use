#!/usr/bin/env python3
"""Predefined actual-worker notification check. Synthetic queue/time/reports only."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from firmware_paths import include_flags
import subprocess
import tempfile
root=Path(__file__).resolve().parents[2]
fixture=r'''
#include "actions.h"
#include "eyes.h"
#include <setjmp.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
unsigned fake_ms;
static jmp_buf complete;
static unsigned reports,visible_reports;
static void observe(void) {
    eyes_pose p;eyes_sample(fake_ms,&p);
    reports++;
    if(p.eye[0].x!=161||p.eye[0].ry!=72||p.eye[1].ry!=72)visible_reports++;
}
bool bridge_mouse(uint8_t b,uint16_t x,uint16_t y){(void)b;eyes_pointer(x,y);observe();return true;}
bool bridge_key(uint8_t m,uint8_t k){(void)m;(void)k;observe();return true;}
bool bridge_consumer(uint16_t u){(void)u;observe();return true;}
bool bridge_scroll(uint16_t x,uint16_t y,int8_t v,int8_t h){(void)x;(void)y;(void)v;(void)h;observe();return true;}
bool bridge_input_online(void){return true;}
void bridge_release(void){}
#include "actions.c"
static queued_action queued;
int host_send(void *q,const void *data,unsigned timeout){(void)q;(void)timeout;memcpy(&queued,data,sizeof(queued));return 1;}
int host_receive(void *q,void *data,unsigned timeout){(void)q;(void)timeout;memcpy(data,&queued,sizeof(queued));return 1;}
static bool guard(uint32_t e,void*c){(void)e;(void)c;return true;}
static void done(const action*a,bool ok,void*c){(void)a;(void)c;assert(ok&&!actions_busy());longjmp(complete,1);}
int main(void) {
    actions_init();
    for(int i=0;i<3;i++) {
        fake_ms=0;reports=visible_reports=0;eyes_reset(0);
        action a={.kind=i==0?ACTION_TAP:i==1?ACTION_TYPE:ACTION_KEYBOARD,
            .x=0,.y=0,.text={'a'},.text_length=1,.key=43,.repeat=1,.press_ms=30};
        assert(actions_reserve()&&actions_enqueue(&a,guard,done,NULL));
        if(!setjmp(complete))action_worker(NULL);
        assert(reports&&visible_reports&&fake_ms==(i==0?70u:30u));
    }
    puts("Actual queued worker publishes action feedback through completion; tap/type/key times remain 70/30/30ms: passed");
}
'''
with tempfile.TemporaryDirectory(prefix='anan-eyes-worker-') as work:
    work=Path(work);(work/'freertos').mkdir()
    for name in ['FreeRTOS.h','task.h']:
        (work/'freertos'/name).write_text((root/'tools/hid_acceptance/freertos'/name).read_text())
    (work/'esp_timer.h').write_text((root/'tools/hid_acceptance/esp_timer.h').read_text())
    (work/'freertos/queue.h').write_text('''#pragma once
#include <stddef.h>
typedef void *QueueHandle_t;
int host_send(void*,const void*,unsigned);
int host_receive(void*,void*,unsigned);
static inline void *xQueueCreate(unsigned n,size_t s){(void)n;(void)s;return (void*)1;}
#define xQueueSend host_send
#define xQueueReceive host_receive
''')
    (work/'fixture.c').write_text(fixture)
    exe=work/'worker'
    subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-I',str(work),*include_flags(),str(work/'fixture.c'),str(root/'main/ui/eyes.c'),'-lm','-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
