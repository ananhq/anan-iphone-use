# HID tool reference

English · [简体中文](hid-tools.zh-CN.md)

Candidate `0.5.1-mcp` advertises seven tools through MCP `tools/list`. It merges gestures and keys from the thirteen-tool `0.5.0-mcp` candidate. No Anan code changes or additional phone bindings are required. Refresh the client's tool directory after upgrading; cached definitions may reject the new arguments. Phone behavior and client interoperability remain device-unverified.

Keep the existing AssistiveTouch, USB Ethernet, pairing/import and physical Start setup. Full Keyboard Access is optional, neither enabled nor detected by firmware. Tab/arrow behavior depends on the App and existing settings.

Candidate `0.7.3-discovery` updates descriptions only with English/Chinese task vocabulary for app-opening workflows, browsing, input/editing and drawing. The seven schemas and execution remain unchanged. The service softly recommends browsing the complete group before multi-step planning when its overview is not visible. See [discovery guidance](history/tool-discovery.md).

## Choose an operation

| Tool | Arguments | Purpose |
| --- | --- | --- |
| `device_status` | None | USB, grant and input-busy status; no screen capture |
| `tap` | `x`,`y`: 0…1; optional `count`: 1/2 (default 1), `holdMs`: 0 or 100…2000 (0), `intervalMs`: 50…300 (100), only with count=2 | Single click by default; count=2 double-clicks; holdMs>0 long-presses. Double-click plus hold is rejected |
| `drag` | `points`: 2…32 `{x,y}`; `durationMs`: 100…2000 and ≥20×(point count−1); optional `holdMs`: 0…1000 (0) | Two points swipe/drag; more points draw one continuous stroke. Hold before movement when needed |
| `press_key` | `key`: names below; optional `repeat`: 1…5 (1), `pressMs`: 20…100 (40), `gapMs`: 50…300 (100) | Named keyboard/editing and system/media keys; repetition only for navigation |
| `type_ascii` | `text`: 1…120 US-layout ASCII bytes, printable plus TAB/LF | Type into focused input; no Chinese, Unicode or IME injection |
| `scroll` | `x`,`y`: 0…1; `vertical`: integer −10…10; optional `horizontal`: −10…10 (0), `repeat`: 1…5 (1), `gapMs`: 50…300 (100) | Wheel over a target region; at least one axis nonzero |
| `stop` | None | Interrupt input and revoke control; physically enable again before further input |

Coordinates are portrait-normalized with the origin at top-left. Scroll units are not pixels or lines; direction/distance depend on phone settings and the target. Start small and observe. All calls release their held input; separate drag calls are separate strokes, not a persistent pen hold. A multi-point path divides movement time equally across segments. Times describe firmware pacing; USB waits and scheduling add latency. An unheld two-point drag retains the old straight-drag timing.

Use `press_key` for Home/Search, editing and caret navigation; `tap` for stationary pointer gestures; `drag` for movement/drawing; `scroll` for wheel adjustments. Observe before activating a focus or choosing a toggle. Tools do not return a UI tree, focus IDs, screenshots or App success.

## Key names

Navigation (the only keys allowing repeat>1): `tab`, `shift_tab`, `up`, `down`, `left`, `right`, `select_left`, `select_right`, `word_left`, `word_right`.

Other keyboard keys (repeat=1): `enter`, `escape`, `space`, `backspace`, `delete`, `command_space`, `select_all`, `copy`, `paste`, `cut`, `undo`, `redo`. Editing uses standard Command/Shift/Alt combinations; `delete` is forward delete. Space may enter text or activate an existing focus.

System/media keys (repeat=1): `home`, `search`, `toggle_keyboard`, `globe`, `volume_up`, `volume_down`, `mute`, `play_pause`, `next_track`, `previous_track`, `brightness_up`, `brightness_down`.

