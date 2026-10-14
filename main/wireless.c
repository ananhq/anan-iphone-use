// SPDX-License-Identifier: Apache-2.0
#include "wireless.h"
#include "mcp.h"
#include "device.h"
#include "bridge.h"
#include "input_backends.h"
#include "network_store.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "mbedtls/platform_util.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
static network_store stored;
static wireless_state state;
static portMUX_TYPE state_lock=portMUX_INITIALIZER_UNLOCKED;
static SemaphoreHandle_t config_lock;
static esp_netif_t *station,*access_point;
static atomic_bool ready,scan_busy,restart_requested,ap_requested;
static atomic_uint scan_driver_status;
static bool started,connecting;
static unsigned selected;
static uint32_t connect_deadline,ap_deadline;
static char ap_password[13];
static EventGroupHandle_t scan_events;
static wireless_scan_result scan_result;
#define SCAN_FINISHED BIT0
#define STA_DISCONNECTED BIT1
static uint32_t now_ms(void){return esp_timer_get_time()/1000;}
wireless_state wireless_snapshot(void){
 portENTER_CRITICAL(&state_lock);wireless_state copy=state;portEXIT_CRITICAL(&state_lock);return copy;
}
void wireless_address(char *out,size_t size){
 wireless_state s=wireless_snapshot();if(!s.connected){if(size)out[0]=0;return;}
 esp_ip4_addr_t ip={.addr=s.address};char buffer[16];esp_ip4addr_ntoa(&ip,buffer,sizeof(buffer));snprintf(out,size,"%s",buffer);
}
static void set_error(int error){portENTER_CRITICAL(&state_lock);state.error=error;portEXIT_CRITICAL(&state_lock);}
static void lost(void){
 portENTER_CRITICAL(&state_lock);state.connected=false;state.address=0;state.netmask=0;++state.generation;portEXIT_CRITICAL(&state_lock);
 mcp_network_lost();
}
static void event(void *arg,esp_event_base_t base,int32_t id,void *data){
 (void)arg;
 if(base==WIFI_EVENT&&id==WIFI_EVENT_SCAN_DONE){
  if(atomic_load(&scan_busy)){atomic_store(&scan_driver_status,((wifi_event_sta_scan_done_t *)data)->status);xEventGroupSetBits(scan_events,SCAN_FINISHED);}
 }else if(base==IP_EVENT&&id==IP_EVENT_STA_GOT_IP){
  const ip_event_got_ip_t *got=data;
  uint32_t subnet=got->ip_info.ip.addr&got->ip_info.netmask.addr;
  // Reserve both local configuration subnets, including before a future AP opens.
  if(subnet==(ESP_IP4TOADDR(192,168,7,1)&got->ip_info.netmask.addr)||subnet==(ESP_IP4TOADDR(192,168,4,1)&got->ip_info.netmask.addr)){
   lost();set_error(ESP_ERR_INVALID_STATE);xEventGroupSetBits(scan_events,STA_DISCONNECTED);return;
  }
  portENTER_CRITICAL(&state_lock);state.address=got->ip_info.ip.addr;state.netmask=got->ip_info.netmask.addr;
  state.connected=true;state.error=0;++state.generation;portEXIT_CRITICAL(&state_lock);
 }else if(base==IP_EVENT&&id==IP_EVENT_STA_LOST_IP)lost();
 else if(base==WIFI_EVENT&&id==WIFI_EVENT_STA_DISCONNECTED){lost();set_error(((wifi_event_sta_disconnected_t *)data)->reason);xEventGroupSetBits(scan_events,STA_DISCONNECTED);}
 else if(base==WIFI_EVENT&&id==WIFI_EVENT_STA_STOP)lost();
}
static void configured(void){portENTER_CRITICAL(&state_lock);state.configured=stored.count>0;portEXIT_CRITICAL(&state_lock);}
static esp_err_t connect_saved(void){
 if(!stored.count)return ESP_OK;
 if(selected>=stored.count)selected=0;
 network_entry *entry=&stored.entries[selected];wifi_config_t config={0};
 memcpy(config.sta.ssid,entry->ssid,strlen(entry->ssid));memcpy(config.sta.password,entry->password,strlen(entry->password));
 config.sta.threshold.authmode=entry->password[0]?WIFI_AUTH_WPA2_PSK:WIFI_AUTH_OPEN;config.sta.pmf_cfg.capable=true;
 esp_err_t result=esp_wifi_set_config(WIFI_IF_STA,&config);mbedtls_platform_zeroize(&config,sizeof(config));
 if(result==ESP_OK&&!started){result=esp_wifi_start();started=result==ESP_OK;}
 if(result==ESP_OK)result=esp_wifi_connect();
 connecting=true;connect_deadline=now_ms()+15000;if(result!=ESP_OK)set_error(result);return result;
}
static esp_err_t save(network_store *next){
 nvs_handle_t handle;esp_err_t result=nvs_open_from_partition("mcp_kv","wireless",NVS_READWRITE,&handle);
 if(result==ESP_OK){
  result=nvs_set_blob(handle,"v2",next,sizeof(*next));
  if(result==ESP_OK){result=nvs_erase_key(handle,"v1");if(result==ESP_ERR_NVS_NOT_FOUND)result=ESP_OK;}
  if(result==ESP_OK)result=nvs_commit(handle);
  nvs_close(handle);
 }
 if(result==ESP_OK){stored=*next;configured();lost();atomic_store(&restart_requested,true);}return result;
}
static esp_err_t start_ap(void){
 device_stop();bridge_release();ble_input_enable(false);
 unsigned char random[12];esp_fill_random(random,sizeof(random));
 portENTER_CRITICAL(&state_lock);
 for(unsigned i=0;i<12;i++)ap_password[i]="23456789ABCDEFGHJKLMNPQRSTUVWXYZ"[random[i]%31];
 ap_password[12]=0;portEXIT_CRITICAL(&state_lock);
 mbedtls_platform_zeroize(random,sizeof(random));
 wifi_config_t config={0};strcpy((char *)config.ap.ssid,"StopWatch-setup");config.ap.ssid_len=strlen((char *)config.ap.ssid);
 strcpy((char *)config.ap.password,ap_password);config.ap.channel=1;config.ap.max_connection=1;config.ap.authmode=WIFI_AUTH_WPA2_PSK;
 esp_err_t result=esp_wifi_set_mode(WIFI_MODE_APSTA);
 if(result==ESP_OK)result=esp_wifi_set_config(WIFI_IF_AP,&config);
 mbedtls_platform_zeroize(&config,sizeof(config));
 if(result==ESP_OK&&!started){result=esp_wifi_start();started=result==ESP_OK;}
 if(result==ESP_OK){
  ap_deadline=now_ms()+600000;
  portENTER_CRITICAL(&state_lock);state.setup_ap=true;state.ap_address=ESP_IP4TOADDR(192,168,4,1);++state.generation;portEXIT_CRITICAL(&state_lock);
  mcp_network_lost();
 }else{esp_wifi_set_mode(WIFI_MODE_STA);ble_input_enable(bridge_input_mode()==INPUT_BLE);}
 return result;
}
static void stop_ap(void){
 device_stop();esp_err_t result=esp_wifi_set_mode(WIFI_MODE_STA);if(result!=ESP_OK){set_error(result);return;}
 portENTER_CRITICAL(&state_lock);state.setup_ap=false;state.ap_address=0;++state.generation;portEXIT_CRITICAL(&state_lock);
 mcp_network_lost();portENTER_CRITICAL(&state_lock);mbedtls_platform_zeroize(ap_password,sizeof(ap_password));portEXIT_CRITICAL(&state_lock);ble_input_enable(bridge_input_mode()==INPUT_BLE);
}
static void manager(void *unused){
 (void)unused;
 for(;;){
  if(xSemaphoreTake(config_lock,pdMS_TO_TICKS(20))==pdTRUE){
   wireless_state current=wireless_snapshot();
   if(current.setup_ap&&(int32_t)(now_ms()-ap_deadline)>=0)atomic_store(&ap_requested,false);
   if(atomic_load(&ap_requested)&&!current.setup_ap){esp_err_t r=start_ap();if(r!=ESP_OK){set_error(r);atomic_store(&ap_requested,false);}}
   if(!atomic_load(&ap_requested)&&wireless_snapshot().setup_ap)stop_ap();
   if(atomic_exchange(&restart_requested,false)){
    lost();esp_wifi_disconnect();connecting=false;selected=0;xEventGroupClearBits(scan_events,STA_DISCONNECTED);connect_deadline=now_ms()+500;
   }
   current=wireless_snapshot();
   if(!atomic_load(&scan_busy)&&!current.connected&&stored.count){
    bool failed=(xEventGroupGetBits(scan_events)&STA_DISCONNECTED)!=0;
    if(connecting&&(failed||(int32_t)(now_ms()-connect_deadline)>=0)){
     esp_wifi_disconnect();connecting=false;selected=(selected+1)%stored.count;xEventGroupClearBits(scan_events,STA_DISCONNECTED);connect_deadline=now_ms()+1000;
    }else if(!connecting&&(int32_t)(now_ms()-connect_deadline)>=0){xEventGroupClearBits(scan_events,STA_DISCONNECTED);connect_saved();}
   }else if(current.connected)connecting=false;
   if(!stored.count&&!current.setup_ap&&started){esp_wifi_stop();started=false;connecting=false;}
   xSemaphoreGive(config_lock);
  }
  vTaskDelay(pdMS_TO_TICKS(250));
 }
}
void wireless_init(void){
 config_lock=xSemaphoreCreateMutex();configASSERT(config_lock);scan_events=xEventGroupCreate();configASSERT(scan_events);
 esp_err_t result=nvs_flash_init();if(result==ESP_OK)result=esp_event_loop_create_default();if(result!=ESP_OK){set_error(result);return;}
 station=esp_netif_create_default_wifi_sta();access_point=esp_netif_create_default_wifi_ap();
 if(!station||!access_point){set_error(ESP_ERR_NO_MEM);return;}
 wifi_init_config_t config=WIFI_INIT_CONFIG_DEFAULT();result=esp_wifi_init(&config);
 if(result==ESP_OK)result=esp_wifi_set_storage(WIFI_STORAGE_RAM);
 if(result==ESP_OK)result=esp_event_handler_register(WIFI_EVENT,ESP_EVENT_ANY_ID,event,NULL);
 if(result==ESP_OK)result=esp_event_handler_register(IP_EVENT,IP_EVENT_STA_GOT_IP,event,NULL);
 if(result==ESP_OK)result=esp_event_handler_register(IP_EVENT,IP_EVENT_STA_LOST_IP,event,NULL);
 if(result==ESP_OK)result=esp_wifi_set_mode(WIFI_MODE_STA);
 if(result!=ESP_OK){set_error(result);return;}
 stored.magic=NETWORK_STORE_MAGIC;nvs_handle_t handle;
 if(nvs_open_from_partition("mcp_kv","wireless",NVS_READONLY,&handle)==ESP_OK){
  unsigned char blob[sizeof(network_store)]={0};size_t size=sizeof(blob);result=nvs_get_blob(handle,"v2",blob,&size);
  if(result==ESP_ERR_NVS_NOT_FOUND){size=sizeof(blob);result=nvs_get_blob(handle,"v1",blob,&size);}
  if(result==ESP_OK)network_store_decode(&stored,blob,size);
  mbedtls_platform_zeroize(blob,sizeof(blob));nvs_close(handle);
 }
 configured();portENTER_CRITICAL(&state_lock);state.ready=true;portEXIT_CRITICAL(&state_lock);atomic_store(&ready,true);
 configASSERT(xTaskCreate(manager,"wifi-manager",4096,NULL,1,NULL)==pdPASS);
}
esp_err_t wireless_configure(const char *ssid,const char *password){
 if(!atomic_load(&ready)||atomic_load(&scan_busy))return ESP_ERR_INVALID_STATE;
 xSemaphoreTake(config_lock,portMAX_DELAY);network_store next=stored;
 bool valid=ssid[0]?network_store_put(&next,ssid,password):!password[0];
 if(!ssid[0]&&valid){mbedtls_platform_zeroize(&next,sizeof(next));next.magic=NETWORK_STORE_MAGIC;}
 bool full=stored.count==NETWORK_LIMIT;
 for(unsigned i=0;i<stored.count;i++)if(!strcmp(stored.entries[i].ssid,ssid))full=false;
 esp_err_t result=valid&&!atomic_load(&scan_busy)?save(&next):full&&ssid[0]?ESP_ERR_NO_MEM:ESP_ERR_INVALID_ARG;
 mbedtls_platform_zeroize(&next,sizeof(next));xSemaphoreGive(config_lock);return result;
}
esp_err_t wireless_forget(const char *ssid){
 if(!atomic_load(&ready)||atomic_load(&scan_busy))return ESP_ERR_INVALID_STATE;
 xSemaphoreTake(config_lock,portMAX_DELAY);network_store next=stored;
 esp_err_t result=network_store_remove(&next,ssid)?save(&next):ESP_ERR_NOT_FOUND;
 mbedtls_platform_zeroize(&next,sizeof(next));xSemaphoreGive(config_lock);return result;
}
unsigned wireless_saved_names(char names[5][33]){
 xSemaphoreTake(config_lock,portMAX_DELAY);unsigned count=stored.count;
 for(unsigned i=0;i<count;i++)strcpy(names[i],stored.entries[i].ssid);
 xSemaphoreGive(config_lock);return count;
}
esp_err_t wireless_setup_start(void){if(!atomic_load(&ready))return ESP_ERR_INVALID_STATE;atomic_store(&ap_requested,true);return ESP_OK;}
void wireless_setup_stop(void){atomic_store(&ap_requested,false);}
void wireless_setup_credentials(char *name,size_t ns,char *password,size_t ps){
 portENTER_CRITICAL(&state_lock);
 snprintf(name,ns,"%s",state.setup_ap?"StopWatch-setup":"");
 snprintf(password,ps,"%s",state.setup_ap?ap_password:"");
 portEXIT_CRITICAL(&state_lock);
}
static bool valid_ssid(const uint8_t *name,size_t length) {
    if (!length || length>32) return false;
    for (size_t i=0;i<length;) {
        uint8_t lead=name[i++];
        if (lead<0x80) { if (lead<32 || lead==127) return false; continue; }
        unsigned extra; uint32_t point,minimum;
        if (lead>=0xc2 && lead<=0xdf) {extra=1;point=lead&31;minimum=0x80;}
        else if (lead>=0xe0 && lead<=0xef) {extra=2;point=lead&15;minimum=0x800;}
        else if (lead>=0xf0 && lead<=0xf4) {extra=3;point=lead&7;minimum=0x10000;}
        else return false;
        if (i+extra>length) return false;
        for (unsigned j=0;j<extra;++j) {
            uint8_t tail=name[i++]; if ((tail&0xc0)!=0x80) return false;
            point=(point<<6)|(tail&63);
        }
        if (point<minimum || point>0x10ffff || (point>=0xd800 && point<=0xdfff)) return false;
    }
    return true;
}
void wireless_scan_snapshot(wireless_scan_result *out) {
    portENTER_CRITICAL(&state_lock); *out=scan_result; portEXIT_CRITICAL(&state_lock);
    out->busy=atomic_load(&scan_busy);
}
static void scan_worker(void *unused) {
    (void)unused;
    xSemaphoreTake(config_lock,portMAX_DELAY);
    esp_err_t result=ESP_OK;
    xEventGroupClearBits(scan_events,SCAN_FINISHED|STA_DISCONNECTED);
    atomic_store(&scan_driver_status,0);
    if (!started) { result=esp_wifi_start(); started=result==ESP_OK; }
    if (result==ESP_OK && !wireless_snapshot().connected) {
        // Stop a pending connection; retries remain paused for this scan.
        if (esp_wifi_disconnect()==ESP_OK)
            xEventGroupWaitBits(scan_events,STA_DISCONNECTED,pdTRUE,pdFALSE,pdMS_TO_TICKS(500));
    }
    wifi_scan_config_t scan={.show_hidden=false,.scan_type=WIFI_SCAN_TYPE_ACTIVE,
                            .scan_time.active={.min=0,.max=120},.coex_background_scan=true};
    if (result==ESP_OK) result=esp_wifi_scan_start(&scan,false);
    if (result==ESP_OK && !(xEventGroupWaitBits(scan_events,SCAN_FINISHED,pdTRUE,pdFALSE,
                                               pdMS_TO_TICKS(8000))&SCAN_FINISHED)) {
        result=ESP_ERR_TIMEOUT;
        esp_wifi_scan_stop();
        // Drain terminal completion before a future scan can start.
        xEventGroupWaitBits(scan_events,SCAN_FINISHED,pdTRUE,pdFALSE,pdMS_TO_TICKS(1000));
    }
    if (result==ESP_OK && atomic_load(&scan_driver_status)) result=ESP_FAIL;
    uint16_t count=40;
    wifi_ap_record_t *records=NULL;
    wireless_scan_result *next=calloc(1,sizeof(*next));
    if (!next) result=ESP_ERR_NO_MEM;
    if (result==ESP_OK) {
        records=calloc(count,sizeof(*records));
        if (!records) result=ESP_ERR_NO_MEM;
        else result=esp_wifi_scan_get_ap_records(&count,records); // Also frees the driver's list.
    }
    if (result!=ESP_OK) esp_wifi_clear_ap_list();
    if (result==ESP_OK) for (unsigned i=0;i<count && next->count<WIRELESS_SCAN_LIMIT;++i) {
        wifi_ap_record_t *record=&records[i];
        size_t length=strnlen((const char *)record->ssid,sizeof(record->ssid));
        if (!valid_ssid(record->ssid,length)) continue;
        if (record->authmode!=WIFI_AUTH_OPEN && record->authmode!=WIFI_AUTH_WPA2_PSK &&
            record->authmode!=WIFI_AUTH_WPA_WPA2_PSK && record->authmode!=WIFI_AUTH_WPA3_PSK &&
            record->authmode!=WIFI_AUTH_WPA2_WPA3_PSK) continue;
        bool duplicate=false;
        for (unsigned j=0;j<next->count;++j) if (!strcmp(next->networks[j].ssid,(const char *)record->ssid)) duplicate=true;
        if (duplicate) continue; // SDK sorted by RSSI: keep the strongest record of each name.
        wireless_network *network=&next->networks[next->count++];
        memcpy(network->ssid,record->ssid,length); network->ssid[length]=0;
        network->rssi=record->rssi; network->open=record->authmode==WIFI_AUTH_OPEN;
    }
    free(records);
    portENTER_CRITICAL(&state_lock);
    if (next) scan_result=*next;
    scan_result.done=true; scan_result.error=result;
    portEXIT_CRITICAL(&state_lock);
    free(next);
    // Scan-only startup is not a saved network and does not survive reboot.
    if (!wireless_snapshot().configured && !wireless_snapshot().setup_ap && started) { esp_wifi_stop(); started=false; }
    atomic_store(&scan_busy,false);
    connecting=false;connect_deadline=now_ms()+500;
    xSemaphoreGive(config_lock);
    vTaskDelete(NULL);
}
esp_err_t wireless_scan_begin(void) {
    if (!atomic_load(&ready)) return ESP_ERR_INVALID_STATE;
    if (xSemaphoreTake(config_lock,0)!=pdTRUE) return ESP_ERR_INVALID_STATE;
    bool expected=false;
    if (!atomic_compare_exchange_strong(&scan_busy,&expected,true)) {
        xSemaphoreGive(config_lock); return ESP_ERR_INVALID_STATE;
    }
    portENTER_CRITICAL(&state_lock); memset(&scan_result,0,sizeof(scan_result)); portEXIT_CRITICAL(&state_lock);
    if (xTaskCreate(scan_worker,"wifi-scan",4096,NULL,1,NULL)!=pdPASS) {
        atomic_store(&scan_busy,false); xSemaphoreGive(config_lock); return ESP_ERR_NO_MEM;
    }
    xSemaphoreGive(config_lock);
    return ESP_OK;
}
