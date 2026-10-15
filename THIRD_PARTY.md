# Provenance and third-party notices

English · [简体中文](THIRD_PARTY.zh-CN.md)

Anan iPhone Use (`anan-iphone-use`) is the chosen project identity. The user confirms that actell source was referenced; exact USB file derivation remains unresolved. The maintainer authorized an open-source license choice; the combined project now adopts GPL-3.0-only in [LICENSE](LICENSE), compatible with the recorded actell reference. Existing per-file declarations and third-party terms are retained, with the prior root Apache-2.0 text preserved in [the legacy notice](licenses/Legacy-first-party-Apache-2.0.txt). See the [license decision and source handling](docs/license-plan.md). This decision does not complete the derivation audit or relicense dependencies.

## Migration scope and evidence

The first-party tree was extracted from Anan's **untracked** `hardware/stopwatch/` on 2026-10-13. [source-snapshot.json](artifacts/migration/source-snapshot.json) records Anan HEAD, every original file hash/status, the historical image and its source manifest. Anan HEAD alone does not identify these uncommitted files. Migration preserves existing declarations; it does not retroactively establish how each file was generated.

| Original files | Declaration / source handling |
| --- | --- |
| `main/main.c`, `main/bridge.h` | Existing Apache-2.0 SPDX; ANAN1 parser/worker/API preserved byte-for-byte. No generation transcript audited; independent provenance not guaranteed |
| `main/board.c` | Existing Apache-2.0 SPDX; StopWatch pin/power facts checked against M5Stack documentation/schematic. Exact origin of the register implementation remains unverified |
| `main/usb.c` | Existing Apache-2.0 SPDX; preserved byte-for-byte; substantial structural overlap with actell files listed below. Copy/translation/adaptation versus API/common-pattern origin remains unresolved |
| `main/CMakeLists.txt`, `main/idf_component.yml`, `CMakeLists.txt`, `sdkconfig.defaults`, `dependencies.lock` | Other build/config inputs retained byte-for-byte; root CMake additionally pins historical IDF app version `ec6bb29-dirty` instead of deriving it from the new Git tree; dependency hashes/version recorded |
| `build.sh`, `flash.sh`, `package.py`, `.gitignore` | Original scripts/ignore file retained; `.gitignore` adds local editor/cache exclusions. No external implementation-origin audit completed |
| `README.md`, `README.zh-CN.md`, `PROTOCOL.md` | Original hashes retained in snapshot; rewritten documentation/path/provenance wording. Protocol behavior unchanged |
| Newly added documentation/records | Authored for this migration; source facts and upstream links identified; no upstream code added |

## Inspected references

The migration table retains original paths. Directory organization now places
`board.c` / `panel_init.h` under `main/hardware/` and USB/HID code under
`main/input/`, with no implementation/header byte changes. Third-party engine,
sensor and QR source locations remain unchanged; see the [module index](main/README.md).

