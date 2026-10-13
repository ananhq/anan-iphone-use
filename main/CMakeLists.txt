idf_component_register(SRCS "main.c" "usb.c" "board.c"
    INCLUDE_DIRS "." REQUIRES esp_driver_gpio esp_driver_i2c esp_netif esp_timer lwip)
