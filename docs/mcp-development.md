# MCP development decisions

English · [简体中文](mcp-development.zh-CN.md)

The user authorized MCP first, wireless later, simple button/option setup UI,
no Anan code changes, and accepted initial iPhone certificate installation/trust.
They subsequently instructed implementation. Code is now implemented on the
MCP development branch; no device/interoperability acceptance is claimed.
Checkpoint `4c6015c` remains recoverable and its provenance/runtime gaps remain.

## Original first-slice decisions (historical)

- MCP2025-11-25, tools-only Streamable HTTP JSON POST over USB HTTPS8443,
  with GET405, cancellation and session termination. Native IDF HTTP/HTTPS,
  cJSON and mbedTLS APIs; the full MCP C SDK/agent runtime is not imported.
- Six tools reuse original HID execution. Default boot remains legacy USB;
  physical MCP selection/authorization until stop/detach/reboot and one session owner.
- Phone HTTP enrollment with a device-confirmed code, per-device CA/leaf/token,
  normal iPhone trust, HTTPS-only token export and a dedicated identity partition.
  No shared signing key or public relay. Ordinary upgrade retains identity.
- Native CO5300/button state UI, pinned driver2.2.0, attributed MIT vendor
  init commands and own glyphs. No LVGL, touch or model-thinking display.
- Original ANAN1 contract/USB descriptors/report timing remain authoritative.
  No source-origin or project-license change. New notices are recorded separately.

The inspected Anan HTTPS guard and Swift MCP SDK0.12.1 support this selected
version/JSON response shape; GET405 behavior is source evidence only. Its native
`phone.observe` is separate from board MCP tools and retains USB/capture/ownership
gates. Idle ANAN1 READY/PING is retained in MCP mode, with old input rejected.
The user deferred further observation work. Neither a board screenshot service
nor wireless Anan observation is claimed. Full implementation details, limits,
button transitions and onboarding are in [device MCP](mcp.md).

## Wireless deferred

The candidate route is Wi-Fi STA2.4GHz for MCP plus BLE HID for input, retaining
USB. Phone setup can collect credentials; StopWatch uses choices/buttons.
The first feasibility gate is iPhone acceptance of the needed BLE absolute
pointer: IDF's relative mouse example is insufficient evidence. Wi-Fi/BLE,
battery, TLS naming and reconnect/no-replay need a separate implementation and
scoped device acceptance. Do not treat this MCP slice as wireless support.

## Checks defined before implementation

| Scenario | Required evidence |
| --- | --- |
| Static/build | Pinned IDF build/package, RAM/Flash report, dependency/source notices, no secrets in source/image inputs |
| Discovery | Actual initialize/version/initialized/tools-list sequence with a compatible independent client; separately check unchanged Anan |
| Input | Synthetic portrait note: one tap, drag, key and ASCII input; observe visible results and preserve failures |
| Stop | Button/stop/cancellation/detach during input; bounded measured latency and released reports where host remains available |
| Arbitration | Simultaneous legacy/MCP attempts; one executor, no duplicate input, no implicit mode switch or replay |
| Malformed requests | Length/nesting/type/version/Origin/auth limits reject before dispatch; no command details or credentials in diagnostics |
| Enrollment | Physical confirmation, profile trust and successful normal TLS; old credentials cease working after explicit reset |
| Legacy regression | Current ANAN1 greeting, heartbeat, ID ordering, report timing, stop and no-replay behavior on default USB route |

Device, live model, simulator and E2E execution require a separately approved
scope. No unit tests are to be added after implementation. Until observed, do
not mark discovery/interoperability, display, stop latency or hardware acceptance
as passed. Wireless comes after the MCP slice; BLE absolute-pointer compatibility
remains its first input feasibility gate.

## Sources inspected

- [MCP 2025-11-25 transport](https://modelcontextprotocol.io/specification/2025-11-25/basic/transports), [lifecycle](https://modelcontextprotocol.io/specification/2025-11-25/basic/lifecycle).
- [Swift SDK 0.12.1](https://github.com/modelcontextprotocol/swift-sdk/tree/a0ae212ebf6eab5f754c3129608bc5557637e605); Anan local MCP/Phone sources were read, not modified.
- [ESP-IDF v5.5.5 HTTPS](https://docs.espressif.com/projects/esp-idf/en/v5.5.5/esp32s3/api-reference/protocols/esp_https_server.html), [ESP32-S3 entropy](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32s3/api-reference/system/random.html); pinned SDK headers were also inspected.
- [Apple certificate profile trust](https://support.apple.com/en-us/102390), [TLS certificate requirements](https://support.apple.com/en-us/103769).
- [Espressif MCP SDK reference](https://github.com/espressif/esp-iot-solution/tree/58c2f852125cb1606c2137fc9b697efc3ce1c3d0/components/mcp-c-sdk): manifest 2.0.1, IDF ≥5.4, Apache-2.0; inspected, not imported.
- [StopWatch hardware](https://docs.m5stack.com/en/core/StopWatch), [factory display/IO reference](https://github.com/m5stack/M5StopWatch-UserDemo/tree/6b4aa125288b6fe9dca661f10159f6e1e5ee785c/main/hal), [M5IOE1 definitions](https://github.com/m5stack/M5IOE1/tree/846eec7d05e25c09013be2acdb8804487f48a62e): source files declare MIT; only the attributed panel command table is adapted for development.
- [CO5300 driver](https://components.espressif.com/components/espressif/esp_lcd_co5300/versions/2.2.0/readme).

## HTTP onboarding follow-up: agreed scope and checks

The user chose plain HTTP for the current USB connection, asked to wait until
Anan supported it, and now reports that client support is ready. Implement the
existing MCP handler at HTTP80 `/mcp`, export the HTTP URL from physically
approved `/config`, and let the same page complete pairing and import without
certificate installation. Retain optional HTTPS8443, per-device identity/token,
USB-only scope, the six tools, control/stop/session behavior and ANAN1. Do not
modify Anan, remove old source/license work or add wireless transport.

Before code, define compilation/package, source-invariant review, both-language
static UI preview bounds/glyphs, embedded JavaScript/config JSON and local-link
checks. Do not add post-implementation unit tests or execute device input. The
user will flash, remove only the old StopWatch CA profile/configuration, reset
board pairing and repeat the first-user flow. Record that separately from static
checks; do not erase the whole Flash or unrelated phone settings.

## MCP-only follow-up: approved scope and checks

The user reports the HTTP first-user flow works well and explicitly asks to
remove the old TCP route, retaining only MCP. Remove port28766, the ANAN1 parser,
READY/PING/STOP logic and legacy input ownership. Boot into MCP, remove source
selection buttons, keep USB NCM/HID, pairing, the six tools, per-device identity,
language preference, volatile grants, input timing and immediate stop. Do not
modify Anan. The inspected observation readiness allows capture without an
input connection; old Anan direct input will no longer connect.

Before implementation, define compile/package, source and linked-symbol review
for the retired server, preserved USB/action/identity hashes, bilingual static
UI glyph/layout previews, JSON/JavaScript/Python syntax and documentation links.
Retain PROTOCOL.md and checkpoint/old build evidence as historical records,
clearly distinguish them from current MCP instructions. Do not add unit tests
after implementation or perform device/phone actions. The new image needs a
separate user reflash/acceptance; no publication or license change is implied.
