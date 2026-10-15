// SPDX-License-Identifier: Apache-2.0
#include "discovery.h"
#include "lwip/apps/mdns.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdatomic.h>
static atomic_bool conflict;
bool discovery_conflict(void) {return atomic_load(&conflict);}
static void name_result(struct netif *netif,u8_t result,s8_t service) {
 (void)netif;(void)service;
 if(result==MDNS_PROBING_CONFLICT)atomic_store(&conflict,true);
}
static void refresh(void *unused) {
 (void)unused;
 struct netif *net;
 NETIF_FOREACH(net) {
  if(!netif_is_up(net)||!netif_is_link_up(net)||ip4_addr_isany_val(*netif_ip4_addr(net))||!(net->flags&NETIF_FLAG_IGMP))continue;
  if(!mdns_resp_netif_active(net)) {
   if(mdns_resp_add_netif(net,"stopwatch")==ERR_OK)
    mdns_resp_add_service(net,"StopWatch MCP","_http",DNSSD_PROTO_TCP,80,NULL,NULL);
  }
 }
}
static void initialize(void *unused) {
 (void)unused;mdns_resp_init();mdns_resp_register_name_result_cb(name_result);refresh(NULL);
}
static void worker(void *unused) {
 (void)unused;
 for(;;){tcpip_callback(refresh,NULL);vTaskDelay(pdMS_TO_TICKS(2000));}
}
void discovery_init(void) {
 configASSERT(tcpip_callback(initialize,NULL)==ERR_OK);
 configASSERT(xTaskCreate(worker,"mdns-watch",2048,NULL,1,NULL)==pdPASS);
}
