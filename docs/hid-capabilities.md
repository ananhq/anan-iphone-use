# HID capability research

English · [简体中文](hid-capabilities.zh-CN.md)

Subsequent implementation: candidate `0.5.0-mcp` now implements the no-custom-phone-configuration subset; see [current tools](hid-tools.md). This document preserves the preceding research snapshot; device behavior remains unverified.

Date: 2026-10-14. Firmware baseline: `c794813f960b139089a1d4c72106e5c46c265b61`, `0.4.0-mcp`.

Purpose: improve phone-side [Anan](https://anan.sh) efficiency and accuracy primarily through firmware, retaining generic MCP discovery and screenshot observation. No client edits, phone actions, models or hardware experiments were executed.

This is source/document research, not implemented features. The keyboard direction has been accepted by the user; implementation has not begun. Discuss candidate interface, descriptor and timing changes before implementation. Preserve existing reports, timing, physical grants, stop and no-replay behavior.

## Highest-value findings

The current absolute mouse descriptor already declares five buttons, vertical wheel and horizontal pan. Actions use only the left button and always send zero wheel values. Consumer Control also offers Apple-documented system entry points beyond media controls. Bounded long presses, double clicks, drag preparation and continuous paths can use existing mouse mechanics without first attempting touchscreen emulation.

These improve execution and tool granularity. They do not let the board inspect UI, focus, clipboard contents or success. Efficiency and accuracy benefits remain hypotheses.

| Direction | Required work | Possible value | Priority |
| --- | --- | --- | --- |
| Named keyboard/navigation repetition | Descriptions/new tools; existing report | Easier arguments, fewer known short-run calls | P0; [keyboard study](keyboard-navigation.md) |
| Wheel scrolling | Sender/actions/MCP; retain descriptor | Small scroll adjustments without drag endpoints | P0, vertical first |
| Long press/double click/pre-drag hold | Existing reports, bounded action timing | Menus, selection, drag-and-drop | P0/P1 |
| Home/Search/keyboard visibility/language key | New Consumer report/sender/release/tool | Fewer system-navigation gestures | P1 |
| Extra mouse button bindings | Action entry point and phone setup | User-configured system access | P1 |
| Continuous drawing path | Bounded point/time data and interpolation | Continuous strokes and fewer model calls | P1, strong demo value |
| Pointer-only movement | New tool/action | Preview landing before clicking, if captured | P2; extra observations |
| Media/volume controls | Consumer extension | Avoid locating player buttons | P1/P2 |
| Editing/fixed Chinese text replacements | Existing modifiers or named keys; phone configuration | Faster editing/fixed phrase entry | P1, context-dependent |
| AssistiveTouch gesture presets | Phone setup/menu operation | Indirect system multifinger gestures | P2 |
| Generic digitizer/trackpad/gamepad | Descriptor and host-compatibility research | Specialized gestures/games | Defer |

Priorities are research recommendations, not a promised next-version feature list.

## Wheel scrolling

Locked TinyUSB `0.21.0~2` `TUD_HID_REPORT_DESC_ABSMOUSE` includes five buttons, absolute X/Y, signed vertical wheel and horizontal AC Pan. Current `bridge_mouse()` passes zero for both scroll fields to `tud_hid_n_abs_mouse_report`.[Source](../main/usb.c)

A proposed scroll action can position the pointer over a list, then send small deltas. Do not call deltas pixels, lines or screen fractions: OS settings and applications determine displacement. Calibrate direction, magnitude and natural scrolling on the target; test horizontal scrolling separately. Large moves may remain better as drag. Preserve the old report path and descriptor.

## Bounded gestures

Current tap is a short press, and drag moves immediately after pressing. Same-start/end drag is accepted by the parser and holds the button stationary for the requested duration, making this an existing-tool experiment rather than a required implementation change:

```json
{"x":0.5,"y":0.5,"toX":0.5,"toY":0.5,"durationMs":700}
```

It is not verified long-press recognition on every App. Existing duration remains 100–2000ms. Named long press, double click and pre-drag hold could expose useful timing; double-click intervals should be handled inside one bounded action rather than across model calls. Keep press, hold, movement and release distinct. Avoid persistent cross-call held-button states in the first slice; use existing worker/grant/cancellation checks and release on interruption.[Actions](../main/actions.c)

## Consumer Control

The [official Apple Accessory Design Guidelines](https://developer.apple.com/accessories/Accessory-Design-Guidelines.pdf), downloaded version **2026-09-21**, chapter 14, table 14-3, printed pages 104–105, gives these mappings. They establish documented semantics, not StopWatch device acceptance.

| Usage | Apple mapping / experiment target |
| --- | --- |
| `0x0040` | Home |
| `0x0221` | Spotlight |
| `0x01AE` | Toggle onscreen keyboard |
| `0x029D` | Globe key; context determines language/keyboard behavior |
| `0x00E9` / `0x00EA` | Increase/decrease volume |
| `0x00CD` | Play/pause |
| `0x00B5` / `0x00B6` | Next/previous track |
| `0x006F` / `0x0070` | Increase/decrease brightness |
| `0x0030` | Lock, not generic unlock |

This supplements the current keyboard's missing Fn modifier: some system functions have independent Consumer usages. Locked TinyUSB supplies the Consumer descriptor template and most related constants. No named enum for `0x029D` was found; that value comes from Apple's table and fits the template, with provenance to record in implementation. Proposed report ID 3 would append to existing mouse ID 1 and keyboard ID 2 in the same HID interface. A 16-bit Consumer payload plus ID fits the current 16-byte IN endpoint; enumeration, release and phone behavior still require validation.

Test Home, Search, keyboard visibility and small volume changes first. Test other usages and media contexts separately. Do not equate volume increments with setting a percentage, or assume AC Back is universal iPhone navigation. Toggle actions have unknown-state risks and must not replay automatically.

The guide also imposes accessory requirements on real control input and macros; citing its mappings does not establish Apple certification or full design compliance for Agent automation.

PDF SHA-256: `b631b8b25d6b4fba85f90dff494a7a7edc9996e357cbc97093aedc9d9bd6f2c9`. The PDF and temporary extraction are not added to the repository. The official URL may change content later.

## Extra buttons

The descriptor supports five buttons and the sender accepts a mask, while actions currently use only 0/1. Expose bounded presses/releases for buttons 2–5; the user assigns device actions in AssistiveTouch settings.[Apple button assignment](https://support.apple.com/en-us/111775)

Home or Control Center can be binding experiment targets, subject to the actual OS/device settings list. Right-click is not universal Back or context menu. Expose button numbers or semantic names only for confirmed mappings. Consumer Home/Search could remove configuration for those actions; buttons remain useful for custom actions. Shortcut binding availability has not been independently established here.

## Paths and pointer preview

Current drag is a straight two-point path. A proposed `draw_path` can press once, interpolate multiple points and release once. Candidate limits are 32 points, normalized coordinates and movement time at most 2000ms, also respecting the current 4096-byte request and eight-second action deadline. These are design bounds, not implementation. Preserve cancellation and no held state across tasks. Mouse drawing is not Pencil pressure or native multitouch.

A pointer-only move can preview landing before clicking, but first verify that screen sharing captures the pointer and Dwell does not activate controls. Extra observation may make it slower; do not require it for every tap.

## Editing and Chinese text

Current reports can express ordinary-key/modifier combinations such as editing commands, subject to application/focus behavior. Copy/paste keystrokes do not provide direct Unicode clipboard access.

Apple documents text replacements with an external keyboard.[Official keyboard guide](https://support.apple.com/en-kw/guide/iphone/iph3da414515/ios) A user-configured ASCII abbreviation expanding into a fixed Chinese phrase is a possible small experiment; separator behavior, field type and Full Keyboard Access interception must be checked. This is not arbitrary Chinese input. General Chinese still needs IME/candidate handling or an additional phone-side Shortcut/clipboard workflow, not HID magically writing Unicode. No unverified complete text-input interface is proposed here.

## Deferred device classes

AssistiveTouch exposes multifinger/custom gestures; the existing pointer could select them in the menu for experiments. System-generated gestures are different from transmitting multiple touch contacts and cost configuration/menu steps.[Apple gesture guide](https://support.apple.com/en-us/111794)

The Digitizer protocol exists, but generic USB touchscreen acceptance on iPhone is not established. Apple guide chapter 15 explicitly introduces its trackpad feature at iPadOS 14.5; that is not iPhone compatibility evidence. TinyUSB Gamepad templates likewise do not prove recognition by iPhone's Game Controller framework. A compatible wired controller route can serve a separate game demo, with lower daily-UI value.[Apple controller discovery](https://developer.apple.com/documentation/gamecontroller/discovering-game-controllers)

USB-IF explicitly notes that a defined usage does not imply host support.[HID specification entry](https://www.usb.org/hid)

## Experiment sequence and evidence

1. Accepted keyboard direction: visible focus, Search and input, following the [keyboard study](keyboard-navigation.md).
2. Descriptor-preserving wheel/hold/double-click/drag preparation, compared with current tap/drag.
3. Consumer enumeration followed by individually tested system actions.
4. Configured buttons and continuous paths, changing one relevant setting at a time.

These are prospective acceptance scenarios, not executed tests. Use harmless targets and synthetic content; observe before/after, record time, calls, observations, mistakes and corrections. Record wheel delta versus displacement, gesture holds/intervals, and system/App/media context for Consumer. Verify stop, disconnect and cancellation release, with no replay. Transport completion is not task success.

Generic MCP extensions appear possible without Anan source changes after refreshing discovery, based on prior static client inspection. New-tool interoperability, model adoption and net gains remain unverified. Only the keyboard direction has user acceptance; research does not implement the other proposals.

Source entry points: [USB](../main/usb.c), [actions](../main/actions.c), [action parameters](../main/actions.h), [MCP parsing/limits/guards](../main/mcp.c), [tools](../main/mcp_tools.h); locked TinyUSB `0.21.0~2` `hid_device.h` and `hid.h`. No copied implementation or dependency changes. [Espressif USB Device documentation](https://docs.espressif.com/projects/esp-usb/en/latest/esp32s3/usb_device.html) supplies general background, not device acceptance.
