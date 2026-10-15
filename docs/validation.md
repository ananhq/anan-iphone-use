# Migration validation

English · [简体中文](validation.zh-CN.md)

## Defined acceptance scope

This migration authorizes local extraction, documentation, compilation and static checks. It does not authorize flashing, phone/HID/model operations, simulator/E2E or publication. Before implementation, the checks were defined as: preserve original runtime/build inputs by SHA-256, independently build/package outside [Anan](https://anan.sh), check target/offset/Flash/lock/source manifest, check bilingual links/ignore rules and record provenance gaps. A–H device scenarios from Anan are not automatically approved here.

[source-snapshot.json](../artifacts/migration/source-snapshot.json) identifies the original 16 files; all 12 original build inputs matched the historical image manifest. Runtime source, scripts, lock and wire behavior are unchanged. Root CMake pins the original IDF app version `ec6bb29-dirty`, previously inferred from Anan Git; this is the only build-configuration adjustment. Added module README files are extra manifest entries because package.py hashes all main/ files; they are not compiled sources.

## Evidence levels

| Evidence | Status |
| --- | --- |
| Original image | 455840 bytes, SHA-256 `5e3e96b3bf2e1699c6406874de66a85e7efa1e3823ec83738f4971b0f2d8e230`; source hashes checked at extraction |
| Historical flash | User supplied esptool successful-write/hash output; not a new migration device run |
| Historical behavior | User reported Ethernet, static IP, connection, preview, observe and phone input working |
| Whole task/endurance | Notes task difficult; background improvements reported on Anan; no full firmware acceptance trace |
| Migration build/static | See [build-check.json](../artifacts/migration/build-check.json) for commands/tool versions/results; not device acceptance |
| Provenance | Partial review, USB implementation origin unresolved |
| Local checkpoint | Explicitly authorized after the user's successful reflash report; full input/stop acceptance and source resolution remain pending |

Generated binaries, components, sdkconfig and raw machine-local build logs stay ignored. Committed evidence stores sanitized outcomes/source hashes, not private paths, serials, screenshots or credentials. Cross-machine binary identity is not promised.

## Baseline preparation

Preparation evidence is [preparation-check.json](../artifacts/baseline/preparation-check.json).
The user subsequently supplied esptool successful-write/hash-verification output
and explicitly requested the first local commit. The sanitized current scope is
recorded in [checkpoint.json](../artifacts/baseline/checkpoint.json).
Migration records above remain historical. The candidate's manifest is generated
under ignored `build/`; use the [baseline procedure](baseline-acceptance.md) to
report results against that exact image hash. Phone input/stop acceptance and
source-origin resolution remain pending; neither a stable baseline nor public-release readiness is
claimed. [Release preparation](release-preparation.md) lists the remaining gates.

## Next device acceptance, separately authorized

Use the migrated image hash and synthetic notes: Ethernet/HID enumeration and 60s READY/PONG; full-screen portrait preview; observe-only; one tap/ASCII/drag with visual confirmation; stop/button/detach during input and no new actions/replay; no-Mac multi-step closure. Record board revision/OS/client/model only as appropriate, preserve failures and unknown partial effects. Store repeatable sanitized artifacts under `artifacts/e2e/<scenario>/<run-id>/` if E2E is approved. Until that run, keep deviceVerified=false.

## MCP development candidate

[build-check.json](../artifacts/mcp/build-check.json) records the separate
`feat/device-mcp` current candidate. The original `0.1.0-mcp` build remains in
[initial-build-check.json](../artifacts/mcp/initial-build-check.json). Compilation/packaging and static
configuration, original-action-body equivalence, embedded JSON/JavaScript,
DER structure and documentation checks passed. `usb.c` and PROTOCOL.md match
the local checkpoint byte-for-byte; new arbitration/UI/identity code is separate.
[Device MCP](mcp.md) documents setup and runtime limits. No new flash, phone,
display, TLS trust, interoperability, latency or endurance run was performed.
Runtime heap peak is unmeasured. Existing provenance and acceptance gaps remain.

The user subsequently reported a blank display; exact flashed image/reset state
were not established. Candidate `0.1.1-mcp` adds expander wake/settling/sleep
configuration and verified power writes, plus read-only USB startup diagnostics.
Its compilation/static checks are recorded separately from that report; no
verified device fix or new device action is claimed.

The user then reported the screen lighting and `USB origin/host required` while
accessing the phone setup flow. Exact image/flash trace was not supplied.
Pinned IDF HTTP/lwIP source plus current IPv6 configuration explain a separate
address-family defect: the dual-stack listener maps IPv4 into `AF_INET6`, while
the adapter formerly accepted only `AF_INET`. Candidate `0.1.2-mcp` normalizes
both representations to the unchanged exact USB addresses, permits equivalent
HTTP default-port spelling, and returns fixed rejection categories. Static/build
results are recorded; successful phone enrollment/trust/MCP input remains pending.

Candidate `0.2.0-mcp` implements the user's touch-first and device bilingual
switching request. It retains the USB/ANAN1 source contract, adds a native
CST820B adapter, shared widget layout/hit regions, frame/gesture authorization
checks and separate language storage. Compilation/package, both-language glyph
coverage, 22 static document previews and layout/text bounds passed. The font
source/derivation/OFL notice is recorded. These synthetic previews are not phone
or board screenshots. Touch, actual stop latency, preference persistence and
MCP phone acceptance remain unverified; no new device action was performed.

Candidate `0.2.1-mcp` implements the user-approved removal of the three-minute
control grant. Grants remain volatile and stop/detach/reboot-revoked; other
revocation paths and session/action bounds remain. Build/static/layout checks
are recorded separately; no longer-than-180-second device run is claimed.

The preceding touch build is preserved in
[touch-ui-build-check.json](../artifacts/mcp/touch-ui-build-check.json);
[build-check.json](../artifacts/mcp/build-check.json) now identifies `0.2.1-mcp`.

Candidate `0.3.0-mcp` adds HTTP80 MCP and physically confirmed same-page config
export while retaining optional HTTPS. HTTP onboarding no longer requires iOS
certificate trust. Compile/package, static both-language layout/glyph and
JavaScript/source/links checks are recorded in the current MCP build record.
The preceding continuous-control build remains in
[continuous-control-build-check.json](../artifacts/mcp/continuous-control-build-check.json).
No new flash, certificate removal, board reset, HTTP phone interoperability or
clipboard/device run was executed by the agent; those remain user device checks.

The user subsequently reports the HTTP first-user flow verified and working
well; see [sanitized feedback](../artifacts/mcp/http-user-feedback.json). This
is a user report, without an independently checked image hash or full scenario
trace; it does not establish acceptance of the next candidate.

Candidate `0.4.0-mcp` removes ANAN1 TCP28766, parsing/heartbeat/legacy arbitration
and control-source UI. It retains USB/HID input sequences and initializes MCP
without automatically granting control. The prior HTTP build is preserved in
[http-onboarding-build-check.json](../artifacts/mcp/http-onboarding-build-check.json).
Source/symbol/build/package/document-render checks are separate from a new
flash/device run; current MCP-only acceptance remains pending.

## HID expansion candidate

Candidate `0.5.0-mcp` implements thirteen MCP tools including seven named keyboard/scroll/gesture/Consumer tools; no client changes or new phone bindings/FKA setup. Existing mouse/keyboard IDs and legacy executor/sender bodies are retained; Consumer ID3 is appended. [Predefined acceptance](history/hid-development.md) and first-party offline fixtures preceded implementation. Full pinned-IDF compilation/packaging and simulated input/cancellation/progress checks passed; [sanitized record](../artifacts/hid/expansion-build-check.json) separates them from device behavior.

No flash, phone input, provider, client connection or E2E was executed. Consumer enumeration/mappings, wheel/gesture recognition, new-tool Anan interoperability, stop latency and endurance remain device-unverified. Prior provenance and acceptance gaps remain; historical `usb.c` byte-identity statements above apply to earlier candidates.

## Tool consolidation candidate

Candidate `0.5.1-mcp` advertises seven tools, merging stationary gestures into `tap`, held drags/continuous paths into `drag` and named keyboard/Consumer keys into `press_key`. Firmware retains hidden legacy names and old drag arguments; clients must refresh discovery and may locally reject old names or new parameters. Default tap and unheld two-point drag retain prior timing. Descriptions are concise; shared rules appear in both server description and initialize instructions. Read-only inspection found Anan consumes the server description, so the design does not depend on its consuming instructions.

Predefined consolidation scenarios passed with simulated reports/time; seven schemas/full key enums, unchanged executor/USB/power/grants/stop/dependencies and bilingual documentation checks passed. Full pinned-IDF compile/package outcomes are in the [current record](../artifacts/hid/build-check.json); the prior [expansion record](../artifacts/hid/expansion-build-check.json) is preserved. No flash, phone actions, client connection, provider or E2E ran; merged Anan discovery/calls and device behavior remain unverified.

## Wireless candidate

Candidate `0.6.0-wireless` adds Wi-Fi station HTTP MCP and experimental BLE HID,
explicit input selection and USB-first Wi-Fi configuration. Pinned-IDF compile,
package, unchanged USB sender/descriptor checks, seven-schema comparison,
predefined offline routing/payload scenarios, existing HID scenarios, browser
script syntax and 28 bilingual static layouts passed. Report mutex behavior and
radio/phone delivery are not established by host simulation. See
[predefined scope](history/wireless-development.md), [setup](wireless.md) and
[sanitized build record](../artifacts/wireless/build-check.json). No flash,
Bluetooth/Wi-Fi device probe, phone input, client/provider or E2E was executed.

## Wi-Fi scan candidate

Candidate `0.6.1-wifi-scan` adds paired USB-only asynchronous scans, bounded and
deduplicated names, scan/select/manual setup and open-network password handling.
Acceptance was defined before implementation. Pinned-IDF compilation/package,
existing offline HID/routing/report scenarios and synthetic browser selection,
literal hostile SSIDs, manual/open-network and failure recovery checks passed.
No real Wi-Fi scan, flash, phone action or E2E was executed. See
[scan record](../artifacts/wireless/scan-build-check.json). Radio timeout/coexistence
and reconnect behavior remain device-unverified.

## Wireless setup candidate

Candidate `0.7.0-wireless-setup` adds temporary WPA2 SoftAP/DHCP configuration, five-network storage with v1 migration and retry cycling, and one fixed HTTP hostname on USB/Wi-Fi through the already pinned lwIP responder. [Acceptance](history/wireless-setup-development.md) and storage/authority/browser scenarios were defined before implementation. Pinned-IDF compilation/package, existing HID/routing/report scenarios, migration/capacity/removal and exact-Origin fixtures, USB/hotspot synthetic scan UI, JavaScript syntax, 32 static bilingual layouts, preserved seven schemas/USB HID sender bodies and manifest checks passed. USB now adds IGMP; sdkconfig reserves one mDNS client slot. No dependency upgrade or project license change; the additional Verisure BSD notice is retained.

The [sanitized record](../artifacts/wireless/setup-build-check.json) marks flashed/deviceVerified false. No serial/radio probe, phone input, live client/provider, simulator or E2E was executed. The user reported the prior wireless flow working; it does not establish this candidate's DHCP/mDNS, AP+STA scanning, multi-network timing, radio coexistence, runtime heap, grant/stop or endurance acceptance. Numeric recovery paths remain.

Candidate `0.7.2-wifi-qr` follows the user's failed `.local` report with a source-reproduced fix for the pinned lwIP timer's mandatory IPv6 probe on IPv4-only netifs, and adds a screen Wi-Fi QR while retaining random WPA2 credentials and physical pairing. Timer regression and QR scenarios were defined before the changes. SDK compile/package, existing offline scenarios, independent QR and full static round-screen decoding passed; [record](../artifacts/wireless/qr-build-check.json). No agent-executed flash, radio/serial probe, phone camera/joining, live client/provider or E2E ran. IPv4 name-conflict behavior and new phone `.local` success remain device-unverified.

## Task-oriented discovery metadata

Candidate `0.7.3-discovery` changes only seven tool descriptions, initialize description/instructions and version, with bilingual usage terms and optional full-group overview guidance. Acceptance was defined first in [tool discovery](history/tool-discovery.md). Embedded JSON, exact name/order/schema/annotation preservation, MCP code equivalence outside its two metadata statements, preserved input/HID/radio/storage/dependencies, existing offline HID scenarios, pinned-IDF compile/package and manifest/link checks passed. Tool descriptions increased from 1,373 to 1,525 characters; server description is 463 characters, below the inspected Anan 500-character limit. These are character counts, not measured model tokens. See [record](../artifacts/discovery/build-check.json). No client code, new unit tests, flash, device/provider or E2E execution. Search/model adoption and real app-opening outcomes remain unverified; metadata guidance does not load schemas or grant input.

## Bond-preserving flash helper

The user reports reconnection failure after reflashing. The [predefined scope](flashing.md) adds a first-party host helper that splits existing merged images around default BLE `nvs`; partition layout, firmware and BLE security remain unchanged. Predefined offline sector-write scenarios, image/layout rejection, failure propagation and syntax checks passed. The existing candidate was repackaged with refreshed source hashes and identical binary SHA-256. See [record](../artifacts/flashing/check.json). No firmware recompilation, device access or actual iPhone reconnection verification ran; previously erased bonds require one new pairing.

## BLE held-pointer candidate

Candidate `0.7.4-ble-drag` follows the user report that USB draws consecutive Notes strokes while BLE only draws the first. [Scope and fixtures](history/ble-drag.md) preceded implementation; the pre-fix actual sender failed the expected report-pair assertion. Consistent button payloads, three strokes, cancellation/partial-send release and no-replay host checks passed, along with existing HID/wireless/flash checks. Pinned-IDF compile/package had zero compiler warnings. USB, executor pauses, tool schemas, descriptors, pairing and dependencies are unchanged. See [record](../artifacts/wireless/ble-drag-build-check.json). Additional BLE notifications add unmeasured latency. No flash, device/phone action or E2E ran; Notes behavior and physical-stop latency remain device-unverified.

The user subsequently reports that the BLE first-stroke-only failure persists after the candidate handoff; the exact flashed hash was not independently verified. `0.7.4` is not an accepted Notes fix. See the [feedback and relative-motion hypothesis](history/ble-drag.md#user-feedback-drawing-still-fails); no additional firmware/device run followed this feedback.

## Single BLE absolute-mouse comparison

Candidate `0.7.5-ble-absolute` implements the user-approved comparison after the previous Notes fix failed. [Scope and host fixtures](history/ble-absolute.md) preceded code changes. BLE removes relative report ID4 and uses a single position/button/wheel report; input subscription readiness and keyboard output references match the three inputs. Predefined sender/readiness/stroke/wheel/release/no-replay scenarios and existing HID/wireless/flash checks passed. Pinned-IDF compile/package had zero compiler warnings; USB/BLE maps extracted from the ELF match byte-for-byte (172 bytes). USB, executor pauses, seven tools, security/storage and dependencies are unchanged. See [record](../artifacts/wireless/ble-absolute-build-check.json). Re-pair for the changed BLE map; actual taps, drawing and stop behavior remain device-unverified. No flash, device probe, phone/client/provider action or E2E ran.

## Large-target UI and Home battery

Candidate `0.7.6-touch-ui` implements the reviewed menus and Home-only coarse
battery estimate. [Acceptance](history/touch-usability.md) and host fixtures preceded
firmware changes and remained unchanged. Actual control/touch functions with
synthetic reads/time passed parent/cleanup, jitter, reset-hold, stale/error and
stop/no-same-contact-grant scenarios. The read-only M5PM1 adapter and estimate
passed register/endian/timeout/failure/range checks. Existing HID/wireless/flash
checks, 46 shared-layout bilingual renders, 242 label bounds, regenerated glyphs,
independent full-frame QR decoding and pinned SDK compile/package passed with
zero compiler warnings. README illustrations were replaced in place, and setup
paths updated in both languages and the setup skill. See [record](../artifacts/ui/usability-build-check.json).
USB/BLE/HID/MCP/radio/storage sources, physical stop and revoke bodies are retained.
No flash, device probe, phone/client/provider action, simulator or E2E ran.
Usability, battery accuracy, shared-I2C responsiveness and the preceding BLE
drawing comparison remain device-unverified.

## Button-only touch stop

The user explicitly replaced touch-anywhere stop in `0.7.7-button-stop`.
The [predefined follow-up](history/touch-usability.md#button-only-touch-stop-agreed-follow-up-before-implementation)
and extended actual-function host fixture preceded firmware edits; the previous
candidate failed the new blank-touch assertion. Blank/status/battery/busy touches
and sliding into Stop are now ignored; only a fresh press inside the displayed
Stop target revokes input, consuming the contact through release. Read-error,
physical-key, MCP and disconnect revocation are retained. The predefined UI,
HID/wireless and synthetic browser checks, JavaScript syntax, 46 bilingual
renders, 242 label bounds, pinned build/package and manifest checks passed with
zero compiler warnings. [Record](../artifacts/ui/button-stop-build-check.json).
No flash, device probe, phone/client/provider, simulator or E2E ran; targeting
and stop latency remain device-unverified.


## Input-driven control eyes

Candidate `0.7.8-eyes` implements the [agreed pure-face scope](history/eyes.md): Start
shows only two animated eyes on black; either front key stops and returns Home,
consuming the press through release. Control-view touches are ignored; read-error
revocation remains. Predefined fixtures preceded runtime edits and the previous
candidate failed the empty-active-layout assertion. Shared action-header linkage
required a mock-linkage adjustment; no new post-implementation scenarios were added.
Actual UI strip painting, geometry/lifecycle/time-wrap, actual control/touch/key
functions with synthetic SDK/time, existing HID/wireless/flash checks, 46 bilingual
renders, 230 label bounds, 100 glyphs, two independently decoded full-frame QR
previews and pinned compile/package passed with zero compiler warnings. USB/BLE
sender/report, MCP and display sources and stop/key/executor/pause bodies are
retained. See [sanitized record](../artifacts/ui/eyes-build-check.json).
No flash, device probe, phone/client/provider, simulator or E2E ran. Actual
appearance/frame rate, shared-bus responsiveness, key stop latency, input under
animation load and the preceding BLE drawing issue remain device-unverified.


## UI task stack correction

After the eyes candidate, the user reports a green flash/no response on Home's
Settings. No device log or exact flashed-image confirmation was collected.
`0.7.9-ui-stack` addresses a target-compiler-confirmed stack-budget defect: the
known touch/layout/libc path uses 4288 bytes against 4096 allocated; the key path
uses 4144. Both tasks now allocate 8192 bytes (8 KiB total increase). The
[predefined check](history/eyes.md#ui-stack-correction-defined-before-implementation)
failed before the runtime edit and passes after it, including a 2 KiB reserve;
this is not a measured runtime high-water mark or exhaustive worst-case proof.
Existing UI/HID/wireless/synthetic-flash checks and pinned compile/package pass
with zero compiler warnings. The only runtime edit is the two task allocations;
UI/input/stop bodies are unchanged. See [record](../artifacts/ui/ui-stack-build-check.json).
No flash, device probe or phone/client/provider/simulator/E2E ran; the reported
symptom's cause and successful Settings navigation still require device confirmation.


## Stronger eye action feedback

The user reports Settings navigation works after the stack candidate, but input
reactions resemble idle blinking; the exact flashed image and display cadence
were not verified. `0.7.10-eyes-feedback` strengthens only the eye sampler's
geometry and visual lifetime. The [predefined checks](history/eyes.md#stronger-action-feedback-defined-before-implementation)
preceded runtime edits; the previous first pose failed the visibility assertion.
The actual queued worker publishes action feedback with synthetic tap/type/key
execution times retained at 70/30/30ms. All action families have distinct initial
poses and at least four visible samples at synthetic 160ms cadence, including
20ms actions; geometry/scroll bounds, lifecycle, native painting, control/stop,
existing HID/wireless/synthetic-flash, target-stack checks, 46 bilingual renders
and pinned compile/package pass with zero compiler warnings. Worker/input/device/
display sources are unchanged. See [record](../artifacts/ui/eyes-feedback-build-check.json).
No flash/device probe, phone/client/provider, simulator or E2E ran; actual
appearance and rendering/input coexistence remain device acceptance items.


## Interactive avatar candidate

`0.8.0-avatar` implements the [discussed scope and predefined acceptance](history/avatar.md):
rounded eyes/eyelids, continuous damped transitions, faster gaze/slower face,
BMI270 gravity/gyro tilt, local face touch and MCP expression priority. Native
changed-area updates retain the existing DMA strip buffer; 30fps is a scheduling
target, not a measurement. The exact Bosch v2.86.1 files/configuration and BSD
notice are recorded; the AGPL avatar reference supplies design inspiration only.
Predefined pure tilt/geometry/transition/priority fixtures, actual cropped-DMA
and IMU/touch adapters with fake SDK/I2C, dirty/full raster equivalence, previous
UI/HID/wireless/synthetic-flash checks, target stack analysis, 46 bilingual renders
and pinned compile/package pass with zero compiler warnings. Physical stop/key/
authorization comparison bodies and HID/MCP/worker/power sources are retained.
See [record](../artifacts/ui/avatar-build-check.json). No flash/device probe,
phone/client/provider, simulator or E2E ran. Actual frame cadence, appearance,
sensor axes/neutral, latency, shared-bus coexistence and battery impact remain
separately authorized device acceptance items.


## Avatar raster correction

The user reports low frame rate and weak reactions after the avatar candidate;
no exact flashed-image hash, live frame timings or notification trace was
collected. Source comparison confirms different proportions/no full brow or
click-expression timelines, plus a per-pixel float shader and combined-eye
transfer overhead. `0.8.1-avatar-fast` implements the [independent predefined
performance correction](history/avatar-performance.md): analytic per-row white spans,
gray edge, per-eye dirty transfers and five-second timing summaries. It preserves
the native geometry/input behavior; upstream AGPL expression import awaits the
user's license decision and is not claimed complete. Analytic raster, separate-
eye/full-frame equivalence, synthetic timing-window, prior UI/IMU/input/wireless/
flash and target-stack checks plus pinned compile/package pass with zero compiler
warnings. See [record](../artifacts/ui/avatar-fast-build-check.json). No device run
or actual performance improvement is established; frame rate and input coexistence
remain device acceptance items.

## Approved upstream avatar port

`0.9.0-upstream-avatar` replaces the independently approximated face after the
maintainer approved AGPL import. [Predefined scope](history/avatar-upstream.md) and
fixtures preceded runtime edits. The pinned upstream tables, timelines, easing,
blink, projection and drawing are retained; the exact documented patch was
verified against the pinned source. A bounded native primitive/scanline adapter
replaces M5Unified graphics, while actual queued input notifications drive
visible timelines. Single/double/hold/swipe gestures stay local. The prior
physical stop, display DMA completion handling, HID timing/descriptors, board
power, IMU adapter and MCP tool metadata are retained.

All twelve expressions, brows, dirty/full-raster equivalence, short-action/reset
and touch lifecycles, overflow rejection, existing UI/IMU/HID/wireless/synthetic-
flash scenarios, 46 bilingual previews, target stack budgets (including C++
render/reset paths), pinned compile/package and source-manifest checks passed,
with zero compiler warnings. The deterministic allowlisted source archive is
embedded in the image, linked from both setup pages and served by static GET
routes; its contents/hash are verified offline. No live download is claimed.
See [sanitized record](../artifacts/ui/upstream-avatar-build-check.json),
[source/build information](../SOURCE.md) and exact origins in THIRD_PARTY.

No flash, device probe, phone/provider, simulator or E2E ran. Approximately 60fps
is a scheduling target; actual cadence, appearance, gesture feel, stop latency,
input coexistence, battery impact and network-source download remain device
acceptance items. Existing provenance/linked SDK notice-review limits remain.

## Input activity candidate

The user reports liking the 0.9.0 avatar and asks for stronger MCP feedback while
keeping its style. `0.9.1-avatar-activity` implements the [agreed scope and
predefined fixtures](history/avatar-activity.md): face bounce/lean, cyan cursor/rings and
six-point trail, blue scroll cards, miniature typing keyboard, purple keycap,
amber interruption mark and occasional decorative waiting bubbles/bulb.
Waiting is not model telemetry; input completion is not task verification.

Predefined short-action, colored-prop, bounded motion, rapid crossfade/reset and
dirty/full-raster fixtures pass (52 commands observed, capacity160). Initial
checks caught and corrected an unsigned age in bounce decay; target compilation
caught and corrected a host/ESP32 integer-template type mismatch. Three ordinary
idle preview samples match the preceding candidate byte-for-byte. Prior UI,
input/stop, worker/HID timing, BLE/wireless, DMA/IMU, synthetic flash and source-
offer checks pass. Native 46-scene bilingual previews and a synthetic 10fps GIF
were generated; that GIF cadence is not a board measurement. Target UI/activity
stack budgets and pinned compile/package/source hashes pass with zero compiler
warnings. [Sanitized record](../artifacts/ui/activity-avatar-build-check.json).
The executor, input notifications, HID/BLE, MCP metadata/server, power, display
DMA, IMU, device stop/control code and upstream engine remain byte-identical to
the previous candidate. No flash, device probe, phone/provider, simulator or E2E
ran; actual colors, cadence, visibility, coexistence and stop latency remain
separate device acceptance items.

## Responsive avatar correction

After 0.9.1, the user reports choppy animation, trails and sluggish tilt. No live
timing/scanout trace was collected. `0.9.2-avatar-responsive` implements the
[agreed correction and predefined fixtures](history/avatar-responsive.md): remove idle
bubbles/bulb, keep IMU tilt active with additive MCP attention, use actual-time
analytic interaction springs, shorten short props to 480ms plus a 140ms fade,
remove double prop layers, cache unchanged commands and paint compact clipped
regions with larger DMA batches/overlap merging. Only avatar files use -O2;
input/USB/BLE flags and timing remain unchanged. Sensor hardware configuration
is retained; first-party gravity correction is more responsive. Upstream changes
are limited to `updateInteraction`, with origins/modifications recorded.

Predefined fixtures pass for 20s plain idle, live tilt during MCP, finger gaze,
200ms/five-40ms spring equivalence, stall/wrap/reset, shorter props/no overlap,
compact/full raster equivalence and fake-SDK compact DMA stride/capacity/merge/
failure/completion handling. Synthetic idle needs 9 repaint frames over 71
updates; an 80×180 region needs two transfers versus the previous twelve. These
are synthetic operation counts, not measured device FPS. Optimized host checks,
prior UI/stop/worker/HID/BLE/wireless/DMA/IMU/source/synthetic-flash checks,
46 bilingual previews, target stack and pinned compile/package/hash checks pass
with zero compiler warnings. Static DRAM grows from 90,112 to 99,144 bytes for
command caches; the 14,912-byte DMA buffer is retained. This is not runtime heap
measurement. Five-second diagnostics now include actual transfer/pixel totals.
See [sanitized record](../artifacts/ui/responsive-avatar-build-check.json).

Physical stop/key/touch/grant bodies and input/worker/MCP/power/sensor transport
sources are retained. No flash, device probe, phone/provider, simulator or E2E
ran. Device cadence, tilt latency/noise, coexistence, scanout tearing versus
persistent trails and stop latency remain acceptance items. The offline image
checks cannot establish synchronization with physical panel scanout.

### Fine-trail follow-up

The user reports that 0.9.2 is smooth but leaves thin trails. Before runtime
edits, the compact-DMA fixture was extended to reject odd window boundaries;
the existing code failed on its first 80px-wide, 93-row batch. The separately
packaged `0.9.3-avatar-edges` aligns region starts/exclusive ends and every DMA
chunk to even coordinates. Expanded pixels are repainted from the current scene.
The predefined edge/1px/odd-size/variable-batch coverage checks, full/dirty avatar,
existing UI/stop/HID/wireless/source/synthetic-flash checks, target stack and
pinned build/package/source hashes pass; zero compiler warnings. Relative to
0.9.2, `display.c` is the only changed firmware implementation file; motion,
input, stop, power and sensor sources remain byte-identical. No preview update
is needed because scene pixels are unchanged. See the
[sanitized record](../artifacts/ui/avatar-edges-build-check.json).
No flash/device probe ran. Alignment violations were reproduced offline;
physical trail removal and scanout tearing remain unverified.

## Network startup diagnostic candidate

The user reports 0.9.3 is visually acceptable with small remaining defects, but
configured USB and station HTTP fail, and Set up Wi-Fi produces neither QR nor
hotspot. Authorized read-only LAN HTTP80/HTTPS8443 probes timed out; the host
neighbor entry was unresolved. This does not establish root cause or prove a
shared USB failure mechanism. No HID, configuration write or flash was executed.

`0.9.4-network-diagnostics` implements the [predefined diagnostic scope](history/network-startup.md).
Actual-function/fake-SDK checks pass for delayed AP requests, each AP driver
failure and rollback, HTTP allocation failure and all 15 route-registration
failures despite HTTPS readiness. Existing UI/stop/HID/wireless/IMU/avatar,
source and synthetic-flash checks pass. Fifty bilingual native previews were
rendered; setup-error and connection-status pages were visually inspected and
all layout CJK glyphs are present. Pinned compile/package/source hashes and
target stack budgets pass, with zero compiler warnings. See the
[sanitized record](../artifacts/ui/network-diagnostics-build-check.json).

The runtime changes expose startup state/errors and setup context; they do not
claim to repair the connection failure. No new flash or device probe ran during
implementation. After a separately authorized flash, read AP stage/error, HTTP
status and current/minimum heap; correlate those with actual interface access.

### Tool-directory memory follow-up

The user reports address access returning after reset/reboot, then initialization
success, tool discovery URLError -1005 and later HTTP503. A read-only LAN status
fetch received HTTP200 essential headers but no body within six seconds. This
shows partial service reachability, not a proven memory fault or root cause.

`0.9.5-tools-memory` retains the preceding diagnostics and assembles tools/list
directly from static JSON plus the encoded request ID. Predefined fixtures pass
for byte-identical responses, numeric/escaped/control/UTF-8 IDs, an incremental
8KiB allocation budget, both allocation failures and send-failure cleanup. With
custom cJSON hooks, maximum observed incremental allocation across those IDs
falls from 32,455 to 5,632 bytes. These host figures exclude allocator overhead
and disable cJSON's realloc fast path; they are not board free-heap readings.
Content-Length, schemas, authentication, four-session/ten-minute expiry, replay
ledger and input are retained. Initialization now distinguishes full sessions
from random-source failure, and `/status` exposes nonblocking session counts.

Wireless/HID/source checks, pinned build/package/hash and target UI stack checks
pass, with zero compiler warnings. Target frames are 384 bytes for mcp_handler
and 32 for the new helper; these alone are not an exhaustive HTTP task budget.
UI, rendering and input sources match 0.9.4, so previews were retained. See the
[sanitized record](../artifacts/ui/tools-memory-build-check.json). No flash, HID,
configuration write, simulator or E2E ran. Physical error recovery remains unverified.

### Hotspot lifecycle follow-up

The user reports total inaccessibility again after reboot and Set up Wi-Fi
returning Home. Installed version and actual reset versus navigation are pending
user confirmation. Before edits, actual-function fixtures reproduced both a
pending AP close not cancelled by re-entry, and an old AP-close screen transition
cancelling a newly queued setup page. `0.9.6-hotspot-lifecycle` reasserts explicit
intent and guards that transition against queued requests/startup errors; genuine
expiry still exits. Diagnostic setup shows version/reset reason/uptime, also
available in read-only status. No timeout, AP mode, power or console change.

Predefined lifecycle/error/expiry, prior UI/wireless/directory/stop/input timing,
avatar and source checks pass. Fifty bilingual previews were rendered; new
diagnostic pages were visually inspected. Pinned build/package/source hashes and
target UI stack budgets pass with zero compiler warnings. Input/identity/render
sources and physical stop/key/touch/grant functions are retained. See the
[sanitized record](../artifacts/ui/hotspot-lifecycle-build-check.json).
No flash, new device probe, HID, simulator or E2E ran. Physical network recovery
and the cause of any actual MCU reset remain unverified.

### PSRAM memory candidate

The user's 0.9.6 screen establishes AP MODE `ESP_ERR_NO_MEM` with 2,396 bytes
free, a 1,084-byte minimum and power-on reset reason 1. This is user-provided
device evidence for that allocation failure, not an independently measured peak
or proof that all prior network failures share the same cause.

The user-approved `0.9.7-psram-memory` follows the [predefined memory scope](history/network-startup.md).
Pinned IDF compilation/package, generated SDK configuration, actual diagnostic
helper with distinct synthetic heaps/absent PSRAM, UI/layout/stop/avatar/IMU,
wireless/HID/source and synthetic-flash checks pass with zero compiler warnings.
Fifty bilingual previews were regenerated and the diagnostic layout inspected;
target UI stack budgets pass. See the [sanitized record](../artifacts/ui/psram-memory-build-check.json).

The final configuration enables Octal 80MHz PSRAM and its boot test, uses a
1KiB malloc threshold and 32KiB internal reserve, routes supported network
allocations externally and bounds Wi-Fi buffers. Receive BA/TCP reorder limits
remain six/four; external task stacks are explicitly disabled. NimBLE/TLS
allocation, USB/BLE/input/stop, identity storage and display/avatar sources are
retained. SDK PSRAM defaults extend the interrupt-watchdog timeout from 300ms
to 800ms; input action timing is unchanged. No eFuse, voltage or SDK version change.

The PSRAM driver also consumes internal memory: linked static data/BSS increases
by 1,388 bytes and internal text by 6,776 bytes. These are link-map figures,
not an 8MiB increase in internal heap or a measurement of runtime savings.
Actual PSRAM capacity, recovered internal headroom, repeated-operation leakage,
radio/USB access and phone stability remain unverified. No flash, new device
probe, HID, simulator or E2E ran. The image uses a fresh ignored SDK configuration;
existing generated sdkconfig values can override changed defaults when rebuilding.

Subsequent user feedback rejects 0.9.7 for device acceptance: Home appears,
buttons do not respond and the screen later becomes black. This supersedes any
interpretation of build success as runtime readiness. The actual flashed hash,
crash/reset reason and cause are unconfirmed. The 0.9.6 recovery image hash was
rechecked locally; no new flash, erase or device diagnostic was executed.

### Offline reset diagnostic candidate

After the user-reported rollback, Wi-Fi setup blacks out briefly then returns
Home; USB `/status` is reported unreachable. This still does not distinguish a
real reset from navigation. `0.9.8-reset-info` implements the [predefined offline
scope](history/network-startup.md) with both defaults and generated SDK values identical
to 0.9.6, PSRAM disabled. Read-only status compiles with either PSRAM setting.

Predefined actual RTC/module, device/layout, AP stage/error and both compiled
memory-helper branches pass, alongside existing UI/HID/wireless/stop/avatar/IMU,
source and synthetic-flash checks. Invalid/torn/cold/brownout records are rejected;
software/panic/watchdog records are boot-latched and consumed once. All Connect
button bounds remain unchanged. Fifty bilingual previews were generated and the
Connect diagnostics inspected; pinned compilation/package/source matching and
target UI stack budgets pass, with zero compiler warnings. See the
[sanitized record](../artifacts/ui/reset-info-build-check.json).

Only numeric stage/error/heap/largest-block/task-stack values enter RTC memory,
never NVS, credentials or actions. A previous stage is not a panic backtrace or
causality proof; runtime trace overhead and RTC retention remain unverified.
No flash, new device probe, input, simulator or E2E ran. This is a diagnostic
candidate, not an accepted fix for low-memory networking or startup regression.

### Compact avatar-cache repair candidate

The user's 0.9.8 screen shows AP MODE `ESP_ERR_NO_MEM`, internal free 6,388 bytes,
about 1.1KiB minimum/4KiB largest block, and power-on reset reason 1. This attempt
confirms an allocation failure, not panic. `0.9.9-compact-cache` implements the
[predefined compact-cache scope](history/network-startup.md). Target symbols/link-map
confirm 16-byte command and eight-byte bounds caches with the original 160 slots;
static internal BSS falls exactly 8,960 bytes, with internal code unchanged.

The pre-edit native reference and candidate produce identical full RGB bytes
for 110 predefined idle/tool/tilt/touch/replacement/mixed samples. Signed-range
rejection, original capacity/overflow/recovery, existing UI/full-dirty raster/
DMA/avatar/IMU/stop/input timing/HID/wireless/source/synthetic-flash checks pass.
Pinned compilation/package, source matching, identical 0.9.8 SDK configuration
and target UI stack budgets pass, zero compiler warnings. See the
[sanitized record](../artifacts/ui/compact-cache-build-check.json).

Only `avatar_native.cpp` changes firmware implementation; SDK/radio/USB/BLE/
input/stop/storage/power/identity/reset diagnostics remain. No PSRAM or removed
features. The native byte comparison does not measure panel scanout or FPS.
No flash, new device probe, phone input, simulator or E2E ran. Actual recovered
heap, AP/scan/network/MCP operation and sustained stability remain unverified.

The user subsequently reports the problem resolved on the 0.9.9 handoff and
continues testing, requesting no commit. This is user feedback, not an independent
image-hash/endurance run.

### BLE status wording

The user reports Bluetooth reconnecting on the phone before StopWatch's label,
with the label eventually updating after a long wait. Source shows the UI used
full HID readiness for both link state and pairing advice. `0.9.10-ble-state`
changes only the Home status label/color to distinguish waiting, BLE link
connected/awaiting input and fully ready; [pre-edit scope](history/touch-ui.md). Start
still requires the same encrypted mouse/keyboard subscriptions. BLE callbacks,
timing, routing, storage, grants, compact cache and SDK config are unchanged.

Existing UI/stop/wireless checks, glyph coverage, 54 bilingual previews including
the two predefined intermediate states, pinned compile/package/source and target
stack checks pass, with zero compiler warnings. New status layouts were visually
inspected; see [record](../artifacts/ui/ble-state-build-check.json). No new unit
tests, device probe, flash, input or commit. This clarifies the display; actual
reconnect duration/cause and device behavior remain unverified.

## Directory and documentation organization only

The [predefined scope](repository.md#file-ownership) groups first-party modules
under `app/`, `hardware/`, `input/`, `network/` and `ui/`, retaining one `main`
component. Common setup document paths remain stable; implementation history and
research move under `history/` and `research/`. Historical JSON evidence, upstream
licenses, HID descriptors and all implementation/header contents are retained.

Seventy firmware source/header/page/dependency files remain byte-identical.
Allocatable code/data and relocations of all 34 target translation units match.
Only header search paths and source-name mappings change; source registration
order and other compiler options are retained, including the former logical file
names in assertions/errors. SDK configuration, RAM/IRAM section sizes and target
stack reports match. The flash read-only section shrinks with source archive/docs.
All 54 bilingual previews, the gallery and layout JSON remain byte-identical.

Existing UI/HID/wireless, synthetic flashing, source-offer, link and whitespace
checks pass; pinned SDK compilation/packaging completes with no compiler warnings.
See the [sanitized record](../artifacts/organization/build-check.json). No flashing,
device access, hardware performance measurement or commit. Existing provenance gaps
and device acceptance boundaries remain.
