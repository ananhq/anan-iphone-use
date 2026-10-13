# Touch UI decisions

English · [简体中文](touch-ui.zh-CN.md)

The user requested a touch-first UI because the long-press menu and USB/MCP
names were difficult to operate/understand. They explicitly require Chinese and
English, switchable on StopWatch. This authorizes native touch/UI implementation,
not a new flash, phone action, simulator, provider or E2E run. Anan stays unchanged.

## Original touch scope before implementation (historical)

- StopWatch CST820B, I2C0 on SDA47/SCL48, address0x15, interrupt GPIO13,
  reset M5IOE1 PYG4/bit3. Pinned factory driver/reference uses seven bytes from
  register0x00, finger count, event0/2 and 12-bit x/y. Use native IDF APIs and
  bounded polling on a separate task; do not import the factory UI or LVGL.
- Home: connection status, clearly named Anan direct/MCP app source buttons,
  one main start/stop control and Settings. Both sources use the same USB cable.
  Default ANAN1/report semantics stay intact; direct control is authorized in Anan.
- Pairing: phone address, matching code, a visible Confirm pairing button, progress
  and instructions to import on the phone before leaving setup. Secrets/typing
  remain on the phone. Reset is separate, with a two-second physical hold.
- Settings: Chinese/English selector, MCP setup, reset, Back. Language is remembered
  separately from identity, grants/sessions/actions are never persisted.
- Any raw front-button press still stops active/granted input. Touch during
  granted/active input stops and consumes that contact. Menu navigation and
  enabling require a fresh deliberate tap/release on the currently displayed
  button; no held contact or stale frame can enable control. Physical keys remain
  a fallback when touch fails; their I2C-free stop task must remain independent.
- Reuse the current DMA strip renderer, with rounded buttons/consistent hit boxes
  and a small Noto Sans SC bitmap subset for Chinese. Own ASCII glyphs remain.
  Font source is pinned and the complete SIL OFL notice retained; no project
  license change or new runtime graphics dependency.

## Predefined checks

| Check | Evidence required |
| --- | --- |
| Static/build | Pinned build/package, original USB/ANAN1/action source invariants, bilingual labels/font coverage, exact font/license/source hashes, bounded I2C/task/geometry checks |
| Visual documents | Render the same layout/text/font assets to standalone images; inspect circle clipping, both languages, labels and button areas; this is not device execution |
| Touch hardware | Separately authorized tap coordinates, down/up/contact behavior, polling/error recovery and all visible buttons on the real board |
| Input/stop | Front key/touch stop during drag/type; no same-contact re-enable, no action replay or hidden source switch; measured latency |
| Language/setup | Switch/reboot/upgrade language preference, matching-code approval, actual iPhone trust/config export, and clear reset confirmation |
| Failure | Touch unavailable/stale read/held finger/frame changes: no accidental enable; physical fallback/stop remains usable |

Device checks remain pending until separately authorized and observed. No unit
tests are added after implementation. Keep first-party modules documented in
both languages and preserve original provenance gaps.

## References

- [StopWatch touch hardware](https://docs.m5stack.com/en/core/StopWatch).
- Factory commit `6b4aa125288b6fe9dca661f10159f6e1e5ee785c`,
  [CST820 driver](https://github.com/m5stack/M5StopWatch-UserDemo/tree/6b4aa125288b6fe9dca661f10159f6e1e5ee785c/main/hal/drivers/cst820),
  `main/hal/hal_display.cpp`, `main/hal/hal_ioe.cpp`; root MIT applies, individual
  CST files have no separate SPDX. Use register facts, not copied class code.
- Noto Sans SC variable font, google/fonts commit
  `a85815a42757630ce188fdad368c2dfc444d4773`, `ofl/notosanssc/NotoSansSC[wght].ttf`,
  [SIL OFL](https://github.com/google/fonts/blob/a85815a42757630ce188fdad368c2dfc444d4773/ofl/notosanssc/OFL.txt).

## Implemented development slice

Candidate0.2.0 implements the layout, remembered device language and a bilingual
phone page. Touch reads have a20ms timeout and10ms task delay; the raw physical
stop task retains5ms polling and higher priority. Failed reads consume a contact,
release uses the last valid pressed coordinate, and permission changes are
serialized against a fresh displayed-frame revision. Rendering/persistence do
not run inside physical stop. Hardware touch and persistence remain unverified.

For static previews, install Pillow12.3.0 in a development environment and run
`python tools/render_ui.py`. This compiles only `ui_layout.c` into a document
layout exporter, with synthetic code/fingerprint data, and draws the same bitmap
assets to ignored `build/ui-previews/`. It cannot operate firmware, a phone,
provider or HID. `tools/generate_ui_font.py FONT_FILE` recreates the CJK subset
from the exact source/hash recorded in THIRD_PARTY. Generated glyphs retain OFL.

The user approved removing the three-minute grant limit. Candidate `0.2.1-mcp`
uses a volatile boolean grant until stop/detach/reboot and retains mode, identity
maintenance and display-failure revocation. The active UI says Control enabled.
Compile/package, shared-layout bilingual previews and source review are allowed;
longer-than-180-second control and revoke/reboot behavior remain device checks.

HTTP candidate `0.3.0-mcp` changes pairing/reset labels to configuration import
instead of mandatory certificate trust. Start control uses HTTP-or-HTTPS MCP
readiness; physical/touch stop and no same-contact enable remain unchanged.

Candidate `0.4.0-mcp` removes Anan direct/MCP source choices and boots into MCP.
Home shows MCP control with one start/stop button and Settings. Pairing, language
and physical/touch stop remain; current static previews contain20 scenes.
