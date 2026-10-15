# Firmware module

English · [简体中文](README.zh-CN.md)

One ESP-IDF component for M5Stack StopWatch. `main.c` owns initialization;
`bridge.h` retains the existing shared board/input declarations. Subdirectories
organize files without adding components, tasks or runtime abstractions.

| Directory | Modules | Responsibility |
| --- | --- | --- |
| [app/](app/) | `device.c/h, reset_trace.c/h` | Device state, physical control grants, stop and boot-latched reset diagnostics. |
| [hardware/](hardware/) | `board.c, battery.c/h, display.c/h, panel_init.h, touch.c/h, imu.c/h, tilt.c/h` | StopWatch power/pins, read-only battery estimate, panel/touch drivers and BMI270 motion sampling. |
| [input/](input/) | `actions.c/h, hid_input.c/h, input.c, input_backends.h, usb.c, ble_input.c, ble_reports.c/h` | Single HID worker, bounded action parsing, explicit USB/BLE routing and report delivery. |
| [network/](network/) | `mcp.c/h, mcp_tools.h, identity.c/h, wireless.c/h, network_store.c/h, network_authority.c/h, discovery.c/h, wifi_qr.c/h` | HTTP/HTTPS MCP, sessions/pairing, identity, Wi-Fi/hotspot setup, network storage, authority checks, mDNS and QR payloads. |
| [ui/](ui/) | `ui.c/h, ui_layout.c, ui_font.h, ui_preferences.c/h, eyes.c/h, avatar.cpp/h, avatar_native.cpp/hpp, avatar_activity.cpp/hpp, avatar_timing.c/h` | Bilingual page layout/preferences, input-feedback events, native avatar rendering, MCP activity props and timing diagnostics. |
| [vendor/](vendor/) | `stopwatch_avatar/, bmi270/, qrcodegen.c/h` | Pinned third-party engine, sensor API and QR encoder; preserve origins and file-specific notices. |
| [web/](web/) | `setup.html` | Embedded bilingual pairing, Wi-Fi configuration and MCP export page; no agent/model code. |

Clients own observation, reasoning, planning and user authorization. Firmware runs
no model and supplies no phone screenshots. USB/BLE selection is explicit, without
fallback. Commands, requests and control grants are not persisted; stop, disconnect
or reboot never restores control or replays input. Physical stop is independent
of display, I2C and web availability.

The control face uses the AGPL-3.0-or-later engine from
**[Trentct/m5stack-stopwatch-avatar](https://github.com/Trentct/m5stack-stopwatch-avatar)**.
See [origins/modifications](vendor/stopwatch_avatar/ORIGIN.md) and
[responsive activity](../docs/history/avatar-responsive.md) for the native port and MCP feedback.
`eyes.c` still supplies input events and the older fallback renderer; directory
organization does not remove or change either path.

Current SDK defaults leave PSRAM disabled. Compact avatar caches and reset
diagnostics are described in [network memory history](../docs/history/network-startup.md).
The user reports the provisioning fault resolved; endurance and reconnect timing
remain separate validation work. Compilation/offline checks are not device acceptance.

Build with `./build.sh` from the root. [Repository layout](../docs/repository.md)
explains directories and commands; the [tools index](../tools/README.md) lists
existing offline checks. [MCP](../docs/mcp.md) and [wireless setup](../docs/wireless.md)
describe current behavior; candidate history and staged evidence remain in
[validation](../docs/validation.md). [Notices](../THIRD_PARTY.md) and the
[source offer](../SOURCE.md) retain applicable terms.
