// SPDX-License-Identifier: Apache-2.0
#include "bridge.h"
#include "tinyusb.h"
#include "tinyusb_default_config.h"
#include "tinyusb_net.h"
#include "tusb.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/etharp.h"
#include "netif/ethernet.h"
#include <stdatomic.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static const uint8_t reports[] = {
    TUD_HID_REPORT_DESC_ABSMOUSE(HID_REPORT_ID(1)),
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(2)),
    TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(3))
};
static const tusb_desc_device_t device = {
    .bLength = sizeof(tusb_desc_device_t), .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200, .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON, .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = 64, .idVendor = 0x303a, .idProduct = 0x4008,
    .bcdDevice = 0x0100, .iManufacturer = 1, .iProduct = 2, .bNumConfigurations = 1
};
static const uint8_t configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, 3, 0, TUD_CONFIG_DESC_LEN + TUD_CDC_NCM_DESC_LEN + TUD_HID_DESC_LEN, 0, 100),
    TUD_CDC_NCM_DESCRIPTOR(0, 3, 4, 0x81, 64, 0x02, 0x82, 64, 1514),
    TUD_HID_DESCRIPTOR(2, 5, HID_ITF_PROTOCOL_NONE, sizeof(reports), 0x83, 16, 5)
};
static char host_mac_string[13];
static const char *strings[] = {(const char[]){0x09, 0x04}, "Anan prototype", "Anan StopWatch", "USB Ethernet", host_mac_string, "Mouse and keyboard"};
static atomic_bool online;
static atomic_uint completed;
static struct netif network;
static SemaphoreHandle_t initialized;
static uint8_t mac[6];
static uint16_t pointer_x, pointer_y;
static bool consumer_held;

static void device_event(tinyusb_event_t *event, void *arg) {
    (void)arg;
    atomic_store(&online, event->id == TINYUSB_EVENT_ATTACHED || event->id == TINYUSB_EVENT_RESUMED);
}
bool bridge_usb_online(void) { return atomic_load(&online); }

// Only the action worker calls these, so each report has one completion owner.
static bool report_ready(void) {
    int64_t until = esp_timer_get_time() + 200000;
    while (bridge_usb_online() && !tud_hid_ready() && esp_timer_get_time() < until) vTaskDelay(pdMS_TO_TICKS(2));
    return bridge_usb_online() && tud_hid_ready();
}
static bool report_completed(unsigned before) {
    int64_t until = esp_timer_get_time() + 200000;
    while (bridge_usb_online() && atomic_load(&completed) == before && esp_timer_get_time() < until) vTaskDelay(pdMS_TO_TICKS(2));
    return bridge_usb_online() && atomic_load(&completed) != before;
}
bool bridge_mouse(uint8_t buttons, uint16_t x, uint16_t y) {
    if (!report_ready()) return false;
    unsigned before = atomic_load(&completed);
    pointer_x = x; pointer_y = y;
    return tud_hid_n_abs_mouse_report(0, 1, buttons, x >> 1, y >> 1, 0, 0) && report_completed(before);
}
bool bridge_key(uint8_t modifiers, uint8_t key) {
    if (!report_ready()) return false;
    unsigned before = atomic_load(&completed);
    uint8_t keys[6] = {key};
    return tud_hid_n_keyboard_report(0, 2, modifiers, keys) && report_completed(before);
}
bool bridge_scroll(uint16_t x, uint16_t y, int8_t vertical, int8_t horizontal) {
    if (!report_ready()) return false;
    unsigned before = atomic_load(&completed);
    pointer_x = x; pointer_y = y;
    return tud_hid_n_abs_mouse_report(0, 1, 0, x >> 1, y >> 1, vertical, horizontal) &&
           report_completed(before);
}
bool bridge_consumer(uint16_t usage) {
    if (!report_ready()) return false;
    unsigned before = atomic_load(&completed);
    // Remember uncertain presses too: cleanup must attempt a zero report.
    if (usage) consumer_held = true;
    bool sent = tud_hid_n_report(0, 3, &usage, sizeof(usage)) && report_completed(before);
    if (sent && !usage) consumer_held = false;
    return sent;
}
void bridge_release(void) {
    bridge_mouse(0, pointer_x, pointer_y);
    bridge_key(0, 0);
    // Legacy actions send no additional report unless a Consumer press needs release.
    if (consumer_held) bridge_consumer(0);
}
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) { (void)instance; return reports; }
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t id, hid_report_type_t type, uint8_t *buffer, uint16_t len) {
    (void)instance; (void)id; (void)type; (void)buffer; (void)len; return 0;
}
void tud_hid_set_report_cb(uint8_t instance, uint8_t id, hid_report_type_t type, uint8_t const *buffer, uint16_t len) {
    (void)instance; (void)id; (void)type; (void)buffer; (void)len;
}
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, uint16_t len) {
    (void)instance; (void)report; (void)len; atomic_fetch_add(&completed, 1);
}

