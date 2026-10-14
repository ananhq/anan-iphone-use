# HID expansion acceptance scope

English · [简体中文](hid-development.zh-CN.md)

Agreed 2026-10-14: implement multiple firmware-only capabilities with self-describing MCP tools. New features must not depend on custom phone mappings, text replacements or enabling Full Keyboard Access. Existing USB network/AssistiveTouch/pairing/control setup remains. Anan source is outside scope.

Candidate `0.5.0-mcp`: preserve the six existing tools and add `keyboard`, `scroll`, `long_press`, `double_tap`, `drag_hold`, `draw_path`, `system_key`. Append Consumer report ID 3; retain mouse ID 1, keyboard ID 2, USB interfaces, power and legacy input timing. No dependency/license changes. No persistent key/button holds or automatic retry/replay.

Before implementation, define offline checks: schema/parser agreement and invalid arguments; named modifiers and forbidden activation repetition; exact wheel signs; double-click spacing; drag hold before movement; continuous path interpolation/endpoints; interruption during held input; unit progress; Consumer zero release and legacy report compatibility. These are host checks with simulated reports/time, not iPhone behavior or USB enumeration.

Separately authorized device acceptance: enumerate NCM/mouse/keyboard/Consumer together; legacy tap/drag/key/ASCII regression; small vertical and horizontal wheel deltas over lists; stable target long press/double tap/hold-before-drag; synthetic continuous drawing; normal keyboard/editing without Full Keyboard Access; Home/Search/keyboard toggle/media/volume usages individually. Test stop/cancel/detach mid-hold, release/no replay and MCP directory refresh in Anan. Use synthetic data, record unknown/partial results, timing/settings/context. Do not change phone bindings to make a failed usage appear supported. Focus navigation can be a separate optional test if already enabled.

Implementation/build results and device acceptance will be recorded separately. Firmware can report input dispatch, completed units and possible partial effects, not focus/App success. Consumer semantics follow the pinned Apple 2026-09-21 guide and locked TinyUSB APIs; [research](hid-capabilities.md) retains evidence and limitations.

## Tool consolidation scope (2026-10-14)

User approved reducing discovery from thirteen tools to seven: `device_status`, `tap`, `drag`, `press_key`, `type_ascii`, `scroll`, `stop`. Candidate `0.5.1-mcp` keeps old calls as compatibility entries outside `tools/list`; refreshing discovery is required. `tap` adds optional `count` (1/2), `holdMs` (0 or 100…2000) and `intervalMs` (double tap only); double long holds are rejected. `drag` uses 2…32 points, duration and optional pre-hold; exactly two points without hold retain old straight-drag timing. Old `x/y/toX/toY` drag arguments remain callable but are not advertised. `press_key` combines named keyboard and Consumer keys; raw `key` stays a hidden compatibility entry. USB descriptors, executor, power, grants, stop, dependencies and client code stay unchanged.

Defined before implementation: run the existing expansion fixture; add offline parser/executor scenarios for default tap timing, long/double dispatch and invalid combinations; two-point timing equivalence, held/multi-point strokes and ambiguous old/new drag arguments; named/Consumer key equivalence, repeat restrictions and all key mappings; exactly seven advertised schemas with concise descriptions and parser-aligned enums. No device actions are authorized. Separately authorized device acceptance would cover tool refresh in Anan, old calls, new merged arguments and interruption; compile/offline results do not prove them.

## Reproduce offline checks

After sourcing ESP-IDF v5.5.5, run `bash tools/check_hid.sh`. The pre-implementation fixture and minimal time/FreeRTOS/report stubs are in `tools/hid_acceptance/`. It compiles the actual action executor/parser against the pinned cJSON source; no USB functions or network calls execute. It is not a threading, SDK driver, client or real-device acceptance test.

The isolated candidate was built with `idf.py -B build/hid-merged-candidate build` and `python package.py build/hid-merged-candidate`; use that directory's manifest/image together. Existing cached builds from another checkout were not deleted or used as the candidate.
