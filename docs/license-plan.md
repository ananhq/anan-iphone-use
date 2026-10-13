# Anan iPhone Use: license and source handling

English · [简体中文](license-plan.zh-CN.md)

The user confirms that actell source was referenced and authorized choosing an open-source project license. The combined Anan iPhone Use (`anan-iphone-use`) project now adopts **GPL-3.0-only**, with the complete text in [LICENSE](../LICENSE). The previous root Apache-2.0 text is retained in [Legacy-first-party-Apache-2.0.txt](../licenses/Legacy-first-party-Apache-2.0.txt). Existing source declarations, third-party licenses and historical evidence are preserved. This is an overall distribution-license decision, not a completed source audit or an all-independent implementation claim.

## Current declarations and source handling

| Area | Current declaration / evidence | Required handling |
| --- | --- | --- |
| Root LICENSE and existing first-party C files | Root LICENSE is GPL-3.0; historical Apache-2.0 file declarations and earlier grants are retained; declarations alone do not resolve provenance | Distribute the combined work under GPL-3.0-only and audit per-file declarations; preserve earlier copyright/license grants and applicable notices rather than blindly replacing headers |
| `main/usb.c` | Existing Apache-2.0 SPDX; unchanged from checkpoint; substantial overlap with actell USB files | Establish exact copied/adapted portions, retain applicable actell terms and document modifications; do not claim the current SPDX resolves GPL derivation |
| Original `main/main.c`, `bridge.h`, `board.c` and later action extraction | Existing declarations; historical generation and board register origin incompletely established | Review origin and modifications separately, preserve source snapshots and resolve obligations before distribution |
| MCP, identity, display/touch and other additions | First-party code uses pinned SDK APIs; MIT panel command data and OFL CJK font subset are recorded separately | Audit combined distribution and each applicable notice; project-wide GPL distribution does not erase dependency MIT/Apache/BSD/OFL terms |
| esp32-iphone-agent | MIT, Copyright (c) 2026 Henry Yuan | Keep the [complete upstream notice](../licenses/esp32-iphone-agent-MIT.txt); identify any actual historical reuse before claiming a complete audit |
| SDK and libraries | ESP-IDF/esp_tinyusb Apache-2.0, TinyUSB MIT, lwIP BSD and additional notices in THIRD_PARTY | Preserve their own licenses, exact versions and notices; complete the linked-binary inventory |

## Exact references and modification record

- actell: commit `e701786d1686823f9ab60524e689e387b7c3b904`, [root license](https://github.com/ja1chen/actell/blob/e701786d1686823f9ab60524e689e387b7c3b904/LICENSE), [source notices](https://github.com/ja1chen/actell/blob/e701786d1686823f9ab60524e689e387b7c3b904/THIRD_PARTY.md), [USB network](https://github.com/ja1chen/actell/blob/e701786d1686823f9ab60524e689e387b7c3b904/firmware-device/main/usb_network.c) and [USB HID](https://github.com/ja1chen/actell/blob/e701786d1686823f9ab60524e689e387b7c3b904/firmware-device/main/usb_hid.c). The inspected distribution declares GPL-3.0-only with file-specific terms; upstream THIRD_PARTY retains GPL-3.0-or-later headers for original v0.1/new independent files and GPL-2.0-only OR GPL-3.0-only for raw mux. No such module is newly imported here. The two USB files lack individual SPDX headers; that does not grant permission to relicense them. Exact local derivation remains unresolved.
- Local modifications: migration preserved `main/usb.c` byte-for-byte; subsequent MCP development also leaves it unchanged. Local protocol/action logic differs from actell; later development extracts actions to `actions.c`, adds arbitration in `main.c`, exposes the board I2C bus and adds MCP/UI modules. These are local modification facts, not a complete upstream-to-local derivation map. The [source snapshot](../artifacts/migration/source-snapshot.json), checkpoint `4c6015c` and [THIRD_PARTY](../THIRD_PARTY.md) retain the detailed record.
- esp32-iphone-agent: commit `68584cfc8b3f227d6324ec7c8008c48b53f20a18`, original file `LICENSE`, MIT, Copyright (c) 2026 Henry Yuan. The full notice is copied without changes to `licenses/esp32-iphone-agent-MIT.txt`; implementation code is not imported by this documentation update.

## GPL distribution requirements

1. The whole-work license is GPL-3.0-only; resolve the per-file source/notice map, especially USB and original board code. Retain the accurate upstream versions, copyright notices, terms and prominent modification notices where required.
2. Preserve third-party licenses and the historical evidence. Credit is not a substitute for license obligations; a root license change cannot itself complete provenance review.
3. For a binary distribution subject to GPLv3, provide Corresponding Source through a compliant distribution method, including required source and scripts/build materials; provide Installation Information where applicable. See [GPLv3 sections 1, 5 and 6](https://www.gnu.org/licenses/gpl-3.0.html) and the [release checklist](release-preparation.md).
4. Record the exact distributed source commit, dependency/toolchain versions and image hashes. Complete the linked application/bootloader notice inventory before publishing; exclude credentials, pairing records, private data and generated caches.

The root license decision is applied. No source implementation, existing per-file header, dependency version, historical artifact, publication or cross-repository [Anan](https://anan.sh) edit is changed by this licensing/documentation update. Exact derivation and release preparation remain separate work.

The MCP-only follow-up removes the local ANAN1 TCP parser/server; it does not
change the retained USB implementation or resolve its recorded actell origin.
The combined-project license is now GPL-3.0-only; dependency notices remain unchanged.
