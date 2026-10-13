# Background and design

English · [简体中文](design.zh-CN.md)

## Why this device

The goal is an on-phone agent that can act outside its own app with a small wired peripheral. StopWatch offers native ESP32-S3 USB, power management and two stop buttons. Its screen can later communicate device state, but display/touch drivers are not initialized today. It is not a USB touchscreen implementation: the phone receives an absolute HID **mouse**, with AssistiveTouch.

```text
phone screen → phone capture/client → model
model → client → USB NCM / TCP → StopWatch → HID → phone
```

| Owner | Current responsibility |
| --- | --- |
| Firmware | USB descriptors, fixed local network, command parsing, HID reports, heartbeat, generation ownership and stop |
| Phone client | Screen capture, model networking, user control permission, coordinate normalization, result observation and chat |
| Model | Interpret the supplied screenshot and select actions; must observe actual results |

`DONE` proves neither a target hit nor task completion. The firmware does not know the app, visible controls, thinking state or full task progress. No cloud service or model credentials live on the board.

## Source organization

[main module](../main/README.md): `board.c` configures power/buttons, `usb.c` builds USB/NCM/HID, `main.c` owns the single action worker/TCP parser. `build.sh` calls pinned IDF then `package.py`; `flash.sh` is the only device-writing entrypoint. Dependencies are registry-managed and locked, not vendored. Outputs keep their historical `anan-stopwatch-*` names for traceability.

## References and differences

- [actell at the inspected commit](https://github.com/ja1chen/actell/tree/e701786d1686823f9ab60524e689e387b7c3b904): its retained `firmware-device` route combines phone-side capture with NCM/HID. Its later CoreDevice route has different device/protocol/pairing responsibilities and is not included here. Our USB networking code has substantial similar structure; its generation history remains unresolved, as recorded in [provenance](../THIRD_PARTY.md).
- [esp32-iphone-agent at the inspected commit](https://github.com/onehumanbeing/esp32-iphone-agent/tree/68584cfc8b3f227d6324ec7c8008c48b53f20a18): a computer-side AirPlay/Python agent sends Wi-Fi actions to Arduino firmware. Here the agent/capture are on the phone, commands use USB Ethernet, and the firmware uses ESP-IDF.
- [M5Stack documentation](https://docs.m5stack.com/en/core/StopWatch) supplies board and download-mode information. It describes a touch AMOLED board; no graphics or touch implementation is migrated.

Anan's background capture/stream scheduling and Live Activity remain in Anan. Moving firmware does not grant a phone client continuous background execution or make arbitrary agents network-reachable. The migration changes no prompt/history/cache prefix.
