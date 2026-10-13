// SPDX-License-Identifier: Apache-2.0
#include "bridge.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_check.h"

static i2c_master_bus_handle_t board_bus;
i2c_master_bus_handle_t board_i2c_bus(void) { return board_bus; }

void board_init(void) {
    // StopWatch's system I2C: SDA=47, SCL=48, M5PM1=0x6e.
    // Hold only the MCU's 3.3V rail; do not enable its external 5V boost.
    i2c_master_bus_handle_t bus;
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0, .sda_io_num = 47, .scl_io_num = 48,
        .clk_source = I2C_CLK_SRC_DEFAULT, .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus));
    i2c_device_config_t device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7, .device_address = 0x6e, .scl_speed_hz = 100000
    };
    i2c_master_dev_handle_t power;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &device_config, &power));
    uint8_t reg = 0x07, hold;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(power, &reg, 1, &hold, 1, 100));
    uint8_t write[] = {reg, hold | (1u << 5)};
    ESP_ERROR_CHECK(i2c_master_transmit(power, write, sizeof(write), 100));
    board_bus = bus;
    gpio_config_t buttons = {
        .pin_bit_mask = (1ULL << 1) | (1ULL << 2), .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE, .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&buttons));
}
bool board_stop_pressed(void) {
    return gpio_get_level(1) == 0 || gpio_get_level(2) == 0;
}
