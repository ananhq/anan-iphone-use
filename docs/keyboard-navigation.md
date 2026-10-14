# Keyboard and system focus exploration

English · [简体中文](keyboard-navigation.zh-CN.md)

Subsequent implementation: candidate `0.5.0-mcp` now implements the no-custom-phone-configuration subset; see [current tools](hid-tools.md). This document preserves the preceding research snapshot; device behavior remains unverified.

Date: 2026-10-14. Source baseline: `c794813f960b139089a1d4c72106e5c46c265b61`, firmware `0.4.0-mcp`.

Status: source and official-document review only. No device experiments or proposed extensions have been implemented or executed. This document changes neither the current MCP contract nor HID descriptors, input timing or grants.

## Findings

The existing firmware can send Tab, Shift+Tab, Space and Command plus an ordinary key. A first Full Keyboard Access experiment requires no firmware or Anan code changes. The user enables the system feature; Anan uses its existing screenshot observation to identify focus and the existing MCP `key` tool to navigate.

Firmware-only improvements can make tools easier for a model to use, batch short known navigation runs and provide experimental pacing. They cannot tell the board which control has focus. Explore system shortcuts, nearby focus navigation and coordinate input together rather than traversing every page with Tab.

## Current implementation

| Layer | Implementation | Meaning |
| --- | --- | --- |
| Descriptor | `main/usb.c:21`, TinyUSB standard keyboard, report ID 2 | Eight modifier bits and six ordinary-key slots |
| Report | `bridge_key()`, `main/usb.c:69`, `keys[6] = {key}` | One ordinary key at a time, potentially several modifier bits |
| Action | `stroke()`, `main/actions.c:86` | Press, approximately 12ms wait, release all, approximately 12ms wait; USB and scheduling add time |
| MCP | Numeric `keycode` and `modifiers` | Sufficient for common shortcuts; less convenient than named keys |
| Text | `type_ascii`, bounded US-layout strokes | ASCII, not Unicode injection; focus and keyboard settings matter |

Left Ctrl/Shift/Alt/GUI modifier bits are 1/2/4/8; right equivalents are 16/32/64/128. GUI normally maps to Command on iPhone, but users can remap modifiers. Each action releases the keys, so separate calls cannot hold Tab while pressing H. The descriptor already provides multiple ordinary-key slots; adding chord support would not necessarily require changing it, but is unnecessary for the first experiment.

## Existing calls

