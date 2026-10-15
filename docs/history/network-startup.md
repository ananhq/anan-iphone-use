# Network startup diagnostics

English · [简体中文](network-startup.zh-CN.md)

## Offline reset evidence: acceptance before runtime edits

After the user-reported rollback, Set up Wi-Fi briefly blacks out then returns
Home, and USB `/status` is unreachable. This suggests a reset but does not prove
one. Prepare `0.9.8-reset-info` without claiming a network repair:

- Restore the exact 0.9.6 sdkconfig defaults; no PSRAM, new radio allocations or
  input changes. Compile PSRAM-specific status calls out when disabled.
- Retain a small checksummed RTC no-init record containing only AP stage/error,
  internal free/largest-block bytes and the calling task's stack high-water
  mark in bytes. Save at existing AP progress points and before error rollback.
  Capture a prior valid record only after software/panic/watchdog reset; discard
  it on power-on, brownout, invalid/torn records. No NVS, logs or credentials.
- Existing Connect / Wi-Fi shows reset reason/uptime, current internal free/
  minimum/largest block and previous AP record. Preserve every button's bounds,
  pairing/input grants, stop behavior and ordinary control animation.
- Predefine synthetic retention/checksum/reset-category fixtures; extend actual
  device/layout/startup checks before edits. Check both compiled PSRAM status
  branches, both languages, exact baseline SDK config and target stack budgets.
- Physical acceptance is separate: compare reset/uptime before/after one manual
  Wi-Fi setup attempt and read the previous stage/heap/stack if reset occurred.
  A retained stage is the last recorded event, not proof the SDK call caused a
  crash. No agent-executed flash, new probe or phone action in this slice.

## Compact-cache repair: acceptance before runtime edits

The user's 0.9.8 image shows AP MODE 0x101, internal free 6,388 bytes, minimum
about 1.1KiB, largest block about 4KiB and power-on reset reason 1. This attempt
is a confirmed allocation failure, not a confirmed panic. Target symbols show
two 6,400-byte avatar command caches, 2,560 bytes of bounds and 160 group bytes.

- Keep capacity 160 and all primitive types/colors/transforms. Store the current
  and previous commands as 16-byte kind/color/six-coordinate records, and clipped
  bounds as eight bytes. Decode to the existing integer arithmetic for painting;
  preserve meaningful fields including transformed spare line coordinates.
- Check signed-coordinate range before packing, retaining fail-closed overflow.
  Aim to release exactly 8,960 static internal bytes without PSRAM, changed task
  stacks, removed animations, radio settings or MCP protocol changes.
- Before edits, save a native reference executable; predefine full-frame RGB
  comparisons over idle, all tool families, tilt, touch, replacement and a long
  mixed sequence. Run existing full/dirty raster, command-limit, DMA, timing,
  input/stop and wireless checks, target stack and link-map comparison.
- Package `0.9.9-compact-cache` with matching source and retained reset evidence.
  No flash/new device probe. Device acceptance must establish actual headroom,
  AP/scan/HTTP/MCP behavior and animation appearance; extra capacity is not proof
  that all network problems or any possible leak are fixed.

**0.9.7 device feedback: not accepted.** After the PSRAM candidate handoff the
user reports Home appearing, unresponsive buttons and a later black screen.
This is a startup regression report; the flashed hash, crash/reset and root
cause are not independently confirmed. Stop recommending 0.9.7. The retained
0.9.6 image is the recovery path for the previously operable UI, not a fix for
its known low-memory network failure. The user subsequently reports rollback;
no agent-executed device recovery has run.

The user reports HTTP connection failure over configured USB and station Wi-Fi,
and no QR/hotspot after selecting Set up Wi-Fi. A scoped read-only LAN check
found HTTP80/HTTPS8443 timeouts and unresolved neighbor discovery. This does not
identify a firmware root cause, or rule out LAN isolation/host restrictions.
No client, radio, pairing-storage or input protocol change is proposed here.

## Acceptance defined before runtime edits

- Remember that the user entered Wi-Fi setup. Until the AP is ready, show its
  actual startup stage/error, never the unrelated Connect MCP app instructions.
  Existing MCP-only setup, pairing approval, QR, Back/stop and AP lifetime remain.
- Record queued/release/BLE/mode/config/start/ready stages and the actual API
  error. A rejected request or driver failure is visible on the screen; retain
  the failing stage, preserve settings, and perform the existing rollback only.
  An API success is not proof of beacon reception or phone connectivity.
- Expose HTTP startup success/error separately from the existing HTTP-or-HTTPS
  input readiness. Show HTTP and heap/minimum-heap diagnostics on pending/error
  Wi-Fi setup; add a compact HTTP indicator to Connect / Wi-Fi. Retain grants.
  Add only non-secret fields to existing read-only `/status`, no MCP tools.
- Predefine fake-SDK fixtures for request rejection, asynchronous queuing, each
  AP API failure and success/rollback, and HTTP start/route registration failure
  despite HTTPS readiness. Extend existing device/layout fixtures for delayed
  AP activation, correct setup context, both languages, Back and retained stop.
- Build/package `0.9.4-network-diagnostics` separately with matching source,
  target stack checks and native UI previews. Keep 0.9.3 as rollback. No flash,
  HID, configuration mutation or new device probes in this implementation slice.

