# Interactive control avatar

English · [简体中文](avatar.zh-CN.md)

**Current implementation:** `0.9.0-upstream-avatar` replaces the earlier approximate face with the approved [upstream engine port](avatar-upstream.md), including the original eye proportions, twelve expressions, brows and touch timelines. This page retains the 0.8.x baseline/IMU acceptance record below; its inspiration-only and 30fps statements are historical. See [source and license handling](../../SOURCE.md).

Scope discussed before implementation of `0.8.0-avatar`: native rounded eyes and
eyelids, smooth transitions, BMI270 tilt and local touch feedback, with visible
MCP reactions. Keep normal Home/settings, pure-black control UI, either front key
stop, fresh grants, input timing/reports and MCP schemas. No client changes.

## Design and sources

The reference is [Trentct/m5stack-stopwatch-avatar](https://github.com/Trentct/m5stack-stopwatch-avatar/tree/204963257cb4dc2f3d7501eff900897bac55ef82),
commit `204963257cb4dc2f3d7501eff900897bac55ef82`, AGPL-3.0-or-later. Its separation
of pose/expression/interaction, faster gaze/slower head, transition continuity
and changed-area updates inform this design. No avatar code, expression tables
or artwork are imported. This remains the native ESP-IDF project, without
Arduino, M5Unified, M5GFX or a project-license change. The reference's reported
frame rates are its measurements, not this firmware's performance.

StopWatch C152 has BMI270 at I2C 0x68 on SDA47/SCL48, confirmed by
[official specifications](https://docs.m5stack.com/en/core/StopWatch).
The official factory `main/hal/hal_imu.cpp` at
`6b4aa125288b6fe9dca661f10159f6e1e5ee785c` swaps sensor X/Y into screen axes.
Use the Bosch SensorAPI at `41129fcfe39c583ee5462d79195741945d51c1fe`, base BMI270
v2.86.1: unmodified `bmi2.c/h`, `bmi2_defs.h`, `bmi270.c/h` including its sensor
configuration array, under BSD-3-Clause; exact hashes/notices in THIRD_PARTY.

- A bounded low-priority IMU task initializes and samples the sensor. A pure
  gravity/gyro filter produces bounded screen-relative tilt; it does not claim
  compass heading. Neutral is established on entry, with no calibration UI.
  Invalid/stale data relaxes toward center; IMU failure does not block MCP.
- Existing worker notifications retain their action feedback. A screen-owned
  state moves continuously toward each target, including interruption, using
  damped motion. The eyes lead the slower face offset. No render/state waits
  enter input execution; physical stop stays independent of I2C/rendering.
- MCP expression wins over touch reactions. Ambient tilt remains subtle during
  input. Touching/dragging the control face changes only the avatar's attention
  and a short reaction; it never sends phone input, opens settings or grants/
  revokes input. Read errors retain existing fail-closed revocation.
- Initial face entry and leaving it redraw fully. Subsequent frames repaint the
  union of old/new eye bounds, including clearing old pixels, through the existing
  DMA strip buffer. Target 30fps scheduling includes render time; actual speed,
  animation quality and power consumption need board measurements.

## Acceptance defined before runtime edits

- Pure fixtures: finite/bounded tilt, screen axis mapping, stationary drift,
  zero-gravity/motion rejection, clock wrap and stale-data recovery; avatar
  geometry inside the round display, rounded corners/lids, touch feedback,
  MCP priority, distinct responses, gradual interruption/release and reset.
- Fake-display execution of actual cropped transfer: byte order/stride, bounds,
  partial final strip, no transmission outside the region, failure propagation
  and no buffer reuse after uncertain DMA completion. Compose successive dirty
  frames and compare with full raster frames to catch trails/clear errors.
- Fake I2C/SDK execution of the actual IMU adapter, including missing sensor,
  init/read failure and bounded callbacks; verify vendor source hashes.
- Extend actual control/touch fixtures before edits to allow local face touch
  notifications without a grant/stop/navigation change. Reuse action timing,
  physical-stop, black-only UI, stack-budget and HID/wireless/flash checks.
- Pinned compile/package and native static/GIF previews; no post-implementation
  unit tests. Separately authorized device acceptance: tilt directions/neutral,
  held angle, touch/drag/release, each input family, rapid interruptions,
  stop/detach, Settings navigation, no trails, measured cadence and power.

No flash, serial/radio probe, phone/client/provider or E2E is implied by this
implementation scope. Sensor orientation, touch feel and input coexistence remain
device-unverified until that acceptance.
