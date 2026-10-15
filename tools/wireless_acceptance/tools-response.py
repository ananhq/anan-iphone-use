#!/usr/bin/env python3
"""Predefined tools response allocation/byte-equivalence fixtures; no network."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from firmware_paths import include_flags
import os,subprocess,tempfile
root=Path(__file__).resolve().parents[2]
idf=Path(os.environ['IDF_PATH'])
source=(root/'main/network/mcp.c').read_text()
def function(signature):
 start=source.index(signature);i=source.index('{',start)+1;depth=1
 # Braces in quoted JSON strings must not count as C block boundaries.
 quote=False;escaped=False
 while depth:
  c=source[i]
  if quote:
   if escaped:escaped=False
   elif c=='\\':escaped=True
   elif c=='"':quote=False
  elif c=='"':quote=True
  else:depth+=(c=='{')-(c=='}')
  i+=1
 return source[start:i]+'\n'
stub=r'''
#include "cJSON.h"
#include "mcp_tools.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
typedef int esp_err_t,httpd_req_t;
#define ESP_OK 0
#define ESP_FAIL -1
#define ESP_ERR_NO_MEM 0x101
#define HTTPD_RESP_USE_STRLEN -1
typedef union {size_t n;max_align_t align;} Block;
static size_t live,peak,limit=SIZE_MAX,sent;
static int send_result;
static char wire[8192];
static void *tracked(size_t n){
 if(n>limit-live)return NULL;
 Block *p=malloc(sizeof(Block)+n);if(!p)return NULL;
 p->n=n;live+=n;if(live>peak)peak=live;return p+1;
}
static void release(void *ptr){if(ptr){Block *p=(Block*)ptr-1;live-=p->n;free(p);}}
static void mbedtls_platform_zeroize(void *p,size_t n){memset(p,0,n);}
static int httpd_resp_set_hdr(httpd_req_t*r,const char*k,const char*v){(void)r;assert(k&&v);return 0;}
static int httpd_resp_set_status(httpd_req_t*r,const char*s){(void)r;assert(!strcmp(s,"200 OK"));return 0;}
static int httpd_resp_set_type(httpd_req_t*r,const char*s){(void)r;assert(!strcmp(s,"application/json"));return 0;}
static int httpd_resp_send(httpd_req_t*r,const char*s,long n){
 (void)r;if(n<0)n=strlen(s);assert(n<(long)sizeof(wire));
 memcpy(wire,s,n);wire[n]=0;sent=(size_t)n;return send_result;
}
#define malloc tracked
#define free release
'''
cases=r'''
static size_t original(httpd_req_t*r,const cJSON*id){
 size_t base=live;peak=live;sent=0;assert(rpc_result(r,id,cJSON_Parse(mcp_tools_json))==ESP_OK);
 assert(live==base);return peak-base;
}
int main(void){
 cJSON_Hooks h={tracked,release};cJSON_InitHooks(&h);httpd_req_t r=0;
 char controls[65];memset(controls,1,64);controls[64]=0;
 cJSON *ids[]={cJSON_CreateNumber(3),cJSON_CreateNumber(-9007199254740991.),
  cJSON_CreateString("quoted\"\\id"),cJSON_CreateString("中文ID"),cJSON_CreateString(controls)};
 char expected[8192];size_t old_peak=0,new_peak=0;
 for(unsigned i=0;i<sizeof(ids)/sizeof(*ids);i++){
  limit=SIZE_MAX;size_t previous=original(&r,ids[i]);if(previous>old_peak)old_peak=previous;
  strcpy(expected,wire);size_t base=live;peak=live;sent=0;limit=live+8192;
  assert(rpc_tools_result(&r,ids[i])==ESP_OK&&!strcmp(wire,expected)&&live==base);
  if(peak-base>new_peak)new_peak=peak-base;
  send_result=ESP_FAIL;assert(rpc_tools_result(&r,ids[i])==ESP_FAIL&&live==base);send_result=0;
  sent=0;limit=live;assert(rpc_tools_result(&r,ids[i])==ESP_ERR_NO_MEM&&!sent&&live==base);
  limit=live+1024;assert(rpc_tools_result(&r,ids[i])==ESP_ERR_NO_MEM&&!sent&&live==base);
 }
 assert(new_peak<8192&&old_peak>new_peak*3);limit=SIZE_MAX;
 for(unsigned i=0;i<sizeof(ids)/sizeof(*ids);i++)cJSON_Delete(ids[i]);assert(live==0);
 printf("Actual tools/list byte equivalence, numeric/escaped/UTF8 IDs, 8KiB budget, allocation/send cleanup: passed (old peak %zu, new %zu)\n",old_peak,new_peak);
}
'''
with tempfile.TemporaryDirectory(prefix='anan-tools-response-') as d:
 d=Path(d);c=d/'check.c';exe=d/'check'
 c.write_text(stub+''.join(function(s) for s in ('static void response_headers(', 'static esp_err_t send_json(', 'static cJSON *envelope(', 'static esp_err_t rpc_result(', 'static esp_err_t rpc_tools_result('))+cases)
 subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror',*include_flags(),'-I',str(idf/'components/json/cJSON'),str(c),str(idf/'components/json/cJSON/cJSON.c'),'-lm','-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
assert 'rpc_result(r, id, cJSON_Parse(mcp_tools_json))' not in source
