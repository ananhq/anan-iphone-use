# Control eyes

English · [简体中文](eyes.zh-CN.md)

Current interactive tilt/touch and smooth rendering supersede the passive face below; see [avatar](avatar.md).

Agreed scope and acceptance, defined before implementation of `0.7.8-eyes`:

- A fresh Start control grant replaces Home with a pure black frame and two
  filled pale eyes. No text, battery, status, buttons or touch targets remain.
  Either front key stops and returns to ordinary Home; the press is consumed
  through release. All touches during control are ignored. Read errors retain
  fail-closed revocation. MCP stop, input/display loss and reboot still revoke.
- Idle eyes blink; tap/long press/double tap squish, drag/path follow the latest
  successfully sent pointer coordinates, typing bounces, scroll looks along its
  direction, keyboard/Consumer input winks. Failed input briefly looks subdued
  if control remains granted. These are input feedback, not phone success or
  model-thinking indicators. Status/discovery produce no action animation.
- Worker begin/end and pointer notifications publish only bounded atomic visual
  metadata, never text/key content. No drawing, allocation, wait or new HID
  report occurs there. The existing low-priority display task samples geometry
  at a nominal 40ms interval. The original short-action visual lasted at least 400ms;
  a new action replaces the old visual. A fresh grant resets previous visuals.
- Retain MCP tools/schemas, HID descriptors/senders/execution pauses, physical
  stop and power policy. No client changes, dependencies or external assets.
  Battery polling is limited to ordinary Home while stopped.
- Before runtime edits, define host fixtures for empty active layout/hit regions,
  all-touch-ignore behavior, both physical keys and consumed release, geometry
  bounds, coordinate tracking, reset/replacement/linger/time wrap, and black-only
  background. Run unchanged HID/wireless/flash checks and pinned compile/package.
  Render previews from the same native sampler and strip painter.
- Actual frame rate, appearance, shared-bus responsiveness, key stop latency and
  BLE/USB input under animation load require separately authorized board/phone
  acceptance. Host checks/builds do not establish these outcomes.

This supersedes the active Stop button in `0.7.7`; pairing/settings retain their
ordinary layout. No flashing, device probe, phone/client or E2E action is implied.

## UI stack correction: defined before implementation

After `0.7.8` the user reports a green flash and no response when tapping Home's
Settings. No device log/reset cause was collected. The target compiler identifies
2592-byte frames in each layout-based hit helper; the touch/key tasks allocate
only 4096 bytes. Adding the task, layout and linked libc formatting frames
exceeds that budget on the touch path, before all deeper/runtime overhead.
This is a source/build-confirmed resource defect, not device-proven causation.

The `0.7.9-ui-stack` correction gives both interactive UI tasks 8192 bytes, adding
8 KiB total task-stack allocation. Keep display power/timing, UI layout, eyes,
touch targeting, input reports, action execution and stop bodies unchanged.
Before runtime edits, define `tools/check_ui_stack.py`: compile actual UI sources
with the candidate target flags and `-fstack-usage`, read linked libc entry frames,
and require each UI task to fit the known formatting path plus 2 KiB reserve.
Run first against the preceding allocation, then the correction. Reuse existing
UI/HID/wireless/flash checks and pinned compile/package. The static reserve is not
an exhaustive worst-case proof or measured high-water mark. Separately authorized
device acceptance: Settings → each submenu → Back, short taps and long holds,
Start → eyes → either key exit, with no reset/green flash and unchanged input/stop.

## Stronger action feedback: defined before implementation

The user reports navigation works after the stack correction, but action feedback
looks like idle blinking. Source tracing finds worker begin/end and successful
pointer notifications already connected. The original keyboard wink/tap peak
occupies only about 200ms; typing moves by at most 8px and pointer gaze by 30px.
No live notification trace or display frame timing was collected.

For `0.7.10-eyes-feedback`, use a distinct pose from the first sample, larger
coordinate-based gaze and alternating typing hops. Scroll magnitude maps to a
bounded direction, not an unbounded displacement. Keep a short action visible
for 900ms from start and at least 320ms after completion, then blend out over
240ms; a new input replaces it immediately. Keyboard winks stay asymmetric,
and taps/long presses retain a squint between pulses. Do not alter rendering
cadence, black-only UI, input reports/pauses, stop or client/MCP behavior.

Before runtime edits, extend host acceptance: every input family has a visibly
different first pose; even an action that ends in 20ms has at least four distinct
samples at a synthetic 160ms frame interval across phase offsets; feedback then
settles to idle. Run the actual queued worker with synthetic time/reports to
verify begin/end notification and unchanged tap/type/key durations. Retain all
geometry/render/stop/stack checks and pinned compile/package. These fixtures do
not measure the actual display cadence or establish board appearance. Separate
device acceptance: one tap, a long drag/path, ASCII text, scroll and Home/search
key, with visibly distinct reactions and immediate physical stop.