**actell**, commit `e701786d1686823f9ab60524e689e387b7c3b904`:
[repository](https://github.com/ja1chen/actell/tree/e701786d1686823f9ab60524e689e387b7c3b904),
[THIRD_PARTY](https://github.com/ja1chen/actell/blob/e701786d1686823f9ab60524e689e387b7c3b904/THIRD_PARTY.md),
[USB network](https://github.com/ja1chen/actell/blob/e701786d1686823f9ab60524e689e387b7c3b904/firmware-device/main/usb_network.c),
[USB HID](https://github.com/ja1chen/actell/blob/e701786d1686823f9ab60524e689e387b7c3b904/firmware-device/main/usb_hid.c).
The inspected distribution declares GPL-3.0-only, with file-specific terms. Its original v0.1/new independent file headers retain GPL-3.0-or-later and its raw mux module retains GPL-2.0-only OR GPL-3.0-only; those modules are not newly imported here. Retain the actual applicable terms when mapping derivation. The two USB files inspected have no individual SPDX header, which is not permission to relicense them. Similarities include the pbuf-copy/synchronous-send/free path with 50ms timeout and 1514-byte bound, lwIP netif/start sequence, 2s semaphore initialization, eFuse-derived host-MAC bit flip, NCM endpoint layout, HID report IDs/descriptors and USB event/config callbacks. Anan differs in action worker, power/buttons, protocol and several USB details; those differences do not resolve implementation origin. No actell files are newly copied by this migration. Do not publish an all-independent guarantee. Map actual derivative reuse and retain its applicable license/source obligations before distribution; GPL-3.0-only is now the combined-project license. Credit alone does not resolve the file-origin map.

**esp32-iphone-agent**, commit `68584cfc8b3f227d6324ec7c8008c48b53f20a18`:
[repository](https://github.com/onehumanbeing/esp32-iphone-agent/tree/68584cfc8b3f227d6324ec7c8008c48b53f20a18),
[MIT license](https://github.com/onehumanbeing/esp32-iphone-agent/blob/68584cfc8b3f227d6324ec7c8008c48b53f20a18/LICENSE), Copyright 2026 Henry Yuan.
Referenced for external USB HID/agent architecture. Its Arduino/Wi-Fi/computer-side agent is not a build dependency; no files are newly imported. Historical copy/adaptation has not been exhaustively audited. The [complete upstream MIT notice](licenses/esp32-iphone-agent-MIT.txt) is retained, including Copyright (c) 2026 Henry Yuan; historical file reuse still needs review.

**M5Stack**: [StopWatch documentation](https://docs.m5stack.com/en/core/StopWatch) and linked schematic; board/download-mode facts, not an imported vendor library or artwork. `M5PM1` register behavior was not proven solely by the product page; retain the original board implementation and verify its exact code/reference source before an origin guarantee. Vendor documents are not granted this repository's license.

The withdrawn 0.9.7 PSRAM configuration follows the documented ESP32-S3R8/8MB hardware and Octal
80MHz values in the factory [sdkconfig.defaults](https://github.com/m5stack/M5StopWatch-UserDemo/blob/6b4aa125288b6fe9dca661f10159f6e1e5ee785c/sdkconfig.defaults),
commit `6b4aa125288b6fe9dca661f10159f6e1e5ee785c`. These are configuration facts;
no factory implementation is imported. The allocator/PSRAM implementation is
the existing pinned ESP-IDF5.5.5, under its retained notices.

## Build dependencies

| Dependency | Pinned source/version | Notice included |
| --- | --- | --- |
| ESP-IDF | v5.5.5, `b774170ff46c393eeb5e495ea37936038d3f4f4f` | [Apache-2.0](licenses/ESP-IDF-Apache-2.0.txt); component-specific notices also apply |
| esp_tinyusb | registry 2.2.0, hash in dependencies.lock | [Apache-2.0](licenses/esp_tinyusb-Apache-2.0.txt) |
| TinyUSB | registry 0.21.0~2, hash in dependencies.lock | [MIT](licenses/TinyUSB-MIT.txt), Copyright 2012–2026 hathach |
| lwIP | IDF submodule `fd432e4ee2cfb7f7f1c7eb7227e0173412e7b84e` | [BSD notice](licenses/lwIP-BSD.txt), Swedish Institute of Computer Science |

Sources are fetched separately through IDF/registry, not vendored here. These license copies do not constitute an exhaustive binary-license inventory for every linked SDK component. Before publishing a binary, inspect its exact linked components and retain additional applicable copyright/license notices with the release. Keep [LICENSE](LICENSE) and [NOTICE](NOTICE), record the final repository address/version, and resolve outstanding first-party provenance. No dependency upgrade or project license change was part of the original migration; the later GPL-3.0-only decision is recorded separately above.

## MCP development additions

The checkpoint table above describes migration history. MCP development now
extracts the original action routines into `main/input/actions.c`, adapts `main/main.c`
for arbitration and adds an I2C bus accessor to `board.c`; `usb.c` and PROTOCOL.md
remain unchanged. The old USB/board provenance gaps remain unresolved.

New MCP, identity, button UI, strip renderer, schemas and phone setup page are
first-party implementations using the pinned SDK APIs; neither Anan client nor
Espressif MCP C SDK code is imported. ASCII glyphs are authored here; the later CJK subset is derived under OFL as recorded below. The one adapted
vendor data file is `main/hardware/panel_init.h`: panel commands from M5Stack
[M5StopWatch-UserDemo main/hal/hal_display.cpp](https://github.com/m5stack/M5StopWatch-UserDemo/blob/6b4aa125288b6fe9dca661f10159f6e1e5ee785c/main/hal/hal_display.cpp),
commit `6b4aa125288b6fe9dca661f10159f6e1e5ee785c`; MIT, Copyright (c) 2026
M5Stack Technology CO LTD, [complete notice](licenses/M5Stack-demo-MIT.txt).
The QSPI gap and IO-expander pin facts use the same factory reference and
[M5IOE1 definitions](https://github.com/m5stack/M5IOE1/tree/846eec7d05e25c09013be2acdb8804487f48a62e)
(commit `846eec7d05e25c09013be2acdb8804487f48a62e`), not a copied vendor UI/library.

| Added use | Exact source | Notice |
| --- | --- | --- |
| CO5300 display driver | Registry `espressif/esp_lcd_co5300` 2.2.0; component hash in lock | [Apache-2.0](licenses/esp_lcd_co5300-Apache-2.0.txt), Espressif file notices retained in fetched source |
| Build helper | Registry `espressif/cmake_utilities` 0.5.3; transitive lock | [Apache-2.0](licenses/cmake_utilities-Apache-2.0.txt) |
| JSON | IDF cJSON1.7.19, `b2890c8d76bbb64e710585ebc0a917196b9c67e7` | [MIT](licenses/cJSON-MIT.txt), Dave Gamble and contributors |
| TLS/X.509/DRBG | IDF mbedTLS3.6.6, `9d669eadb1955d348986b9280156710aaaadf79f` | Apache-2.0 alternative selected; [complete upstream dual-license text](licenses/mbedTLS-dual-license.txt) preserved |
| HTTP/HTTPS/NVS/SPI/LCD/entropy | Built-in pinned ESP-IDF5.5.5 | Existing IDF notice plus applicable component/file notices |

The original MCP additions did not change the project license or resolve provenance; the later overall GPL-3.0-only decision leaves the source gaps open. Generated
private keys/certificates/tokens and pairing records are never committed. Before
binary publication, complete the linked-component notice inventory as above.

Display startup correction: `main/hardware/display.c` now independently implements
M5IOE1 wake/retry/settling and disables expander I2C sleep before screen power.
Behavior/register facts were checked against `src/M5IOE1.cpp`,
`src/M5IOE1_i2c_compat.h` and `src/M5IOE1.h` at the pinned M5IOE1 commit above,
and factory `main/hal/hal_ioe.cpp` at the pinned demo commit. The vendor library
is not imported. Retain the [complete M5IOE1 MIT notice](licenses/M5IOE1-MIT.txt),
Copyright (c) 2026 M5Stack Technology CO LTD. No original USB/board provenance
gap is cleared by this correction.

## Touch/bilingual UI additions

`main/hardware/touch.c` is a bounded first-party IDF adapter based on CST820B register and
reset facts from the pinned factory `main/hal/drivers/cst820/cst820.cpp/.h` and
`hal_display.cpp` / `hal_ioe.cpp`. CST source files lack individual SPDX headers;
the inspected root MIT license applies, and its complete M5Stack notice remains
included. No vendor C++ class, LVGL or M5GFX implementation is copied/imported.

`main/ui/ui_font.h` is generated **StopWatch UI Glyphs**, a renamed 24px bitmap
subset derived from Noto Sans SC's 22px weight500 rasterization. Exact font:
google/fonts commit `a85815a42757630ce188fdad368c2dfc444d4773`,
`ofl/notosanssc/NotoSansSC[wght].ttf`, Git blob
`fb0637bafbcd804fe32152370a1225990745b4bc`, SHA-256
`a3041811a78c361b1de50f953c805e0244951c21c5bd412f7232ef0d899af0da`.
The 17,772,300-byte source font stays outside the repository/firmware; only UI
glyphs are included. [Complete SIL OFL1.1](licenses/NotoSansSC-OFL-1.1.txt),
Copyright 2014–2021 Adobe, Reserved Font Name 'Source'. The generated glyph data
is OFL-1.1, not relicensed as Apache-2.0. Own ASCII glyphs/renderer/layout/gesture
code retain their first-party declarations. These file-specific licenses and provenance gaps
remain after the overall GPL-3.0-only decision.

`tools/generate_ui_font.py` and `tools/render_ui.py` use Pillow12.3.0 only for
build-time glyph generation/static document previews, not a linked firmware
component. Rendering uses Pillow's FreeType support; source font hash is checked,
and layout previews export the same first-party C widget layout. Preserve
applicable toolchain notices if redistributing those development tools.

## HID expansion additions

Candidate `0.5.0-mcp` adds first-party bounded parsing/execution, schemas and offline acceptance fixtures. The fixtures were authored before implementation and simulate reports/time, not device acceptance. USB code now uses the existing locked TinyUSB `0.21.0~2` Consumer descriptor/API; no upstream implementation was copied or dependency upgraded. Numeric Consumer usages follow Apple Accessory Design Guidelines 2026-09-21, chapter14 table14-3 (including 0x029D, absent as a named enum in this SDK); [research](docs/research/hid-capabilities.md) records the PDF hash. Existing original USB/board provenance gaps and license notices remain. Mouse/keyboard IDs and USB configuration are retained with Consumer ID3 appended.

Candidate `0.5.1-mcp` consolidates tool discovery and first-party parsing, retaining the executor and USB code. The added consolidation acceptance scenarios preceded this implementation. No external code, dependencies or licenses changed; prior provenance gaps remain.

## Wireless candidate additions

Candidate `0.6.0-wireless` activates components within the existing ESP-IDF
v5.5.5 lock; no SDK upgrade, Arduino library, upstream application code or
vendor HID demo is imported. `input.c`, `ble_input.c`, `ble_reports.c/h` and
`wireless.c/h` are first-party implementations using SDK APIs. BLE reports use
the already pinned TinyUSB descriptor macros; USB descriptors/sender bodies are
retained with backend symbol renaming. No project license change.

| Activated component | Exact source | Notices |
| --- | --- | --- |
| NimBLE | IDF submodule `685675c0128deafdd201c9eb82e61d227364646c` | [complete LICENSE](licenses/NimBLE-LICENSE.txt), [NOTICE](licenses/NimBLE-NOTICE.txt), [BSD notices](licenses/NimBLE-BSD-NOTICES.txt) |
| TinyCrypt / micro-ecc | NimBLE `ext/tinycrypt`, and IDF `components/bt/common/tinycrypt` at the pinned SDK commit | [complete BSD notice](licenses/TinyCrypt-BSD.txt); linked-file headers retain their terms |
| Wi-Fi radio | `22bb3517b08760ed43b889013f8a122db0d46da0` | [Apache-2.0](licenses/ESP-IDF-Apache-2.0.txt) |
| PHY | `59c1234e929212aec0fdda75769b759951235536` | [Apache-2.0](licenses/ESP-IDF-Apache-2.0.txt) |
| Coexistence | `79e618f033e926d134decc6d95bb1b1850bd032c` | [Apache-2.0](licenses/ESP-IDF-Apache-2.0.txt) |
| S3 Bluetooth controller (c3-family library) | `0a08c4b32f3666003080b662a1a61794da24ff0f` | [Apache-2.0](licenses/ESP-IDF-Apache-2.0.txt) |

All four radio LICENSE files match the already retained Apache-2.0 file
(SHA-256 `cfc7749b96f63bd31c3c42b5c471bf756814053e847c10f3eb003417bc523d30`).
The SDK's radio libraries are precompiled components; this is not an assertion
that their implementation sources are available. NimBLE LICENSE lists additional
platform-specific material; the S3 binary is not a Nordic/babblesim target.
Binary publication still needs its final linked-file notice inventory.

[xhoantran/iphone-use](https://github.com/xhoantran/iphone-use/tree/bc0fe36e78ec5a3af095c813574e3a8b7bbea4dd)
was consulted for BLE absolute positioning followed by relative-button input.
No Swift files or report bytes were copied. No AirPlay/client code is added.
Existing USB provenance gaps are unchanged. See [wireless scope](docs/history/wireless-development.md).

The `0.6.1-wifi-scan` slice adds first-party scan orchestration and setup-page
selection using the same pinned `esp_wifi.h` API. No source files are imported,
and dependencies, report descriptors and licenses are unchanged.

Candidate `0.7.0-wireless-setup` adds first-party network-storage/HTTP-authority/discovery adapters. CMake activates `src/apps/mdns/mdns.c`, `mdns_out.c` and `mdns_domain.c` from the already pinned lwIP submodule `fd432e4ee2cfb7f7f1c7eb7227e0173412e7b84e`, unchanged and not vendored. Their Copyright (c) 2015 Verisure Innovation AB BSD headers are preserved in the [complete mDNS notice](licenses/lwIP-mDNS-BSD.txt), alongside the existing [lwIP notice](licenses/lwIP-BSD.txt). The global responder definition and SDK client-data slot preserve a consistent netif ABI. No new dependency, SDK upgrade, upstream application import or license change. SoftAP/DHCP use the existing pinned SDK components.

Candidate `0.7.1-mdns` compiles an attributed build-generated adaptation of the same pinned `src/apps/mdns/mdns.c` (original SHA-256 `2bd12a3bb8b3c8830c56c2b5c3b8073ab5f71654171602210f63cf6dafc8d5e9`). Root CMake checks the exact hash and changes only the IPv6 probe condition: skip that family when link-local address slot 0 is invalid, retaining IPv4 probing/conflict checks. The SDK checkout remains unchanged; generated source stays in ignored build/. The original Verisure headers and [complete BSD notice](licenses/lwIP-mDNS-BSD.txt) are retained. No SDK/dependency upgrade or project license change.

The hotspot QR slice vendors only Nayuki `c/qrcodegen.c` and `c/qrcodegen.h`, unmodified, from **v1.8.0**, commit `7ad95cedd8464a87f82221283612732ae4f3f305`, [upstream](https://github.com/nayuki/QR-Code-generator/tree/7ad95cedd8464a87f82221283612732ae4f3f305/c), under MIT, Copyright (c) Project Nayuki. Their complete license headers and the [MIT notice](licenses/Nayuki-QR-MIT.txt) are retained; first-party adapters/rendering are separate. Source SHA-256: C `300eff07ee25baaa7578f20284411638154716379437391e7e689c0e6ce81403`, header `e82df4bff37d18b5863b9e7486fe6bda1b6cda8c3b9ecebfec473907265cb589`. No SDK upgrade or project license change. ZXing-C++ 2.3.0 is an optional independent offline decoder used only for synthetic validation, not linked or vendored.

## Large-target UI and battery indicator

Candidate `0.7.6-touch-ui` adds first-party menu/layout/touch feedback and a coarse
voltage estimate. M5PM1 register facts (VBAT 0x22/0x23, little-endian mV) were
checked against `docs/M5PM1_Datasheet_EN.pdf` and `src/M5PM1.h/.cpp` at
[M5PM1 commit be9a5456c007c333e7ac963f33bfde1ffa5d82ee](https://github.com/m5stack/M5PM1/tree/be9a5456c007c333e7ac963f33bfde1ffa5d82ee).
No vendor library/class/percentage algorithm is copied or linked; the
[complete M5PM1 MIT notice](licenses/M5PM1-MIT.txt), Copyright (c) 2025 M5Stack
Technology CO LTD, is retained. The same pinned OFL glyph subset is regenerated
for the new labels; runtime dependencies and power policy are unchanged.
Existing original board/USB provenance gaps remain.

The `0.7.8-eyes` sampler, ellipse strip painter and offline previews are first-party
procedural geometry, without imported artwork, animation libraries or runtime
dependencies. Existing font notices and provenance gaps remain unchanged.


## Interactive avatar / BMI270

Design reference: [Trentct/m5stack-stopwatch-avatar](https://github.com/Trentct/m5stack-stopwatch-avatar/tree/204963257cb4dc2f3d7501eff900897bac55ef82), AGPL-3.0-or-later. Consulted for general pose/interaction/transition/dirty-region ideas; no code, expression tables or artwork imported. First-party native implementation and existing project license retained.

New sensor source: [Bosch BMI270 SensorAPI](https://github.com/boschsensortec/BMI270_SensorAPI/tree/41129fcfe39c583ee5462d79195741945d51c1fe), commit `41129fcfe39c583ee5462d79195741945d51c1fe`, base BMI270 v2.86.1. The five files below are copied byte-for-byte, including the configuration array, with original file notices; [full BSD-3-Clause notice](licenses/Bosch-BMI270-BSD-3-Clause.txt), Copyright (c) 2023 Bosch Sensortec GmbH. First-party ESP-IDF adapter/filter are separate.

| File | Original SHA-256 |
| --- | --- |
| `main/vendor/bmi270/bmi2.c` | `8d42ba3281b8448f674a4df616a7e176a259ad3c38c53b6d2c49bfdbdb0a82db` |
| `main/vendor/bmi270/bmi2.h` | `a86c53c480974a0c56e24d9d4e736445c61ebc07addec6d3eaceddb8a2c97cd3` |
| `main/vendor/bmi270/bmi2_defs.h` | `39c2cf19bfbf735cb0ed209efd975a731be50dff71b432b24a6a2625b2b14304` |
| `main/vendor/bmi270/bmi270.c` | `f6446867378bbcc90ea97ef6593555de8767ca20b799355c1b37c2cabb7b7b41` |
| `main/vendor/bmi270/bmi270.h` | `7979ec4c943e2825351bd9d3d267b63abc6f8314c1f13f1c37e95cabc3fc82c8` |

## Upstream avatar import (0.9.0)

The maintainer approved the AGPL-3.0-or-later import on 2026-10-15. This
supersedes the 0.8.x inspiration-only statement for the new control face.
Two engine files from Trentct/m5stack-stopwatch-avatar at
`204963257cb4dc2f3d7501eff900897bac55ef82` are adapted in
`main/vendor/stopwatch_avatar/`; [exact origins, original hashes and modifications](main/vendor/stopwatch_avatar/ORIGIN.md)
and [upstream notices](main/vendor/stopwatch_avatar/UPSTREAM-NOTICES.md) are retained.
The [full original AGPL text](licenses/StopWatch-avatar-AGPL-3.0.txt) is unchanged.
The native renderer/input adapter is AGPL-3.0-or-later. Original GPL code keeps
its terms; the combination follows GPLv3/AGPLv3 section 13, including the network
source offer at `/source`. See [source/build information](SOURCE.md). No upstream
Arduino/M5Unified/M5GFX hardware stack is linked. Existing provenance gaps and
linked SDK notice-review limits remain explicit.

The approved 0.9.2 responsive follow-up modifies only upstream `updateInteraction` motion integration, recorded in [ORIGIN](main/vendor/stopwatch_avatar/ORIGIN.md). Tables, blink/timeline and projection remain retained. The compact renderer and shorter MCP-only activity logic are local AGPL adaptations; no additional third-party source/dependency or license change.

The 0.9.3 compact-window correction is first-party code in `main/hardware/display.c`.
Even-coordinate/size requirements were cross-checked against
[Zephyr's CO5300 driver](https://github.com/zephyrproject-rtos/zephyr/blob/2afc59f6d9784ad7013242d5477cba9745dde484/drivers/display/display_co5300.c#L125),
commit `2afc59f6d9784ad7013242d5477cba9745dde484`; no source was copied/imported,
and no dependency or license change is introduced.