static err_t output_packet(struct netif *net, struct pbuf *packet) {
    (void)net;
    if (!bridge_usb_online() || packet->tot_len > 1514) return ERR_IF;
    void *bytes = malloc(packet->tot_len);
    if (!bytes) return ERR_MEM;
    pbuf_copy_partial(packet, bytes, packet->tot_len, 0);
    esp_err_t result = tinyusb_net_send_sync(bytes, packet->tot_len, NULL, pdMS_TO_TICKS(50));
    free(bytes);
    return result == ESP_OK ? ERR_OK : ERR_IF;
}
static err_t configure_network(struct netif *net) {
    net->name[0] = 's'; net->name[1] = 'w'; net->mtu = 1500;
    net->hwaddr_len = 6; memcpy(net->hwaddr, mac, sizeof(mac));
    net->output = etharp_output; net->linkoutput = output_packet;
    net->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET;
    return ERR_OK;
}
static void add_network(void *arg) {
    (void)arg;
    ip4_addr_t address, mask, gateway;
    IP4_ADDR(&address, 192, 168, 7, 1); IP4_ADDR(&mask, 255, 255, 255, 0); ip4_addr_set_zero(&gateway);
    if (netif_add(&network, &address, &mask, &gateway, NULL, configure_network, tcpip_input)) {
        netif_set_up(&network); netif_set_link_up(&network);
    }
    xSemaphoreGive(initialized);
}
static esp_err_t receive_packet(void *bytes, uint16_t length, void *arg) {
    (void)arg;
    if (length < 14 || length > 1514) return ESP_FAIL;
    struct pbuf *packet = pbuf_alloc(PBUF_RAW, length, PBUF_POOL);
    if (!packet) return ESP_ERR_NO_MEM;
    if (pbuf_take(packet, bytes, length) != ERR_OK || network.input(packet, &network) != ERR_OK) {
        pbuf_free(packet); return ESP_FAIL;
    }
    return ESP_OK;
}
void bridge_usb_init(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_read_mac(mac, ESP_MAC_EFUSE_FACTORY));
    mac[0] = (mac[0] | 2) & 0xfe;
    uint8_t host_mac[6]; memcpy(host_mac, mac, sizeof(mac)); host_mac[5] ^= 1;
    for (unsigned i = 0; i < 6; ++i) snprintf(host_mac_string + i * 2, 3, "%02X", host_mac[i]);
    initialized = xSemaphoreCreateBinary();
    configASSERT(initialized);
    ESP_ERROR_CHECK(tcpip_callback(add_network, NULL) == ERR_OK ? ESP_OK : ESP_FAIL);
    configASSERT(xSemaphoreTake(initialized, pdMS_TO_TICKS(2000)) == pdTRUE);
    configASSERT(network.input != NULL);
    vSemaphoreDelete(initialized);
    tinyusb_config_t usb = TINYUSB_DEFAULT_CONFIG();
    usb.descriptor.device = &device; usb.descriptor.full_speed_config = configuration;
    usb.descriptor.string = strings; usb.descriptor.string_count = sizeof(strings) / sizeof(strings[0]);
    usb.phy.self_powered = false; usb.event_cb = device_event;
    ESP_ERROR_CHECK(tinyusb_driver_install(&usb));
    tinyusb_net_config_t net = {.on_recv_callback = receive_packet};
    memcpy(net.mac_addr, host_mac, sizeof(host_mac));
    ESP_ERROR_CHECK(tinyusb_net_init(&net));
}
