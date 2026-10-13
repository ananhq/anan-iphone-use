# Firmware module

English · [简体中文](README.zh-CN.md)

Implemented StopWatch-specific ESP-IDF C module. `board.c` configures I²C SDA 47 / SCL 48, M5PM1 0x6e register 0x07 bit 5 and GPIO 1/2 active-low buttons. It holds the MCU LDO; it does not enable external 5V boost. Native USB uses GPIO 19/20.

`usb.c` implements one full-speed NCM+HID composite, lwIP IPv4 netif, bounded synchronous Ethernet sends, absolute mouse and keyboard reports. `bridge.h` exposes board initialization/stop checks, USB initialization/online checks, mouse/key/report release; the single action worker owns report submission. `main.c` owns TCP port 28766, parsing, the volatile generation and heartbeat, one-slot queue and worker.

[PROTOCOL.md](../PROTOCOL.md) owns all wire/action/terminal rules. No command, permission or connection state is persisted. Dependencies are [idf_component.yml](idf_component.yml), [lock](../dependencies.lock) and built-in ESP-IDF GPIO/I²C/netif/timer/lwIP. Build from the repository root with `./build.sh`; see [validation](../docs/validation.md) and [provenance](../THIRD_PARTY.md).

No display, touchscreen, Wi-Fi/BLE, PSRAM allocation, screenshot service or MCP. Physical release is best effort after detach. The static source/build evidence is distinct from device acceptance.
