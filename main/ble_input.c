// SPDX-License-Identifier: Apache-2.0
// Experimental HOGP peripheral using pinned ESP-IDF NimBLE APIs, not a demo task.
#include "input_backends.h"
#include "ble_reports.h"
#include "bridge.h"
#include "device.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "host/ble_store.h"
#include "store/config/ble_store_config.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tusb.h"
#include <stdatomic.h>
#include <string.h>
// Pinned NimBLE's store initializer is exported but not declared in its public header.
void ble_store_config_init(void);

static const uint8_t report_map[] = {
    TUD_HID_REPORT_DESC_ABSMOUSE(HID_REPORT_ID(1)),
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(2)),
    TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(3)),
    TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(4))
};
static const uint8_t report_sizes[] = {7, 8, 2, 5};
static uint16_t report_handles[4];
static uint8_t last_reports[4][8];
static portMUX_TYPE report_lock = portMUX_INITIALIZER_UNLOCKED;
static atomic_bool initialized, enabled, connected, encrypted, suspended, forgetting;
static atomic_int last_error;
static atomic_uint connection = BLE_HS_CONN_HANDLE_NONE, subscribed, completed;
static atomic_uint pending_handle, completion_status, link_generation;
static uint8_t own_address_type;
static uint8_t buttons;
static bool consumer_held;
static unsigned report_generation;
static int gap_event(struct ble_gap_event *event, void *arg);
static int access_device(uint16_t conn,uint16_t attr,struct ble_gatt_access_ctxt *ctxt,void *arg) {
    (void)conn; (void)attr;
    const uint8_t pnp[]={2,0x3a,0x30,0x08,0x40,0,1}; // Same development IDs as USB; not assigned IDs.
    const char *manufacturer="Anan prototype";
    const void *value=arg ? (const void *)pnp : (const void *)manufacturer;
    size_t size=arg ? sizeof(pnp) : strlen(manufacturer);
    return os_mbuf_append(ctxt->om,value,size) ? BLE_ATT_ERR_INSUFFICIENT_RES : 0;
}
static void stop_input(void) {
    if (bridge_input_mode() == INPUT_BLE) device_stop();
}
bool ble_input_ready(void) { return atomic_load(&initialized); }
int ble_input_error(void) { return atomic_load(&last_error); }
bool ble_input_connected(void) { return atomic_load(&connected); }
bool ble_input_online(void) {
    // Consumer keys are optional; require positioning, keyboard and relative mouse.
    return ble_input_ready() && atomic_load(&enabled) && atomic_load(&connected) &&
           atomic_load(&encrypted) && !atomic_load(&suspended) &&
           (atomic_load(&subscribed) & 0x0b) == 0x0b;
}
static int access_report(uint16_t conn, uint16_t attr, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    (void)conn; (void)attr;
    unsigned index = (uintptr_t)arg;
    if (ctxt->op != BLE_GATT_ACCESS_OP_READ_CHR) return BLE_ATT_ERR_WRITE_NOT_PERMITTED;
    uint8_t copy[8];
    portENTER_CRITICAL(&report_lock);
    memcpy(copy, last_reports[index], report_sizes[index]);
    portEXIT_CRITICAL(&report_lock);
    return os_mbuf_append(ctxt->om, copy, report_sizes[index]) ? BLE_ATT_ERR_INSUFFICIENT_RES : 0;
}
static int access_reference(uint16_t conn,uint16_t attr,struct ble_gatt_access_ctxt *ctxt,void *arg) {
    (void)conn; (void)attr;
    if (ctxt->op != BLE_GATT_ACCESS_OP_READ_DSC) return BLE_ATT_ERR_WRITE_NOT_PERMITTED;
    unsigned index = (uintptr_t)arg;
    uint8_t reference[2] = {index == 4 ? 2 : index + 1, index == 4 ? 2 : 1};
    return os_mbuf_append(ctxt->om,reference,2) ? BLE_ATT_ERR_INSUFFICIENT_RES : 0;
}
static int access_hid(uint16_t conn,uint16_t attr,struct ble_gatt_access_ctxt *ctxt,void *arg) {
    (void)conn; (void)attr;
    unsigned kind = (uintptr_t)arg;
    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
        const uint8_t info[] = {0x11,0x01,0,2}, protocol = kind==4 ? 0 : 1;
        const void *bytes = kind == 0 ? (const void *)report_map : kind == 1 ? (const void *)info : &protocol;
        size_t size = kind == 0 ? sizeof(report_map) : kind == 1 ? sizeof(info) : 1;
        return os_mbuf_append(ctxt->om,bytes,size) ? BLE_ATT_ERR_INSUFFICIENT_RES : 0;
    }
    uint8_t value;
    if (OS_MBUF_PKTLEN(ctxt->om) != 1 || ble_hs_mbuf_to_flat(ctxt->om,&value,1,NULL))
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    if (kind == 2) return value == 1 ? 0 : BLE_ATT_ERR_REQ_NOT_SUPPORTED; // Report protocol only.
    if (kind == 3) {
        if (value > 1) return BLE_ATT_ERR_UNLIKELY;
        atomic_store(&suspended, value == 0);
        stop_input(); // Resume never restores a control grant.
    }
    return 0; // Keyboard LED output is accepted, with no input side effect.
}
#define REFERENCE(index) static struct ble_gatt_dsc_def reference_##index[] = { \
    {.uuid=BLE_UUID16_DECLARE(0x2908), .att_flags=BLE_ATT_F_READ, \
     .access_cb=access_reference, .arg=(void *)(uintptr_t)index}, {0}}