Home/Search use Consumer reports; `command_space` remains a keyboard shortcut. Globe depends on text context, media controls the current player, volume/brightness change relative levels. Toggles require observation. There is no universal Back or unlock. System mappings follow Apple Accessory Design Guidelines 2026-09-21 table 14-3; see [research](research/hid-capabilities.md). Mouse/keyboard IDs 1/2 and Consumer ID 3 are unchanged from `0.5.0-mcp`.

## Example arguments

These are tool arguments, not executed actions or full MCP envelopes.

| Tool | Example |
| --- | --- |
| `tap` | `{"x":0.5,"y":0.6}` |
| `tap` (double) | `{"x":0.5,"y":0.6,"count":2}` |
| `tap` (hold) | `{"x":0.5,"y":0.6,"holdMs":700}` |
| `drag` (swipe) | `{"points":[{"x":0.5,"y":0.8},{"x":0.5,"y":0.3}],"durationMs":400}` |
| `drag` (draw) | `{"points":[{"x":0.2,"y":0.3},{"x":0.5,"y":0.6},{"x":0.8,"y":0.3}],"durationMs":800}` |
| `press_key` | `{"key":"select_all"}` or `{"key":"home"}` |
| `scroll` | `{"x":0.5,"y":0.6,"vertical":-2}` |

Unknown fields, duplicate keys, invalid enums/fractions/ranges, repeated activation/media keys, double long holds, intervalMs on a single tap, zero wheel axes and paths too short in duration are rejected before dispatch. Full requests remain limited to 4096 bytes.

## Compatibility calls

The following names remain accepted by the firmware but are absent from `tools/list`. They keep their old argument contracts; agents using the refreshed directory should use the seven public tools instead. A client may reject unlisted names locally, so firmware compatibility does not imply every client can call them.

| Old name | Old arguments | New equivalent |
| --- | --- | --- |
| `keyboard` | `key`, optional `repeat`, `pressMs`, `gapMs` | `press_key` with the same fields |
| `system_key` | `action` | `press_key` with `key=action` |
| `long_press` | `x`,`y`,`durationMs` | `tap` with `holdMs=durationMs` |
| `double_tap` | `x`,`y`, optional `intervalMs` | `tap` with count=2 |
| `drag_hold` | `x`,`y`,`toX`,`toY`,`durationMs`, optional holdMs (default 500) | `drag` with two points and explicit holdMs=500 when omitted before |
| `draw_path` | `points`,`durationMs`, optional `holdMs` | `drag` with the same fields; a two-point unheld path now uses legacy straight-drag timing |
| `key` | `keycode`: 4…231, `modifiers`: 0…255 | Expert raw HID compatibility entry; prefer named `press_key` |

`drag` also accepts its old `x/y/toX/toY/durationMs` form, with unchanged timing. Mixing that form with `points` or adding holdMs to the old form is rejected; use points for the new interface. Default `tap`, raw `key`, `type_ascii`, status and stop retain prior behavior. Descriptors, executor, grants, stop, power and dependencies are unchanged by consolidation.

## Results and stopping

Extended actions still return text and structured HID progress:

```json
{"sequenceCompleted":true,"completedUnits":2,"totalUnits":2,"unit":"key_presses","phoneEffect":"unverified","partialEffectsPossible":false}
```

Units are completed key/Consumer press-release cycles, wheel reports, double clicks, path segments or held gestures. Legacy single tap/two-point drag/raw key/typing retain text-only results. Counts are conservative: incomplete units can affect the phone, and final-release failure can occur after all path segments were counted. Completion never proves App/task success; observe afterwards.

All actions share the same worker, physical grant, session owner, input-ID ledger, cancellation and eight-second deadline. Stop/cancel/failure requests release; detach release is best effort. No reconnect restores grants or replays actions. See [MCP sessions](mcp.md), [predefined acceptance](history/hid-development.md) and [validation](validation.md). Source: [parser](../main/input/hid_input.c), [schemas](../main/network/mcp_tools.h), [executor](../main/input/actions.c), [USB](../main/input/usb.c).
