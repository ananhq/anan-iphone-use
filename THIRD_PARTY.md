# Provenance and third-party notices

English · [简体中文](THIRD_PARTY.zh-CN.md)

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
The inspected distribution declares GPL-3.0-only, with file-specific terms. The two USB files inspected have no individual SPDX header, which is not permission to relicense them. Similarities include the pbuf-copy/synchronous-send/free path with 50ms timeout and 1514-byte bound, lwIP netif/start sequence, 2s semaphore initialization, eFuse-derived host-MAC bit flip, NCM endpoint layout, HID report IDs/descriptors and USB event/config callbacks. Anan differs in action worker, power/buttons, protocol and several USB details; those differences do not resolve implementation origin. No actell files are newly copied by this migration. Do not publish an all-independent guarantee. If actual derivative reuse is confirmed, handle its license/source obligations and obtain approval for any license change before release; credit alone does not resolve that question.

**esp32-iphone-agent**, commit `68584cfc8b3f227d6324ec7c8008c48b53f20a18`:
[repository](https://github.com/onehumanbeing/esp32-iphone-agent/tree/68584cfc8b3f227d6324ec7c8008c48b53f20a18),
[MIT license](https://github.com/onehumanbeing/esp32-iphone-agent/blob/68584cfc8b3f227d6324ec7c8008c48b53f20a18/LICENSE), Copyright 2026 Henry Yuan.
Referenced for external USB HID/agent architecture. Its Arduino/Wi-Fi/computer-side agent is not a build dependency; no files are newly imported. Historical copy/adaptation has not been exhaustively audited. Preserve the complete upstream notice if reuse is established.

**M5Stack**: [StopWatch documentation](https://docs.m5stack.com/en/core/StopWatch) and linked schematic; board/download-mode facts, not an imported vendor library or artwork. `M5PM1` register behavior was not proven solely by the product page; retain the original board implementation and verify its exact code/reference source before an origin guarantee. Vendor documents are not granted this repository's license.

## Build dependencies

| Dependency | Pinned source/version | Notice included |
| --- | --- | --- |
| ESP-IDF | v5.5.5, `b774170ff46c393eeb5e495ea37936038d3f4f4f` | [Apache-2.0](licenses/ESP-IDF-Apache-2.0.txt); component-specific notices also apply |
| esp_tinyusb | registry 2.2.0, hash in dependencies.lock | [Apache-2.0](licenses/esp_tinyusb-Apache-2.0.txt) |
| TinyUSB | registry 0.21.0~2, hash in dependencies.lock | [MIT](licenses/TinyUSB-MIT.txt), Copyright 2012–2026 hathach |
| lwIP | IDF submodule `fd432e4ee2cfb7f7f1c7eb7227e0173412e7b84e` | [BSD notice](licenses/lwIP-BSD.txt), Swedish Institute of Computer Science |

Sources are fetched separately through IDF/registry, not vendored here. These license copies do not constitute an exhaustive binary-license inventory for every linked SDK component. Before publishing a binary, inspect its exact linked components and retain additional applicable copyright/license notices with the release. Keep [LICENSE](LICENSE) and [NOTICE](NOTICE), record the final repository address/version, and resolve outstanding first-party provenance. No dependency upgrade or project license change is part of migration.
