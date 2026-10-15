# BLE held-pointer correction

English · [简体中文](ble-drag.zh-CN.md)

## Scope and acceptance before implementation

The user reports that consecutive Notes drawing strokes work with USB but only
the first stroke appears with BLE, while the pointer continues moving. This is
a user report, not a reproduced iPhone trace. Before `0.7.4`, the BLE absolute report
always carries buttons=0; the separate relative report carries buttons only on
changes. The two report streams therefore disagree during held movement. The
existing parser/executor offline scenarios pass but do not model iOS HID state.

Candidate `0.7.4-ble-drag` will carry the requested button state in both existing
reports and send the zero-delta relative button state after each absolute move.
Release will clear a held absolute report as well as the relative report, at the
last reported coordinates. Relative release remains first on cancellation.
Preserve report descriptors/IDs, coordinate scaling, tools/schemas, USB, action
pauses, pairing, power, grants and no-replay behavior. No new dependency, client
change or iPhone setting is required. Extra BLE notifications add unmeasured
transport latency; this change does not guarantee drawing or pixel accuracy.

Before implementation, extend payload fixtures and add a host fixture extracting
the actual BLE sender/release functions from source, with simulated notification
completion only. Require equal held/released states in both reports, zero
relative movement, three independent strokes, release after partial send,
bounded failure without replay and cleared connection state. Run existing HID
and wireless checks, pinned-IDF compile/package and manifest/source preservation
checks. No tests will be added after implementation.

Separately authorized device acceptance: draw at least three separate strokes in
an empty Notes drawing; compare USB/BLE with identical parameters; interrupt a
held stroke and verify no stuck button or replay. No device access is authorized
by this source/build scope. The iPhone's handling of the corrected reports and
the original failure cause remain unverified until that run.

## Candidate results

The pre-fix sender failed the predefined report-pair assertion. With the
correction, actual-sender host scenarios, existing HID/wireless scenarios and
bond-preserving flash scenarios passed. ESP-IDF v5.5.5 compilation/package
completed with no compiler warnings; image/source hashes and the preserved
USB/executor/tool/descriptor/security boundaries were checked. See the
[sanitized record](../../artifacts/wireless/ble-drag-build-check.json).

The generated image is `build/ble-drag-candidate/anan-stopwatch-flash.bin`.
The descriptors and partition layout did not change. No agent-executed flash or
actual Notes drawing validation ran.

## User feedback: drawing still fails

After the candidate handoff, the user reports the same BLE failure: only the
first Notes stroke appears and later movement leaves no ink. The exact flashed
hash was not independently verified. Record `0.7.4` as an unsuccessful proposed
Notes fix, not as an accepted drawing implementation. Passing the host fixtures
establishes the intended bytes and release paths, not iOS gesture delivery.

A more relevant hypothesis is that pointer movement and the AssistiveTouch
simulated finger follow different HID streams. The ghosthand author's
[relative-pointer measurements](https://ghosthand.dev/docs/relative-pointer#drags-on-ios-17-and-later)
report that, on an SE2 running iOS 18.6.2, absolute reports move the pointer
while held relative-button drags do not follow those positions; relative motion
is required for the simulated finger. Its current product uses Bluetooth
Classic, while this board uses BLE; this is a research lead, not proof for the
user's phone. Our relative X/Y remain zero even after `0.7.4`.

Before another firmware candidate, distinguish click failure from held-motion
failure using the failed tool arguments/results and whether ordinary taps still
work. An isolated relative-motion experiment could distinguish the streams.
A permanent hybrid drag would be a material input-path change: normalized
coordinates need conversion to relative deltas, accounting for viewport size
and iOS pointer acceleration. Discuss that design first; do not guess a fixed
gain, promise exact drawing, or treat screenshot/model observations as available
to the firmware. No new relative-motion implementation or device probe ran.
