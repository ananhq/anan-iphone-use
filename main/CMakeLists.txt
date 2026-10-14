idf_component_register(SRCS "main.c" "usb.c" "input.c" "ble_input.c" "ble_reports.c" "wireless.c" "network_store.c" "network_authority.c" "discovery.c" "wifi_qr.c" "vendor/qrcodegen.c" "board.c" "actions.c" "hid_input.c" "identity.c" "mcp.c" "device.c" "display.c" "touch.c" "ui.c" "ui_layout.c" "ui_preferences.c"
    INCLUDE_DIRS "." REQUIRES esp_driver_gpio esp_driver_i2c esp_driver_spi esp_lcd
    esp_netif esp_timer lwip esp_http_server esp_https_server json mbedtls nvs_flash bootloader_support esp_wifi bt
    PRIV_REQUIRES esp_app_format
    EMBED_TXTFILES "web/setup.html")
