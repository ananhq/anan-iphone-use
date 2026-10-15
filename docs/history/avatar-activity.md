# Input activity animation

English · [简体中文](avatar-activity.zh-CN.md)

Agreed before implementation: retain the approved black background, white eyes,
brows, touch and tilt style; give MCP input stronger choreography and small
colored props. This is a display-only slice, with no new MCP tool, client
integration, dependency, hardware initialization or license change.

Current follow-up: [responsive correction](avatar-responsive.md) removes all idle props, preserves live tilt during input, shortens feedback to about 0.6s and uses compact optimized repainting. The original 0.9.1 acceptance below is retained as history.

## Acceptance defined before runtime edits

- Tap/double tap: forward bounce, cyan cursor and one/two expanding rings.
  Hold: compressed/focused eyes, amber held disc and breathing ring. Drag/path:
  directional lean and a cyan cursor with at most six recent trail points.
- Scroll: large scanning motion and three blue moving cards. Typing: rhythmic
  face bounce and a rounded miniature keyboard with hopping keys. Keyboard/media:
  a large purple keycap with press/rebound. Interrupted input: asymmetric confused
  eyes and an amber question mark; never imply the phone task was completed.
- Short actions remain visible for at least one second, without delaying input.
  Running input stays active; release fades smoothly. New input replaces previous
  input with a brief visual crossfade, without an input/animation backlog. Bound
  motion, transitions, command count, trail storage and per-frame stack usage.
- Optional ambient waiting vignette: after six seconds without input or local
  touch, occasionally show rising blue bubbles becoming a small bulb. This is
  decorative waiting, not actual model-thinking telemetry. Do not animate status
  polling as input; physical/MCP stop still exits immediately through existing UI.
- Keep upstream tables/code and ordinary idle pixels intact. Transform the
  rendered face locally; props use existing primitives/scanline fills, no bitmap
  frames, framebuffer, per-pixel floating effects or full-screen flashing.
- Track old/new actual face bounds and a third prop region. Predefined synthetic
  fixtures compare dirty-region composition with full painting through every
  action, release, rapid replacement and ambient/touch transitions. They require
  colored prop pixels, distinct samples, visible short reactions, preserved idle,
  finite continuous transforms, no leftover trails and no command overflow.
- Reuse physical stop/control, UI, worker/HID timing, BLE/wireless, DMA, IMU,
  flash-helper and source-offer fixtures. Check target stack and pinned build/
  package, including a matching embedded source archive. Build a native preview.

Device acceptance requires separate authorization: actual visibility, smoothness,
input coexistence and stop latency. The user's positive report about the previous
avatar is user-reported feedback, not measurements of this new candidate.
