# Firmware module

English · [简体中文](README.zh-CN.md)

Anan iPhone Use (`anan-iphone-use`): M5Stack StopWatch-specific ESP-IDF C firmware for device-side iPhone input execution. Clients own observation, inference, planning and user authorization; firmware runs no model and supplies no iPhone screenshots. Compilation/static results do not establish
phone, display, trust, latency or interoperability acceptance.

| Module | Responsibility |
| --- | --- |
| `board.c`, `bridge.h` | Existing SDA47/SCL48, M5PM1 0x6e reg0x07 bit5 MCU supply hold, GPIO1/2 active-low stop; export existing I2C bus for display |
| `usb.c` | USB NCM and retained absolute mouse/keyboard IDs; wheel sender and appended Consumer ID 3 |
| `main.c` | Board/USB/actions/MCP/UI initialization only; no ANAN1 TCP listener/parser/heartbeat |
| `actions.c/h` | Shared single worker/slot, original US ASCII mapping and HID report/pause sequence; caller guards and completion |
| `hid_input.c/h` | Strict bounded parsing and named keyboard/Consumer mappings for consolidated gestures/keys and legacy compatibility; progress units |
| `device.c/h` | Touch/physical fallback controls, raw button stop, MCP grant until stop/detach/reboot; separate touch/low-priority screen tasks |
| `mcp.c/h`, `mcp_tools.h` | Bounded HTTP/HTTPS MCP2025-11-25 adapter, seven advertised schemas and legacy compatibility calls, session ownership/ID ledger, async replies, USB-only setup/profile/config and read-only startup diagnostics |
| `identity.c/h` | Hardware-entropy-seeded DRBG, generated CA/leaf/token, dedicated NVS identity partition; explicit maintenance/reset |
| `display.c/h`, `panel_init.h` | CO5300 QSPI strip renderer, M5IOE1 wake/retry/settling and verified screen power/reset, own ASCII glyphs; attributed MIT vendor command data |
| `touch.c/h` | CST820B0x15 bounded native polling, GPIO13, expander PYG4 reset; read errors cancel gestures |
| `ui.c/h`, `ui_layout.c`, `ui_font.h` | Shared drawing/hit layout, Chinese/English labels, SIL OFL CJK glyph subset, currently displayed frame snapshot |
| `ui_preferences.c/h` | Language in a separate ui NVS namespace; no persisted control grants |
| `web/setup.html` | Bilingual phone enrollment, certificate instructions and HTTP-first JSON export, optional HTTPS; no agent/model code |

[MCP](../docs/mcp.md) documents the current sole control protocol and onboarding.
[PROTOCOL.md](../PROTOCOL.md) remains the unchanged historical ANAN1 contract;
the old service and source-selection UI are removed. Boot mode is MCP, without
automatic control authorization. No command, control grant, request
or connection is persisted. Generated TLS/pairing identity and a separate language preference are persisted.
Mouse/keyboard IDs and legacy timing are retained; Consumer ID 3 is appended. MCU hold/5V boost behavior is retained; screen
power is now initialized separately. No Wi-Fi/BLE, PSRAM allocation,
screenshot service, model provider or copied [Anan](https://anan.sh) client is included.

Build from the root using pinned ESP-IDF5.5.5 and `./build.sh`; dependencies and
exact versions are in [manifest](idf_component.yml), [lock](../dependencies.lock)
and [notices](../THIRD_PARTY.md). Generated outputs/logs/credentials stay ignored.

[Touch UI](../docs/touch-ui.md) records hardware/interaction and predefined checks.
Default Chinese, Settings can select English, and language is remembered. The
phone page also has a language switch. Physical fallback: yellow selects, blue
confirms; hold blue for two seconds on the reset confirmation. Stop consumes
the contact/press and does not reauthorize. `tools/` generates glyphs/static previews and runs the pre-implementation offline HID scenarios; none is a device/simulator/E2E or runtime dependency. See [extended tools](../docs/hid-tools.md).
