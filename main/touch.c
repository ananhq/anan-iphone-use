// SPDX-License-Identifier: Apache-2.0
// Register facts: M5StopWatch-UserDemo 6b4aa125288b6fe9dca661f10159f6e1e5ee785c,
// main/hal/drivers/cst820; MIT notice retained in licenses/M5Stack-demo-MIT.txt.
#include "touch.h"
#include "display.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdatomic.h>
#include <string.h>
static i2c_master_dev_handle_t chip;
static atomic_bool ready;
static atomic_int error;
extern i2c_master_bus_handle_t board_i2c_bus(void);
bool touch_ready(void) { return atomic_load(&ready); }
int touch_error(void) { return atomic_load(&error); }
bool touch_init(void) {
    if (!display_reset_touch()) {
        atomic_store(&error, ESP_FAIL);
        return false;
    }
    gpio_config_t irq = {.pin_bit_mask = 1ULL << 13,
                         .mode = GPIO_MODE_INPUT,
                         .pull_up_en = GPIO_PULLUP_ENABLE,
                         .intr_type = GPIO_INTR_DISABLE};
    esp_err_t result = gpio_config(&irq);
    i2c_device_config_t config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7, .device_address = 0x15, .scl_speed_hz = 100000};
    if (result == ESP_OK)
        result = i2c_master_bus_add_device(board_i2c_bus(), &config, &chip);
    if (result != ESP_OK) {
        atomic_store(&error, result);
        return false;
    }
    uint8_t id = 0, version = 0, reg = 0xa7;
    for (unsigned attempt = 0; attempt < 3; ++attempt) {
        result = i2c_master_transmit_receive(chip, &reg, 1, &id, 1, 20);
        uint8_t version_reg = 0xa9;
        if (result == ESP_OK)
            result = i2c_master_transmit_receive(chip, &version_reg, 1, &version, 1, 20);
        if (result == ESP_OK && id && version) {
            atomic_store(&ready, true);
            atomic_store(&error, 0);
            ESP_LOGI("touch", "CST820B ready");
            return true;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    i2c_master_bus_rm_device(chip);
    chip = NULL;
    atomic_store(&error, result == ESP_OK ? ESP_ERR_INVALID_RESPONSE : result);
    return false;
}
bool touch_read(touch_point *point) {
    memset(point, 0, sizeof(*point));
    if (!chip)
        return false;
    uint8_t reg = 0, bytes[7];
    esp_err_t result = i2c_master_transmit_receive(chip, &reg, 1, bytes, sizeof(bytes), 20);
    if (result != ESP_OK) {
        atomic_store(&error, result);
        return false;
    }
    point->event = bytes[3] >> 6;
    point->x = ((bytes[3] & 15) << 8) | bytes[4];
    point->y = ((bytes[5] & 15) << 8) | bytes[6];
    point->pressed = bytes[2] == 1 && (point->event == 0 || point->event == 2);
    if (bytes[2] > 1 || (point->pressed && (point->x >= 466 || point->y >= 466))) {
        atomic_store(&error, ESP_ERR_INVALID_RESPONSE);
        return false;
    }
    atomic_store(&error, 0);
    return true;
}
