# Responsive avatar correction

English · [简体中文](avatar-responsive.zh-CN.md)

The user requests removal of idle bubbles/bulb and restoration of responsive
ordinary eyes when no MCP input is executing. They authorize the discussed
display/motion optimizations, without flashing or live device probes.

## Acceptance defined before implementation

- No idle activity scene or forced upward look at any elapsed time. Local touch,
  expression browsing and tilt remain available; status queries add no animation.
- MCP pointer/scroll attention is a small additive gaze lead, never a synthetic
  touch target that suppresses tilt. Actual finger touch retains its direct gaze.
- Replace the upstream 33ms-capped Euler interaction springs with bounded-target,
  analytic critically damped motion using actual elapsed time. Constant-target
  200ms motion must agree with five 40ms steps and be finite after long stalls.
  Eyes respond faster than the head. Reduce first-party gravity correction lag
  without changing IMU hardware address, bus/range/rate configuration or stop.
- Short input props hold approximately 480ms, then fade over 140ms, with a short
  post-execution tail. New props replace old props immediately; no old/new prop
  double image. Ongoing input stays visible and no input replay/delay is added.
- Raster only the requested compact rectangle, using its stride directly. Reuse
  the existing internal DMA buffer, increasing rows per transfer for narrow
  regions. Coalesce overlapping damage, skip semantically unchanged command
  groups and avoid full-width repaint/compaction for each cropped transfer.
- Predefined fixtures cover 20s idle without props, IMU influence during MCP,
  real-time spring response, compact/full raster equivalence for all input types,
  replacement/release/clock wrap, skipped identical frame transfers, and actual
  compact DMA stride/capacity/overlap/error/timeout behavior with a fake SDK.
  Keep ordinary stop/control, HID/wireless/IMU, source-offer and flash fixtures.
- Apply `-O2` only to the avatar hot path (no fast-math, global input optimization
  or new dependencies). Validate target stack and static RAM costs. Five-second
  diagnostics include actual DMA transfer/pixel counts, alongside existing frame
  and paint times, without coordinates or private input data.
- Preserve upstream origins/licenses and explicitly record the interaction
  modification. Update both language docs; build/package a separate candidate
  with matching source archive and native synthetic previews.

Device acceptance remains separate: measured cadence/latency, sensor noise,
coexistence, tearing versus persistent trails and stop latency. Offline raster
equality cannot establish scanout synchronization or hardware performance.

## Fine-trail correction: acceptance defined before runtime edits

The user reports smooth motion with thin residual lines on 0.9.2. The software
full/dirty raster checks pass, but they did not constrain physical panel windows.
An 80px-wide region uses a 93-row batch: an even initial Y becomes odd on the
next transfer. The native bounds also allow odd Y/heights. The pinned Espressif
driver forwards these addresses directly. The independently inspected
[Zephyr CO5300 driver](https://github.com/zephyrproject-rtos/zephyr/blob/2afc59f6d9784ad7013242d5477cba9745dde484/drivers/display/display_co5300.c#L125)
explicitly normalizes starts and dimensions to even values. This is a concrete
alignment gap; causation of the reported physical trails is still unverified.

- Round compact region starts down and exclusive ends up to even coordinates,
  clipped to the 466×466 panel. Render the expanded pixels from the current
  scene, including black background. Merge only after expansion.
- Every DMA chunk must retain even starts and dimensions; round its capacity
  down to an even row count, using the existing buffer and completion contract.
  Keep compact rendering, unchanged-frame skipping and the current motion.
- Extend the existing fake-panel fixture before implementation: reject any odd
  window; check full pixel coverage, without holes/duplicates, for odd origins,
  1px sizes/moves, all edges, full screen and varying narrow batch capacities.
  Reuse compact/full avatar and ordinary UI/stop/input checks; no added delay,
  periodic full-screen refresh, hardware initialization or dependency changes.
- Build/package a separate `0.9.3-avatar-edges` candidate with matching embedded
  source. No device operation is authorized here. Persistent-trail and tearing
  acceptance remains physical-device work.