REFERENCE(0); REFERENCE(1); REFERENCE(2); REFERENCE(3); REFERENCE(4);
#define INPUT_REPORT(index) {.uuid=BLE_UUID16_DECLARE(0x2a4d), \
    .flags=BLE_GATT_CHR_F_READ|BLE_GATT_CHR_F_READ_ENC|BLE_GATT_CHR_F_NOTIFY, \
    .access_cb=access_report,.arg=(void *)(uintptr_t)index, \
    .val_handle=&report_handles[index],.descriptors=reference_##index}
static const struct ble_gatt_svc_def services[] = {
    {.type=BLE_GATT_SVC_TYPE_PRIMARY,.uuid=BLE_UUID16_DECLARE(0x180a),
     .characteristics=(const struct ble_gatt_chr_def[]){
        {.uuid=BLE_UUID16_DECLARE(0x2a29),.flags=BLE_GATT_CHR_F_READ,.access_cb=access_device},
        {.uuid=BLE_UUID16_DECLARE(0x2a50),.flags=BLE_GATT_CHR_F_READ,.access_cb=access_device,.arg=(void *)1}, {0}}},
    {.type=BLE_GATT_SVC_TYPE_PRIMARY,.uuid=BLE_UUID16_DECLARE(0x1812),
     .characteristics=(const struct ble_gatt_chr_def[]){
        {.uuid=BLE_UUID16_DECLARE(0x2a4b),.flags=BLE_GATT_CHR_F_READ|BLE_GATT_CHR_F_READ_ENC,
         .access_cb=access_hid,.arg=(void *)0},
        {.uuid=BLE_UUID16_DECLARE(0x2a4a),.flags=BLE_GATT_CHR_F_READ,.access_cb=access_hid,.arg=(void *)1},
        {.uuid=BLE_UUID16_DECLARE(0x2a4e),.flags=BLE_GATT_CHR_F_READ|BLE_GATT_CHR_F_WRITE_NO_RSP|
         BLE_GATT_CHR_F_WRITE_ENC,.access_cb=access_hid,.arg=(void *)2},
        {.uuid=BLE_UUID16_DECLARE(0x2a4c),.flags=BLE_GATT_CHR_F_WRITE_NO_RSP|BLE_GATT_CHR_F_WRITE_ENC,
         .access_cb=access_hid,.arg=(void *)3},
        INPUT_REPORT(0), INPUT_REPORT(1), INPUT_REPORT(2), INPUT_REPORT(3),
        {.uuid=BLE_UUID16_DECLARE(0x2a4d),.flags=BLE_GATT_CHR_F_READ|BLE_GATT_CHR_F_WRITE|
         BLE_GATT_CHR_F_WRITE_NO_RSP|BLE_GATT_CHR_F_READ_ENC|BLE_GATT_CHR_F_WRITE_ENC,
         .access_cb=access_hid,.arg=(void *)4,.descriptors=reference_4}, {0}}}, {0}
};
static void advertise(void) {
    if (!ble_input_ready() || !atomic_load(&enabled) || atomic_load(&connected) ||
        atomic_load(&forgetting) || ble_gap_adv_active()) return;
    const char *name = "Anan StopWatch";
    struct ble_hs_adv_fields fields = {0};
    ble_uuid16_t uuid = BLE_UUID16_INIT(0x1812);
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    fields.name = (uint8_t *)name; fields.name_len = strlen(name); fields.name_is_complete = 1;
    fields.uuids16 = &uuid; fields.num_uuids16 = 1; fields.uuids16_is_complete = 1;
    fields.appearance = 0x03c0; fields.appearance_is_present = 1;
    int result = ble_gap_adv_set_fields(&fields);
    struct ble_gap_adv_params params = {.conn_mode=BLE_GAP_CONN_MODE_UND,.disc_mode=BLE_GAP_DISC_MODE_GEN};
    if (!result) result = ble_gap_adv_start(own_address_type,NULL,BLE_HS_FOREVER,&params,gap_event,NULL);
    if (result) atomic_store(&last_error,result);
}
void ble_input_enable(bool value) {
    atomic_store(&enabled,value);
    if (!ble_input_ready()) return;
    if (value) advertise();
    else {
        ble_gap_adv_stop();
        unsigned conn = atomic_load(&connection);
        if (conn != BLE_HS_CONN_HANDLE_NONE) ble_gap_terminate(conn,BLE_ERR_REM_USER_CONN_TERM);
    }
}
static void clear_link(void) {
    atomic_store(&connected,false); atomic_store(&encrypted,false);
    atomic_store(&subscribed,0); atomic_store(&connection,BLE_HS_CONN_HANDLE_NONE);
    atomic_fetch_add(&link_generation,1);
    stop_input();
}
static int gap_event(struct ble_gap_event *event, void *arg) {
    (void)arg;
    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT: {
        if (event->connect.status) { advertise(); break; }
        struct ble_gap_conn_desc desc;
        int count = 0;
        ble_store_util_count(BLE_STORE_OBJ_TYPE_PEER_SEC,&count);
        if (ble_gap_conn_find(event->connect.conn_handle,&desc)) return 0;
        struct ble_store_key_sec key = {.peer_addr=desc.peer_id_addr};
        struct ble_store_value_sec stored;
        if (!atomic_load(&enabled) || atomic_load(&forgetting) || (count && ble_store_read_peer_sec(&key,&stored))) {
            ble_gap_terminate(event->connect.conn_handle,BLE_ERR_REM_USER_CONN_TERM); break;
        }
        atomic_store(&connection,event->connect.conn_handle);
        atomic_store(&connected,true); atomic_store(&encrypted,false);
        atomic_store(&subscribed,0); atomic_store(&suspended,false);
        atomic_fetch_add(&link_generation,1);
        portENTER_CRITICAL(&report_lock); memset(last_reports,0,sizeof(last_reports)); portEXIT_CRITICAL(&report_lock);
        stop_input();
        int result = ble_gap_security_initiate(event->connect.conn_handle);
        if (result) { atomic_store(&last_error,result); ble_gap_terminate(event->connect.conn_handle,BLE_ERR_REM_USER_CONN_TERM); }
        break;
    }
    case BLE_GAP_EVENT_ENC_CHANGE: {
        struct ble_gap_conn_desc desc;
        bool ok = !event->enc_change.status && !ble_gap_conn_find(event->enc_change.conn_handle,&desc) &&
                  desc.sec_state.encrypted && desc.sec_state.bonded;
        atomic_store(&encrypted,ok);
        if (!ok) { stop_input(); ble_gap_terminate(event->enc_change.conn_handle,BLE_ERR_REM_USER_CONN_TERM); }
        break;
    }
    case BLE_GAP_EVENT_SUBSCRIBE:
        for (unsigned i=0;i<4;++i) if (report_handles[i] == event->subscribe.attr_handle) {
            if (event->subscribe.cur_notify) atomic_fetch_or(&subscribed,1u<<i);
            else { atomic_fetch_and(&subscribed,~(1u<<i)); stop_input(); }
        }
        break;
    case BLE_GAP_EVENT_NOTIFY_TX:
        if (!event->notify_tx.indication && event->notify_tx.conn_handle == atomic_load(&connection) &&
            event->notify_tx.attr_handle == atomic_load(&pending_handle)) {
            atomic_store(&completion_status,event->notify_tx.status);
            atomic_fetch_add(&completed,1);
        }
        break;
    case BLE_GAP_EVENT_DISCONNECT:
        clear_link();
        if (atomic_exchange(&forgetting,false)) {
            int rc=ble_store_clear(); if (rc) atomic_store(&last_error,rc);
        }
        advertise(); break;
    case BLE_GAP_EVENT_ADV_COMPLETE: advertise(); break;
    case BLE_GAP_EVENT_REPEAT_PAIRING: return BLE_GAP_REPEAT_PAIRING_IGNORE; // Explicit reset required.
    default: break;
    }
    return 0;
}
static void synced(void) {
    int result = ble_hs_id_infer_auto(0,&own_address_type);
    if (result) { atomic_store(&last_error,result); return; }
    atomic_store(&initialized,true); advertise();
}
static void reset(int reason) { atomic_store(&initialized,false); atomic_store(&last_error,reason); clear_link(); }
static void host_task(void *arg) { (void)arg; nimble_port_run(); nimble_port_freertos_deinit(); }
void ble_input_init(void) {
    esp_err_t result = nvs_flash_init(); // Do not erase existing bonds on initialization failure.
    if (result == ESP_OK) result = nimble_port_init();
    if (result != ESP_OK) { atomic_store(&last_error,result); return; }
    ble_hs_cfg.sync_cb = synced; ble_hs_cfg.reset_cb = reset;
    ble_hs_cfg.sm_io_cap = BLE_HS_IO_NO_INPUT_OUTPUT;
    ble_hs_cfg.sm_mitm = 0;
    ble_hs_cfg.sm_bonding = 1; ble_hs_cfg.sm_sc = 1;
    ble_hs_cfg.sm_our_key_dist = BLE_SM_PAIR_KEY_DIST_ENC|BLE_SM_PAIR_KEY_DIST_ID;
    ble_hs_cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ENC|BLE_SM_PAIR_KEY_DIST_ID;
    ble_store_config_init();
    ble_svc_gap_init(); ble_svc_gatt_init();
    int rc = ble_svc_gap_device_name_set("Anan StopWatch");
    if (!rc) rc = ble_gatts_count_cfg(services);
    if (!rc) rc = ble_gatts_add_svcs(services);
    if (rc) { atomic_store(&last_error,rc); return; }
    nimble_port_freertos_init(host_task);
}
bool ble_input_forget(void) {
    if (!ble_input_ready()) return false;
    stop_input();
    atomic_store(&forgetting,true);
    ble_gap_adv_stop();
    unsigned conn = atomic_load(&connection);
    atomic_store(&encrypted,false);
    if (conn != BLE_HS_CONN_HANDLE_NONE) {
        int rc=ble_gap_terminate(conn,BLE_ERR_REM_USER_CONN_TERM);
        if (!rc) return true; // Clear bonds after the disconnect, not during pairing/key writes.
        atomic_store(&last_error,rc);
        atomic_store(&forgetting,false);
        return false;
    }
    int rc = ble_store_clear();
    atomic_store(&forgetting,false);
    advertise();
    if (rc) atomic_store(&last_error,rc);
    return rc == 0;
}
static bool send_report(unsigned index, uint8_t *data, size_t size) {
    if (!ble_input_online() || !(atomic_load(&subscribed)&(1u<<index))) return false;
    unsigned before = atomic_load(&completed), generation = atomic_load(&link_generation);
    unsigned conn = atomic_load(&connection);
    atomic_store(&pending_handle,report_handles[index]);
    atomic_store(&completion_status,BLE_HS_EUNKNOWN);
    struct os_mbuf *buffer = ble_hs_mbuf_from_flat(data,size);
    if (!buffer) return false;
    int rc = ble_gatts_notify_custom(conn,report_handles[index],buffer); // Stack owns the buffer.
    if (rc) { atomic_store(&last_error,rc); return false; }
    int64_t deadline = esp_timer_get_time()+200000;
    while (ble_input_online() && generation == atomic_load(&link_generation) &&
           atomic_load(&completed) == before && esp_timer_get_time()<deadline) vTaskDelay(pdMS_TO_TICKS(2));
    bool ok = generation == atomic_load(&link_generation) && ble_input_online() &&
              atomic_load(&completed) != before && !atomic_load(&completion_status);
    if (!ok) {
        // Isolate an uncertain notification; a late completion must not acknowledge the next report.
        atomic_store(&encrypted,false); stop_input();
        ble_gap_terminate(conn,BLE_ERR_REM_USER_CONN_TERM);
        return false;
    }
    portENTER_CRITICAL(&report_lock); memcpy(last_reports[index],data,size); portEXIT_CRITICAL(&report_lock);
    return true; // Local notification completion, never proof of an iPhone app effect.
}
bool ble_mouse(uint8_t b,uint16_t x,uint16_t y) {
    unsigned generation=atomic_load(&link_generation);
    if (report_generation!=generation) { buttons=0; consumer_held=false; report_generation=generation; }
    uint8_t absolute[7], relative[5];
    ble_pointer_payload(absolute,x,y);
    if (!send_report(0,absolute,sizeof(absolute))) return false;
    // Separate relative buttons: iOS may ignore the absolute mouse's button bits.
    if (b != buttons) {
        ble_relative_payload(relative,b,0,0);
        if (!send_report(3,relative,sizeof(relative))) return false;
        buttons = b;
    }
    return true;
}
bool ble_scroll(uint16_t x,uint16_t y,int8_t v,int8_t h) {
    if (!ble_mouse(0,x,y)) return false;
    uint8_t relative[5]; ble_relative_payload(relative,0,v,h);
    return send_report(3,relative,sizeof(relative));
}
bool ble_key(uint8_t m,uint8_t k) {
    uint8_t report[8]; ble_keyboard_payload(report,m,k); return send_report(1,report,sizeof(report));
}
bool ble_consumer(uint16_t u) {
    uint8_t report[2]; ble_consumer_payload(report,u);
    if (u) consumer_held=true;
    bool ok=send_report(2,report,sizeof(report));
    if (ok && !u) consumer_held=false;
    return ok;
}
void ble_release(void) {
    uint8_t report[5]; ble_relative_payload(report,0,0,0);
    if (send_report(3,report,sizeof(report))) buttons=0;
    ble_key(0,0);
    if (consumer_held) ble_consumer(0);
}