Apple documents Full Keyboard Access focus highlighting, Tab navigation, Shift+Tab reverse navigation, Space activation, configurable bindings and highlight appearance. Apple also documents Command+Space for Search. See [Full Keyboard Access](https://support.apple.com/en-kw/guide/iphone/ipha4375873f/ios) and [keyboard shortcuts](https://support.apple.com/en-kw/guide/iphone/iph3da414515/ios).

These are arguments to the existing MCP tool `key`, not new tools or device-verified results.

| Intent | Arguments | Conditions |
| --- | --- | --- |
| Next focus item | `{"keycode":43,"modifiers":0}` | Full Keyboard Access enabled, default bindings |
| Previous focus item | `{"keycode":43,"modifiers":2}` | Same |
| Activate selected item | `{"keycode":44,"modifiers":0}` | Visually confirm focus; Space may behave differently while entering text |
| Search | `{"keycode":44,"modifiers":8}` | Standard Command mapping; check interception or custom bindings |
| Enter / Escape | Keycodes 40 / 41, modifiers 0 | Context-dependent |
| Right / Left / Down / Up | Keycodes 79 / 80 / 81 / 82, modifiers 0 | May affect focus, text cursor or a control value |

The current Apple Full Keyboard Access page also lists Fn-based system commands. Fn is absent from the current eight modifier bits; do not substitute a Ctrl/Alt/GUI bit for it. Tab+H is also a two-ordinary-key chord unsupported by the current single-key tool. User-configured Commands bindings may provide alternatives, but firmware must not assume a mapping that has not been configured.

## Expected tradeoffs

After visual confirmation, Space delegates activation to system focus rather than estimated pointer coordinates. This is a possible accuracy benefit, not a measured result. Shortcuts may save gestures; short known key runs may save MCP calls. Observing and reasoning after every Tab or traversing unrelated controls can instead be slower. Focus order need not match visual reading order.

The board has no screenshot, focus identifier or UI tree. It cannot report a focused label, prove target selection, or determine the required number of Tab presses. Long unattended runs remain blind input.

## First experiment with existing firmware

These are prospective acceptance scenarios. Agent device execution requires authorization for the current identified target and scope; the user can also perform them manually.

Prepare paired MCP, physically enabled StopWatch control and user-confirmed Anan screen sharing. Keep the phone unlocked and portrait. Enable Full Keyboard Access in Accessibility → Keyboards & Typing and make focus highlighting conspicuous; settings labels vary by OS version.

Record Slow Keys, Sticky Keys, modifier mappings and custom commands. Start with Slow Keys and Sticky Keys disabled: they change short-press recognition or combination behavior. See [Apple keyboard accessibility settings](https://support.apple.com/en-kw/guide/iphone/ipha7c3927eb/ios).

| Scenario | Procedure | Evidence needed |
| --- | --- | --- |
| Visible focus | Observe a stable Settings page, Tab, observe | Recognized key and distinguishable highlight |
| Forward/backward | Tab and observe, Shift+Tab and observe | Whether focus returns; record groups and wraparound |
| Activation | Focus a harmless confirmed control, Space, observe | Correct control activated, rather than text entered |
| Search | Command+Space, observe, type ASCII `Safari`, inspect | Search opens and results are usable; record binding conflicts |
| Text context | Synthetic content; navigate, type ASCII, Enter | Shortcut interception, Space behavior and focus changes |
| Mixed input | Coordinate tap near a target, keyboard navigation, observe | Whether pointer and keyboard focus agree; never assume they do |
| Stop | Physical stop or USB interruption during input | Release/revocation and no restored permission or replay |

Compare coordinate input with mixed input on identical device, page, task and model settings. Record completion time, tool calls, observations, incorrect activations, corrections and failures. Distinguish key recognition, visual interpretation and actual task success. A few trials establish feasibility, not broad statistical superiority.

Suggested Anan request, requiring no client code changes:

```text
Full Keyboard Access is enabled. Test focus navigation with StopWatch MCP key:
Tab=43/0, Shift+Tab=43/2, Space=44/0 (keycode/modifiers).
Observe first, send one Tab, observe again and identify the highlighted item.
Try at most five steps, observing after each; stop or use coordinates if focus is unclear.
Press Space only after confirming the target. Do not blindly combine navigation and activation.
```

Use synthetic text and avoid consequential submission/deletion targets.

## Firmware-only proposal, pending design discussion

1. Improve existing `key` descriptions with common codes, modifier mappings and observe-before-activation guidance, without changing its schema or execution. This assists models but does not guarantee strategy adoption.
2. Add one candidate named-key tool, `keyboard`, with a finite enum: `tab`, `shift_tab`, `space`, `command_space`, `enter`, `escape` and arrow keys. Preserve `key`. Do not describe Space as unconditional target clicking or assume Fn commands.
3. Candidate navigation-only `repeat` 1–5, default 1; `pressMs` 20–100, default 40; `gapMs` 50–300, default 100. Timing applies only to the new tool. These are experimental starting values, not validated optima; retain legacy key/ASCII timing.

Example proposed arguments:

```json
{"key":"tab","repeat":2,"pressMs":40,"gapMs":100}
```

Anan observes after two Tab presses. No automatic Space follows. Short repetition needs a reasoned focus-distance estimate and must not become unattended page scanning.

Use the existing single worker, physical grant, session ownership, cancellation and deadline checks. Check interruption during every press/release/wait; release on completion/failure and never retry implicitly. Return actual completed strokes and possible partial execution, never claimed selection success. Finalize partial-result fields before implementation.

Read-only Anan inspection found `MCPClientAdapter.tools()` calling `listTools`, `MCPRemoteTool` preserving remote descriptions/schemas, and calls forwarded by remote name. Simple enum/integer extensions therefore appear compatible without Anan code changes, after refreshing discovery. This is source evidence, not a live interoperability or model-adoption result.

## Deferred work and decision point

Do not first upgrade NKRO/descriptors, add proprietary Fn reports, shorten all timing, remove physical grants or treat Unicode strings as HID text. Full Keyboard Access does not solve Chinese IME input.

First establish visible Tab focus, reversible navigation, correct Space activation and Search on the target. Then consider named tools and bounded repetition. If traversal or highlight recognition is poor, retain shortcut benefits without making focus navigation a universal default.

Sources: [USB reports](../main/usb.c), [actions](../main/actions.c), [MCP parsing/guards](../main/mcp.c), [tool definitions](../main/mcp_tools.h); keyboard constants/descriptors checked against locked TinyUSB `0.21.0~2` `hid.h` / `hid_device.h`. Independent Anan files inspected only: `apps/ios/Anan/MCP/MCPClientAdapter.swift`, `MCPToolDirectory.swift`, `MCPRuntime.swift`; no client changes or execution.
