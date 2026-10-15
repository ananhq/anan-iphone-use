idf_component_register(
    SRCS
        "main.c"
        "app/reset_trace.c"
        "input/usb.c"
        "input/input.c"
        "input/ble_input.c"
        "input/ble_reports.c"
        "network/wireless.c"
        "network/network_store.c"
        "network/network_authority.c"
        "network/discovery.c"
        "network/wifi_qr.c"
        "vendor/qrcodegen.c"
        "hardware/board.c"
        "hardware/battery.c"
        "input/actions.c"
        "ui/eyes.c"
        "ui/avatar.cpp"
        "ui/avatar_native.cpp"
        "ui/avatar_activity.cpp"
        "vendor/stopwatch_avatar/avatar_engine.cpp"
        "ui/avatar_timing.c"
        "hardware/tilt.c"
        "hardware/imu.c"
        "vendor/bmi270/bmi2.c"
        "vendor/bmi270/bmi270.c"
        "input/hid_input.c"
        "network/identity.c"
        "network/mcp.c"
        "app/device.c"
        "hardware/display.c"
        "hardware/touch.c"
        "ui/ui.c"
        "ui/ui_layout.c"
        "ui/ui_preferences.c"
    INCLUDE_DIRS "." "app" "hardware" "input" "network" "ui" REQUIRES esp_driver_gpio esp_driver_i2c esp_driver_spi esp_lcd
    esp_netif esp_timer lwip esp_http_server esp_https_server json mbedtls nvs_flash bootloader_support esp_wifi bt
    PRIV_REQUIRES esp_app_format esp_psram
    EMBED_TXTFILES "web/setup.html")
# Optimize only graphics/motion hot paths. Preserve input/USB/BLE timing flags.
set_source_files_properties("ui/avatar.cpp" "ui/avatar_native.cpp" "ui/avatar_activity.cpp"
    "vendor/stopwatch_avatar/avatar_engine.cpp" PROPERTIES COMPILE_OPTIONS "-O2")
# Keep the network source offer matched to the exact build inputs.
idf_build_get_property(stopwatch_python PYTHON)
execute_process(COMMAND "${stopwatch_python}" "${PROJECT_DIR}/tools/source_bundle.py"
    "${CMAKE_CURRENT_BINARY_DIR}/firmware-source.tar.gz" COMMAND_ERROR_IS_FATAL ANY)
file(GLOB_RECURSE stopwatch_source_inputs CONFIGURE_DEPENDS
    "${PROJECT_DIR}/main/*.c" "${PROJECT_DIR}/main/*.h" "${PROJECT_DIR}/main/*.cpp"
    "${PROJECT_DIR}/main/*.hpp" "${PROJECT_DIR}/main/*.md" "${PROJECT_DIR}/main/*.html"
    "${PROJECT_DIR}/main/*.yml" "${PROJECT_DIR}/licenses/*.txt")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${stopwatch_source_inputs}
    "${PROJECT_DIR}/SOURCE.md" "${PROJECT_DIR}/SOURCE.zh-CN.md"
    "${PROJECT_DIR}/NOTICE" "${PROJECT_DIR}/THIRD_PARTY.md" "${PROJECT_DIR}/THIRD_PARTY.zh-CN.md"
    "${PROJECT_DIR}/tools/source_bundle.py" "${PROJECT_DIR}/tools/generate_ui_font.py"
    "${PROJECT_DIR}/package.py" "${PROJECT_DIR}/build.sh" "${PROJECT_DIR}/flash.sh"
    "${PROJECT_DIR}/flash.py" "${PROJECT_DIR}/sdkconfig.defaults" "${PROJECT_DIR}/dependencies.lock"
    "${PROJECT_DIR}/partitions.csv" "${PROJECT_DIR}/LICENSE")
target_add_binary_data(${COMPONENT_LIB} "${CMAKE_CURRENT_BINARY_DIR}/firmware-source.tar.gz"
    BINARY RENAME_TO stopwatch_source)

# Preserve the pre-organization __FILE__ strings used by SDK assertions/errors.
# Optimization, source registration order and runtime configuration are retained.
foreach(stopwatch_group app hardware input network ui)
    target_compile_options(${COMPONENT_LIB} PRIVATE
        "-fmacro-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}/${stopwatch_group}/=./main/")
endforeach()
