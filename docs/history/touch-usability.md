# Large-target UI and home battery indicator

English · [简体中文](touch-usability.zh-CN.md)

Scope agreed before implementation: apply the reviewed native 466px round-screen
design and show estimated battery charge only on Home. Keep the current strip
renderer/font, physical stop, fresh-contact authorization, HID, MCP and client
behavior. No runtime dependency or power-policy change.

## Predefined acceptance

- Every visible button is at least 72px high, stays inside the round screen,
  has a nonoverlapping target and fits both languages. Home's main action is
  96px high; menus have at most three choices plus Back.
- Settings opens Input, Connect / Wi-Fi or More. More opens Language, Bluetooth
  reset or MCP reset. Back returns to the parent; leaving pairing closes its
  setup window/hotspot and does not grant input. Done returns Home.
- Active Home exposes Stop, without a misleading Settings target. Touch or a
  raw front key still stops first and consumes that contact. No replay or
  automatic grant after stop, reset, input change or reconnect.
- A normal tap must start strictly inside its button. Up to 10px of subsequent
  drift is tolerated around that original target, never retargeting a neighbor.
  Two-second reset holds retain strict bounds. Read failures, stale revisions,
  changed controls or physical keys cancel the gesture. Press/progress feedback
  and battery changes alone must not invalidate a gesture.
- The existing M5PM1 I2C device reads two bytes at 0x22, little-endian mV, with
  a bounded timeout and no new power writes. Invalid/failing samples display
  Unknown, not zero or an old value. The first-party coarse estimate maps
  3.3–4.2V linearly to 0–100%, rounded to 5%; samples outside 2.5–4.5V are invalid.
  This is an uncalibrated voltage estimate, not a fuel gauge or runtime estimate;
  charging/load/temperature can bias it. The UI explicitly says Estimated.
- Poll at most once per 15 seconds in the low-priority screen task, only on
  Home and while no touch/front key is pressed. Battery/feedback never change
  the authorization revision or stop task. Do not infer charging from USB.
- Before firmware changes, define host layout/target, estimate and actual
  control-function host fixtures. Then run the pinned SDK build/package, existing
  offline HID/wireless/flash checks, same-layout bilingual renders, glyph/QR
  checks and unchanged transport/report/security/storage source hashes.
- Device acceptance, separately authorized: all navigation and edge taps;
  short/long reset confirmation, jitter and cancellation; touch/key stop during
  input with no same-contact enable; language persistence; hotspot joining/QR;
  battery-powered and USB-powered readings, noise and touch responsiveness.
  Static renders/host fixtures do not establish hardware success.

## Hardware/source reference

M5Stack [M5PM1](https://github.com/m5stack/M5PM1/tree/be9a5456c007c333e7ac963f33bfde1ffa5d82ee),
`docs/M5PM1_Datasheet_EN.pdf`, ADC Register section and `src/M5PM1.h/.cpp` identify
read-only VBAT registers 0x22/0x23 and mV units. The new C adapter uses register
facts, not a copied vendor library/class or battery-percentage algorithm.
The full M5PM1 MIT notice is retained; the existing board provenance gap remains.

## Button-only touch stop: agreed follow-up, before implementation

The user explicitly replaces touch-anywhere stop because of accidental touches.
For `0.7.7-button-stop`, a fresh contact that starts strictly inside the currently
displayed Home Stop button revokes input immediately and is consumed until
release. Touching status, battery, blank space, or starting outside and sliding
into Stop must leave both the grant and ongoing actions intact. Stale frames
cannot supply a Stop target. Physical front-key/MCP stop, display/input loss,
read-error fail-closed behavior and no same-contact enable remain unchanged.
The earlier touch-anywhere bullet above describes `0.7.6` and is superseded.

Before firmware edits, extend the actual-function host fixture for blank/status/
battery touches, slide-in, active/busy Stop, stale display, read failure and
consumption through release. Run it first against `0.7.6` to reproduce the
blank-touch revocation, then against this change. Reuse layout/battery checks,
pinned compile/package and shared-layout bilingual renders; retain report,
executor, radio, security, storage and physical-stop source bodies. Hardware
button targeting, stop latency and accidental-touch behavior require separate
device acceptance. No device execution is authorized by this source change.
