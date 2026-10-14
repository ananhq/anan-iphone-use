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
extracts the original action routines into `main/actions.c`, adapts `main/main.c`
for arbitration and adds an I2C bus accessor to `board.c`; `usb.c` and PROTOCOL.md
remain unchanged. The old USB/board provenance gaps remain unresolved.

New MCP, identity, button UI, strip renderer, schemas and phone setup page are
first-party implementations using the pinned SDK APIs; neither Anan client nor
Espressif MCP C SDK code is imported. ASCII glyphs are authored here; the later CJK subset is derived under OFL as recorded below. The one adapted
vendor data file is `main/panel_init.h`: panel commands from M5Stack
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

Display startup correction: `main/display.c` now independently implements
M5IOE1 wake/retry/settling and disables expander I2C sleep before screen power.
Behavior/register facts were checked against `src/M5IOE1.cpp`,
`src/M5IOE1_i2c_compat.h` and `src/M5IOE1.h` at the pinned M5IOE1 commit above,
and factory `main/hal/hal_ioe.cpp` at the pinned demo commit. The vendor library
is not imported. Retain the [complete M5IOE1 MIT notice](licenses/M5IOE1-MIT.txt),
Copyright (c) 2026 M5Stack Technology CO LTD. No original USB/board provenance
gap is cleared by this correction.

## Touch/bilingual UI additions

`main/touch.c` is a bounded first-party IDF adapter based on CST820B register and
reset facts from the pinned factory `main/hal/drivers/cst820/cst820.cpp/.h` and
`hal_display.cpp` / `hal_ioe.cpp`. CST source files lack individual SPDX headers;
the inspected root MIT license applies, and its complete M5Stack notice remains
included. No vendor C++ class, LVGL or M5GFX implementation is copied/imported.

`main/ui_font.h` is generated **StopWatch UI Glyphs**, a renamed 24px bitmap
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

Candidate `0.5.0-mcp` adds first-party bounded parsing/execution, schemas and offline acceptance fixtures. The fixtures were authored before implementation and simulate reports/time, not device acceptance. USB code now uses the existing locked TinyUSB `0.21.0~2` Consumer descriptor/API; no upstream implementation was copied or dependency upgraded. Numeric Consumer usages follow Apple Accessory Design Guidelines 2026-09-21, chapter14 table14-3 (including 0x029D, absent as a named enum in this SDK); [research](docs/hid-capabilities.md) records the PDF hash. Existing original USB/board provenance gaps and license notices remain. Mouse/keyboard IDs and USB configuration are retained with Consumer ID3 appended.

Candidate `0.5.1-mcp` consolidates tool discovery and first-party parsing, retaining the executor and USB code. The added consolidation acceptance scenarios preceded this implementation. No external code, dependencies or licenses changed; prior provenance gaps remain.
