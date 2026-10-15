# Upstream avatar port

English · [简体中文](avatar-upstream.zh-CN.md)

The 0.9.1 [input activity layer](avatar-activity.md) adds larger face motion and small colored props while keeping this upstream engine and ordinary idle style intact.

The maintainer approved importing the AGPL-3.0-or-later engine on 2026-10-15.
This supersedes the independently approximated control face in 0.8.x. Source:
Trentct/m5stack-stopwatch-avatar, commit
`204963257cb4dc2f3d7501eff900897bac55ef82`.

## Acceptance defined before implementation

- Retain all twelve expression tables, keyframe timings, easing, blink, brow,
  projection and interaction calculations. Idle eyes use the upstream 1000-unit
  design, approximately 72 × 168 pixels, with centres 210 pixels apart.
- Adapt graphics/timing to native ESP-IDF, without Arduino, M5Unified, M5GFX,
  a full-frame sprite, display reinitialization, or changes to USB/BLE input.
  Use bounded primitive commands and scanline spans, preserving drawing order,
  eyelid masks and eyebrow strokes. Explicit overflow fails the frame closed.
- Offline fixtures exercise all expressions, bounded memory, nonempty/different
  output, brow primitives, dirty/full repaint equivalence and RGB565 ordering.
  Short completed MCP actions must still trigger a full expression timeline;
  continuous input must not repeatedly restart its first frame.
- Tap → Happy; double tap → Surprised; hold 650ms → Angry. Horizontal swipes
  browse expressions; vertical swipes select Surprised/Sleepy. Drag follows the
  finger with upstream swipe transitions. Tilt retains the existing optional
  BMI270 adapter. Local face gestures never execute phone input.
- Fresh Start resets the face/event baseline. Either front key immediately
  revokes input and returns Home, consuming the press. Retain existing read-error,
  disconnect and MCP revocation. Do not change descriptors, worker delays,
  authorization comparisons, transport or board power.
- Aim for the upstream 16.667ms cadence including work already spent, retaining
  a low-priority task and five-second update/paint diagnostics. This is a target,
  not a device FPS claim. Check target-compiler stack budgets before packaging.
- Preserve original license/source notices and mark modifications. Supply an
  on-device source-download link on both HTTP/HTTPS setup pages, containing this
  build's source/configuration/licenses/build scripts, dependency pins and
  installation instructions. Archive an explicit allowlist, never credentials,
  generated device configuration, .git, machine paths, logs or unrelated notes.
  Verify deterministic archive contents offline before embedding/packaging.

Device acceptance (requires separate authorization): compare idle proportions,
single/double/hold/swipes, tilt neutral/directions, each MCP action family and
rapid alternation on the board; record actual update/paint timing, input
coexistence, physical stop latency and source-download success. Compilation and
synthetic renders do not establish these outcomes.
