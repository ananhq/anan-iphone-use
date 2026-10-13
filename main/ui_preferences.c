// SPDX-License-Identifier: Apache-2.0
#include "ui_preferences.h"
#include "nvs.h"
ui_language ui_language_load(void) {
    nvs_handle_t h;
    uint8_t language = 0;
    if (nvs_open_from_partition("mcp_kv", "ui", NVS_READONLY, &h) == ESP_OK) {
        nvs_get_u8(h, "language", &language);
        nvs_close(h);
    }
    return language == UI_EN ? UI_EN : UI_ZH;
}
bool ui_language_save(ui_language language) {
    nvs_handle_t h;
    if (nvs_open_from_partition("mcp_kv", "ui", NVS_READWRITE, &h) != ESP_OK)
        return false;
    esp_err_t result = nvs_set_u8(h, "language", language);
    if (result == ESP_OK)
        result = nvs_commit(h);
    nvs_close(h);
    return result == ESP_OK;
}
