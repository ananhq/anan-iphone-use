idf_component_register(SRCS "main.c" "usb.c" "board.c" "actions.c" "identity.c" "mcp.c" "device.c" "display.c" "touch.c" "ui.c" "ui_layout.c" "ui_preferences.c"
    INCLUDE_DIRS "." REQUIRES esp_driver_gpio esp_driver_i2c esp_driver_spi esp_lcd
    esp_netif esp_timer lwip esp_http_server esp_https_server json mbedtls nvs_flash bootloader_support
    PRIV_REQUIRES esp_app_format
    EMBED_TXTFILES "web/setup.html")
