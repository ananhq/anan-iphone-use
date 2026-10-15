#!/usr/bin/env python3
"""Predefined startup diagnostics: actual functions with a fake SDK, no device."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from firmware_paths import include_flags
import subprocess,tempfile
root=Path(__file__).resolve().parents[2]
def function(source,signature):
    start=source.index(signature);brace=source.index('{',start);depth=1;end=brace+1
    while depth:
        depth+=(source[end]=='{')-(source[end]=='}');end+=1
    return source[start:end]+'\n'
wireless=(root/'main/network/wireless.c').read_text();mcp=(root/'main/network/mcp.c').read_text()
ap_stub=r'''
#include "wireless.h"
#include <assert.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
static wireless_state state;
static atomic_bool ready,ap_requested;
static bool started;
static uint32_t ap_deadline;
static char ap_password[13];
static int state_lock,fail_at,calls,stops,releases,notifications;
#define portENTER_CRITICAL(x) ((void)(x))
#define portEXIT_CRITICAL(x) ((void)(x))
#define ESP_IP4TOADDR(a,b,c,d) (((uint32_t)(a)<<24)|((b)<<16)|((c)<<8)|(d))
#define WIFI_MODE_STA 1
#define WIFI_MODE_APSTA 2
#define WIFI_IF_AP 1
#define WIFI_AUTH_WPA2_PSK 3
#define INPUT_BLE 1
typedef struct {struct {char ssid[33],password[65];int ssid_len,channel,max_connection,authmode;} ap;} wifi_config_t;
static uint32_t now_ms(void){return 1234;}
wireless_state wireless_snapshot(void){return state;}
static void device_stop(void){stops++;}
static void bridge_release(void){assert(state.setup_stage==2);releases++;}
static void ble_input_enable(bool enable){if(!enable&&!state.setup_error)assert(state.setup_stage==3);}
static int bridge_input_mode(void){return 0;}
static void esp_fill_random(void *out,size_t n){memset(out,9,n);}
static void mbedtls_platform_zeroize(void *out,size_t n){memset(out,0,n);}
static int api(void){return ++calls==fail_at?ESP_ERR_NO_MEM:ESP_OK;}
static int esp_wifi_set_mode(int mode){if(mode==WIFI_MODE_APSTA)assert(state.setup_stage==4);return api();}
static int esp_wifi_set_config(int iface,const wifi_config_t *c){
 assert(iface==WIFI_IF_AP&&state.setup_stage==5);
 assert(!strcmp(c->ap.ssid,"StopWatch-setup")&&strlen(c->ap.password)==12);
 assert(c->ap.authmode==WIFI_AUTH_WPA2_PSK&&c->ap.max_connection==1);return api();
}
static int esp_wifi_start(void){assert(state.setup_stage==6);return api();}
static void mcp_network_lost(void){notifications++;}
static int traced_stage,traced_error;
static void reset_trace_record(int stage,int error){traced_stage=stage;traced_error=error;}
'''
ap_cases=r'''
int main(void){
 assert(wireless_setup_start()==ESP_ERR_INVALID_STATE);
 assert(state.setup_error==ESP_ERR_INVALID_STATE&&!atomic_load(&ap_requested));
 atomic_store(&ready,true);assert(wireless_setup_start()==ESP_OK);
 assert(atomic_load(&ap_requested)&&state.setup_stage==1&&state.setup_error==0&&!state.setup_ap);
 state.setup_ap=true;state.setup_stage=WIFI_SETUP_READY;atomic_store(&ap_requested,false);
 assert(wireless_setup_start()==ESP_OK&&atomic_load(&ap_requested)&&state.setup_stage==WIFI_SETUP_READY);
 for(int failure=1;failure<=3;failure++){
  memset(&state,0,sizeof(state));calls=0;started=false;fail_at=failure;
  assert(start_ap()==ESP_ERR_NO_MEM&&!state.setup_ap);
  assert((int)state.setup_stage==failure+3&&state.setup_error==ESP_ERR_NO_MEM);
  assert(traced_stage==failure+3&&traced_error==ESP_ERR_NO_MEM);
 }
 memset(&state,0,sizeof(state));calls=0;started=false;fail_at=0;
 assert(start_ap()==ESP_OK&&state.setup_ap&&state.setup_stage==7&&state.setup_error==0);
 assert(calls==3&&started&&ap_deadline==601234&&notifications==1&&stops==4&&releases==4);
 memset(&state,0,sizeof(state));calls=0;started=true;
 assert(start_ap()==ESP_OK&&calls==2&&state.setup_stage==7);
 puts("Actual AP queue/stages, per-API failures/rollback, credentials and success: passed");
}
'''
http_stub=r'''
#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include "esp_err.h"
#define pdPASS 1
#define configASSERT(x) assert(x)
#define HTTP_GET 1
#define HTTP_POST 2
#define HTTP_DELETE 3
typedef int httpd_req_t,httpd_method_t;
typedef void *httpd_handle_t;
typedef struct {int server_port,ctrl_port,stack_size,max_open_sockets,max_uri_handlers,lru_purge_enable,recv_wait_timeout,send_wait_timeout;} httpd_config_t;
typedef struct {const char *uri;httpd_method_t method;esp_err_t (*handler)(httpd_req_t*);void *user_ctx;} httpd_uri_t;
#define HTTPD_DEFAULT_CONFIG() ((httpd_config_t){0})
typedef int identity_job;
static int lock,identity_jobs,start_result,fail_route,route_count,stopped,worker_count;
static atomic_bool http_running,https_running,identity_busy;
static atomic_int http_start_error;
static httpd_handle_t http_server,https_server=(void*)2;
static const bool secure_route=true;
static int xSemaphoreCreateMutex(void){return 1;}
static int xQueueCreate(int n,size_t size){(void)n;(void)size;return 1;}
static void identity_init(void){}
static bool identity_ready(void){return true;}
static bool start_https(void){atomic_store(&https_running,true);return true;}
static void identity_worker(void *ctx){(void)ctx;}
static int xTaskCreate(void(*fn)(void*),const char *name,int size,void *ctx,int priority,void *handle){
 (void)fn;(void)name;(void)size;(void)ctx;(void)priority;(void)handle;worker_count++;return pdPASS;
}
static int httpd_start(httpd_handle_t *out,const httpd_config_t *c){
 assert(c->server_port==80&&c->max_open_sockets==4&&c->max_uri_handlers==15);
 if(!start_result)*out=(void*)1;
 return start_result;
}
static int httpd_stop(httpd_handle_t server){assert(server==(void*)1);stopped++;return 0;}
static int httpd_register_uri_handler(httpd_handle_t server,const httpd_uri_t *route){
 assert(server==(void*)1&&route->uri&&route->handler);return ++route_count==fail_route?ESP_ERR_NO_MEM:ESP_OK;
}
#define HANDLER(n) static int n(httpd_req_t *r){(void)r;return 0;}
HANDLER(page_handler) HANDLER(diagnostic_handler) HANDLER(enrollment_handler)
HANDLER(profile_handler) HANDLER(config_handler) HANDLER(wifi_config_handler)
HANDLER(wifi_networks_handler) HANDLER(wifi_scan_handler) HANDLER(mcp_handler) HANDLER(source_handler)
'''
http_cases=r'''
static void reset(void){http_running=false;https_running=false;http_start_error=0;route_count=stopped=0;http_server=NULL;}
int main(void){
 reset();start_result=ESP_ERR_NO_MEM;mcp_init();
 assert(!mcp_http_ready()&&mcp_http_error()==ESP_ERR_NO_MEM&&mcp_ready()&&worker_count==1&&route_count==0);
 for(int failure=1;failure<=15;failure++){
  reset();start_result=0;fail_route=failure;mcp_init();
  assert(!mcp_http_ready()&&mcp_http_error()==ESP_ERR_NO_MEM&&mcp_ready()&&stopped==1&&http_server==NULL);
 }
 reset();fail_route=0;mcp_init();assert(mcp_http_ready()&&mcp_http_error()==0&&route_count==15&&stopped==0);
 puts("Actual HTTP startup and every route failure visible independently of HTTPS: passed");
}
'''
with tempfile.TemporaryDirectory(prefix='anan-startup-') as d:
    d=Path(d)
    (d/'esp_err.h').write_text('typedef int esp_err_t;\n#define ESP_OK 0\n#define ESP_ERR_NO_MEM 0x101\n#define ESP_ERR_INVALID_STATE 0x103\n')
    ap=ap_stub+''.join(function(wireless,s) for s in ('static void setup_progress(', 'static esp_err_t start_ap(', 'esp_err_t wireless_setup_start('))+ap_cases
    http=http_stub+''.join(function(mcp,s) for s in ('bool mcp_ready(', 'bool mcp_http_ready(', 'int mcp_http_error(', 'static bool register_uri(', 'void mcp_init('))+http_cases
    for name,code in [('ap',ap),('http',http)]:
        c=d/f'{name}.c';c.write_text(code);exe=d/name
        subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-I',str(d),*include_flags(),str(c),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