This candidate diagnoses the reported failure; it does not claim to repair its
unconfirmed cause. Physical acceptance needs the displayed stage, error, heap
and actual USB/Wi-Fi/hotspot behavior after a separately authorized flash.

## Tool-discovery follow-up: acceptance before runtime edits

The user reports access returning after MCP reset/reboot, then initialization
success followed by tool-discovery URLError -1005, later HTTP503. Source review
confirms four sessions retained for ten idle minutes and that the client's
pinned SDK disconnect does not DELETE its session. `/mcp` initialization also
maps random-generation failure to the same session-limit text. These explain
possible exhaustion, not the initial connection loss. A subsequent read-only
status fetch received HTTP200 essential headers but no complete body in six
seconds. The root network/transfer failure remains unconfirmed.

Before edits, a host profile of the pinned cJSON path measured 18,853 bytes for
the 4,825-byte directory tree and 29,721 bytes peak through serialization; a
22,290-byte budget failed serialization. Custom allocation hooks disable cJSON's
realloc fast path; these are host allocation measurements, not board heap proof.

- Assemble the unchanged JSON-RPC tools/list response from the existing static
  directory and an encoded request ID. Preserve Content-Length, status/type,
  tool definitions, authentication, sessions, no-replay ledger and timing.
- Predefine byte-equivalence fixtures for numeric, quoted/control/UTF-8 IDs,
  allocation/send failures and allocator cleanup. An 8KiB synthetic incremental
  heap budget must support the new response; compare the prior actual path.
- Separate session-limit and random-source error text. Read-only `/status` can
  expose non-secret session occupancy with a nonblocking lock, alongside existing
  heap fields. No TTL, session eviction, client or input execution changes.
- Package `0.9.5-tools-memory` with the preceding network diagnostics. Build and
  static checks cannot establish that reduced heap use fixes the physical error.

## Hotspot re-entry follow-up: acceptance before runtime edits

The user reports total loss of access after reboot and Set up Wi-Fi returning
Home. The installed version and whether this is a hardware reboot are not yet
confirmed. Source inspection finds two lifecycle races: an already-active AP
request returns without cancelling a pending close; the screen's old-AP-close
transition unconditionally exits a newly opened setup page between screen ticks.

- Predefine actual-function fixtures for re-entering while AP close is pending,
  old close followed by a new queued request, and failed restart. All must retain
  the new setup intent and visible diagnostics. Genuine expiry must still exit.
- Always reassert explicit setup intent, including when the AP is still active.
  Only auto-exit on an old AP closing when no new request or setup error exists.
  Preserve AP mode/timeout, user Back, input stop and pairing requirements.
- Show build version and numeric reset reason/uptime on diagnostic setup so a
  navigation exit can be distinguished from a real reboot. Add read-only reset/
  uptime status fields. No console transport, power or radio changes.
- Build/package `0.9.6-hotspot-lifecycle` separately. This corrects the reproduced
  navigation/request races, not the unconfirmed total-network failure. Physical
  startup and reset diagnosis remain separate device acceptance.

## PSRAM follow-up: acceptance before runtime edits

The user supplied a 0.9.6 screen showing AP MODE failure 0x101
(`ESP_ERR_NO_MEM`), 2,396 bytes free and a 1,084-byte minimum. This confirms
allocation failure during AP mode selection, not the allocation size, a leak,
or every preceding network failure. Reset reason 1 is power-on. The user agreed
to try PSRAM after discussing its limits and preserving the existing features.

- Follow the pinned M5Stack factory configuration: Octal PSRAM at 80MHz, startup
  initialization and memory test. Use the SDK malloc allocator with a 1KiB
  internal-preference threshold and a 32KiB internal/DMA reserve. Keep task
  stacks, NimBLE, TLS allocation mode and the LCD DMA buffer internal; do not
  move executable code, static BSS, avatar caches or interrupt data externally.
- Prefer PSRAM for SDK-supported Wi-Fi/lwIP allocations. Explicitly retain ten
  static RX buffers; use the SDK's PSRAM-compatible sixteen static TX buffers
  and bounded cache queue rather than silently inheriting changed defaults.
  Retain the six-frame receive BA window and four-pbuf TCP out-of-order bound.
  These internal TX buffers cost about 25.6KB; PSRAM capacity is not internal
  free space, and sufficient headroom must be established on the board.
- Before implementation, extend synthetic device/layout scenarios for distinct
  internal current/minimum/largest-block and external free-space values. Add an
  actual-function diagnostic fixture for healthy and unavailable PSRAM, retaining
  the existing total-heap fields. Check the final generated SDK configuration,
  unchanged task/USB/BLE/display/input behavior, source archive and package.
- Package `0.9.7-psram-memory` separately and preserve 0.9.6 for rollback. No
  agent-executed flashing or new device probe in this slice. Device acceptance:
  confirm PSRAM initialization and capacity; cold boot with saved pairing/Wi-Fi;
  repeatedly enter/leave hotspot setup and scan; verify USB and Wi-Fi HTTP/status,
  initialize/tools-list, then separately authorized BLE/USB input and physical
  stop. Track internal free/minimum/largest blocks across repeats to distinguish
  transient demand, fragmentation and possible leakage. Build/static checks do
  not establish recovered capacity, long-term stability or phone success.
